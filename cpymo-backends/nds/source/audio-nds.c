#include <nds.h>
#include <maxmod9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpymo_engine.h"
#include "cpymo_audio.h"
#include "config.h"
#include "global.h"
#include "fixed.h"
#include "bit.h"
#include "timer.h"
#include "stream.h"
#include "frame.h"
#include "synth.h"
#include "cache-nds.h"
#include <errno.h>
extern volatile unsigned engine_state[46];
extern volatile unsigned vblank_count;
typedef struct {struct mad_stream stream;struct mad_frame frame;struct mad_synth synth;unsigned char input[8192+MAD_BUFFER_GUARD];unsigned used;bool padded;} MP3;
typedef struct {FILE* file;bool loop;int volume;char name[64];s16 cache[1152];unsigned pos,count;unsigned offset,length,remaining,rate,phase;MP3* mp3;} Channel;
typedef struct {Channel ch[3];} Audio;
static Audio* active;
static u64 last_audio_tick;
static unsigned last_audio_vblank;
static bool audio_suspended;
extern volatile unsigned sleep_state[12];
extern void nds_mm_stream_suspend(void);
extern void nds_mm_stream_resume(void);
static void stop(Channel* c){if(c->file)fclose(c->file);if(c->mp3){mad_synth_finish(&c->mp3->synth);mad_frame_finish(&c->mp3->frame);mad_stream_finish(&c->mp3->stream);free(c->mp3);}c->mp3=NULL;c->file=NULL;c->pos=c->count=0;c->name[0]=0;}
static bool refill_mp3(Channel* c){
 MP3* d=c->mp3;unsigned keep=0;
 if(d->stream.next_frame&&d->stream.next_frame>=d->input&&d->stream.next_frame<=d->input+d->used){keep=d->input+d->used-d->stream.next_frame;memmove(d->input,d->stream.next_frame,keep);}
 unsigned need=8192-keep;if(need>c->remaining)need=c->remaining;
 unsigned n=fread(d->input+keep,1,need,c->file);c->remaining-=n;
 if(n!=need){engine_state[19]++;return false;}
 d->used=keep+n;
 if(!c->remaining){if(d->padded)return false;memset(d->input+d->used,0,MAD_BUFFER_GUARD);d->used+=MAD_BUFFER_GUARD;d->padded=true;}
 mad_stream_buffer(&d->stream,d->input,d->used);return d->used>0;
}
static s16 pcm16(mad_fixed_t s){s+=1L<<(MAD_F_FRACBITS-16);if(s>=MAD_F_ONE)s=MAD_F_ONE-1;if(s< -MAD_F_ONE)s=-MAD_F_ONE;return s>>(MAD_F_FRACBITS+1-16);}
static bool decode_mp3(Channel* c){
 MP3* d=c->mp3;
 unsigned started=vblank_count;
 u64 started_tick=tickGetCount();
 for(unsigned tries=0;tries<4096;tries++){
  if(!d->stream.buffer||d->stream.error==MAD_ERROR_BUFLEN){if(!refill_mp3(c)){
   if(!c->loop)return false;
   if(fseek(c->file,c->offset,SEEK_SET))return false;
   c->remaining=c->length;mad_stream_finish(&d->stream);mad_stream_init(&d->stream);d->used=0;d->padded=false;mad_frame_mute(&d->frame);mad_synth_mute(&d->synth);continue;
  }}
  if(mad_frame_decode(&d->frame,&d->stream)==-1){if(d->stream.error==MAD_ERROR_BUFLEN||MAD_RECOVERABLE(d->stream.error))continue;engine_state[19]++;return false;}
  // Use libmad's half-rate synthesis for high-rate sources; no offline conversion.
  if(d->frame.header.samplerate>=32000)d->frame.options|=MAD_OPTION_HALFSAMPLERATE;
  mad_synth_frame(&d->synth,&d->frame);c->count=d->synth.pcm.length;c->pos=0;c->rate=d->synth.pcm.samplerate;
  for(unsigned i=0;i<c->count;i++){mad_fixed_t s=d->synth.pcm.samples[0][i];if(d->synth.pcm.channels==2)s=(s/2)+(d->synth.pcm.samples[1][i]/2);c->cache[i]=pcm16(s);}
  engine_state[21]++;unsigned took=vblank_count-started;if(took>engine_state[24])engine_state[24]=took;
  unsigned us=(tickGetCount()-started_tick)*1000000/TICK_FREQ;if(us>engine_state[26])engine_state[26]=us;
  return c->count>0;
 }
 engine_state[19]++;return false;
}
static int next(Channel* c){
 if(!c->file)return 0;
 if(c->mp3){
  if(c->pos>=c->count&&!decode_mp3(c)){stop(c);return 0;}
  int sample=c->cache[c->pos]*c->volume/256;c->phase+=c->rate;
  while(c->phase>=22050){c->phase-=22050;++c->pos;if(c->pos>=c->count&&!decode_mp3(c)){stop(c);break;}}
  return sample;
 }
 if(c->pos==c->count){
  c->count=fread(c->cache,sizeof(s16),1024,c->file);c->pos=0;
  if(!c->count&&c->loop){rewind(c->file);c->count=fread(c->cache,sizeof(s16),1024,c->file);}
  if(!c->count){if(ferror(c->file))engine_state[19]++;stop(c);return 0;}
 }
 return c->cache[c->pos++]*c->volume/256;
}
static mm_word refill(mm_word length,mm_addr dest,mm_stream_formats format){
 (void)format;s16* out=dest;engine_state[17]++;
 for(unsigned i=0;i<length;++i){int sample=0;if(active)for(unsigned c=0;c<3;++c)sample+=next(&active->ch[c]);if(sample>32767)sample=32767;if(sample< -32768)sample=-32768;out[i]=(s16)sample;if(sample)engine_state[18]++;}
 return length;
}
void cpymo_audio_init(cpymo_audio_system* s){
 Audio* a=calloc(1,sizeof *a);*s=a;active=a;last_audio_tick=tickGetCount();if(!a)return;
 for(unsigned c=0;c<3;++c)a->ch[c].volume=256;
 mm_ds_system sys={0};mmInit(&sys);
 mm_stream stream={0};stream.sampling_rate=22050;stream.buffer_length=nds_cache_enabled?16384:2048;stream.callback=refill;stream.format=MM_STREAM_16BIT_MONO;stream.timer=MM_TIMER0;stream.manual=true;
 mmStreamOpen(&stream);engine_state[15]=1;
}
void nds_audio_update(void){if(active&&!audio_suspended){u64 started_tick=tickGetCount();bool playing=active->ch[0].file||active->ch[1].file||active->ch[2].file;if(playing&&last_audio_tick){unsigned gap=(started_tick-last_audio_tick)*1000000/TICK_FREQ;if(gap>engine_state[28])engine_state[28]=gap;}last_audio_tick=started_tick;unsigned started=vblank_count;mmStreamUpdate();unsigned took=vblank_count-started;if(took>engine_state[25])engine_state[25]=took;unsigned us=(tickGetCount()-started_tick)*1000000/TICK_FREQ;if(us>engine_state[27])engine_state[27]=us;last_audio_vblank=vblank_count;}}
void nds_audio_suspend(void){
 if(!active||audio_suspended)return;
 nds_audio_update();sleep_state[7]=mmStreamGetPosition();
 nds_mm_stream_suspend();soundSetMixerVolume(0);soundSynchronize();audio_suspended=true;
 sleep_state[5]++;sleep_state[8]=0;for(unsigned c=0;c<3;c++)if(active->ch[c].file)sleep_state[8]++;
}
void nds_audio_resume(void){
 if(!active||!audio_suspended)return;
 sleep_state[9]=210;nds_mm_stream_resume();sleep_state[9]=220;soundSetMixerVolume(127);sleep_state[9]=230;soundSynchronize();sleep_state[9]=240;audio_suspended=false;
 last_audio_tick=tickGetCount();last_audio_vblank=vblank_count;sleep_state[6]++;sleep_state[9]=0;
}
/* A cheap display-tick guard: no clock read or mixer call for every row. */
void nds_audio_pump_if_due(void){if(active&&vblank_count!=last_audio_vblank){nds_audio_update();last_audio_vblank=vblank_count;}}
void cpymo_audio_free(cpymo_audio_system* s){Audio* a=*s;if(a){mmStreamClose();for(unsigned c=0;c<3;++c)stop(&a->ch[c]);free(a);}if(active==a)active=NULL;*s=NULL;}
bool cpymo_audio_enabled(cpymo_engine* e){return e->audio!=NULL;}
float cpymo_audio_get_channel_volume(size_t c,const cpymo_audio_system* s){return *s?((Audio*)*s)->ch[c].volume/256.f:0;}
void cpymo_audio_set_channel_volume(size_t c,cpymo_audio_system* s,float v){if(v<0)v=0;if(v>1)v=1;if(*s)((Audio*)*s)->ch[c].volume=(int)(v*256);}
bool cpymo_audio_channel_is_playing(size_t c,cpymo_audio_system* s){return *s&&((Audio*)*s)->ch[c].file;}
bool cpymo_audio_channel_is_looping(size_t c,cpymo_audio_system* s){return *s&&((Audio*)*s)->ch[c].loop;}
bool cpymo_audio_wait_se(cpymo_engine* e,float d){(void)d;return !cpymo_audio_channel_is_playing(1,&e->audio);}
static bool known_missing_audio(const char* dir,const char* kind,cpymo_str name){
 char expected[512],path[512],line[96],entry[48];nds_cache_path(expected,sizeof expected,"",kind,name,"");
 const char* hash=strrchr(expected,'/');if(!hash)return false;
 snprintf(entry,sizeof entry,"%s %s\n",kind,hash+1);snprintf(path,sizeof path,"%s/nds-cache/missing-audio.txt",dir);
 FILE* file=fopen(path,"rb");if(!file)return false;bool found=false;
 while(fgets(line,sizeof line,file))if(!strcmp(line,entry)){found=true;break;}
 fclose(file);return found;
}
static error_t play(cpymo_engine* e,cpymo_str name,unsigned id,bool loop){
 Audio* a=e->audio;if(!a)return CPYMO_ERR_OUT_OF_MEM;
 Channel* c=&a->ch[id];stop(c);char path[512];const char* folder=id==0?"bgm":id==1?"se":"voice";
 if(name.len>=sizeof c->name)return CPYMO_ERR_INVALID_ARG;
 memcpy(c->name,name.begin,name.len);c->name[name.len]=0;
 if(nds_cache_enabled){nds_cache_path(path,sizeof path,e->assetloader.gamedir,folder,name,".pcm");c->file=fopen(path,"rb");if(!c->file){if(errno==ENOENT&&known_missing_audio(e->assetloader.gamedir,folder,name)){c->name[0]=0;engine_state[44]++;return CPYMO_ERR_SUCC;}engine_state[19]++;return CPYMO_ERR_CAN_NOT_OPEN_FILE;}engine_state[42]++;}
 else {snprintf(path,sizeof path,"%s/%s/%s.pcm",e->assetloader.gamedir,folder,c->name);c->file=fopen(path,"rb");}
 c->loop=loop;
 if(!c->file){
  const char* ext=id==0?e->gameconfig.bgmformat:id==1?e->gameconfig.seformat:e->gameconfig.voiceformat;
  cpymo_package* pkg=id==1&&e->assetloader.use_pkg_se?&e->assetloader.pkg_se:id==2&&e->assetloader.use_pkg_voice?&e->assetloader.pkg_voice:NULL;
  c->offset=0;
  if(pkg){cpymo_package_index index;error_t err=cpymo_package_find(&index,pkg,name);if(err){engine_state[19]++;return err;}snprintf(path,sizeof path,"%s/%s/%s.pak",e->assetloader.gamedir,folder,folder);c->offset=index.file_offset;c->length=index.file_length;}
  else snprintf(path,sizeof path,"%s/%s/%s.%s",e->assetloader.gamedir,folder,c->name,ext);
  c->file=fopen(path,"rb");if(!c->file){engine_state[19]++;return CPYMO_ERR_CAN_NOT_OPEN_FILE;}
  if(!pkg){fseek(c->file,0,SEEK_END);c->length=ftell(c->file);}
  if(fseek(c->file,c->offset,SEEK_SET)){stop(c);return CPYMO_ERR_BAD_FILE_FORMAT;}
  c->remaining=c->length;c->phase=0;
  c->mp3=calloc(1,sizeof *c->mp3);if(!c->mp3){stop(c);return CPYMO_ERR_OUT_OF_MEM;}
  mad_stream_init(&c->mp3->stream);mad_frame_init(&c->mp3->frame);mad_synth_init(&c->mp3->synth);
  if(!decode_mp3(c)){stop(c);return CPYMO_ERR_BAD_FILE_FORMAT;}
  if(pkg)engine_state[22]++;else engine_state[23]++;
 }
 engine_state[16]++;return CPYMO_ERR_SUCC;
}
error_t cpymo_audio_bgm_play(cpymo_engine* e,cpymo_str n,bool loop){return play(e,n,0,loop);}
error_t cpymo_audio_se_play(cpymo_engine* e,cpymo_str n,bool loop){return play(e,n,1,loop);}
error_t cpymo_audio_vo_play(cpymo_engine* e,cpymo_str n){return play(e,n,2,false);}
void cpymo_audio_bgm_stop(cpymo_engine* e){if(e->audio)stop(&((Audio*)e->audio)->ch[0]);}
void cpymo_audio_se_stop(cpymo_engine* e){if(e->audio)stop(&((Audio*)e->audio)->ch[1]);}
void cpymo_audio_vo_stop(cpymo_engine* e){if(e->audio)stop(&((Audio*)e->audio)->ch[2]);}
const char* cpymo_audio_get_bgm_name(cpymo_engine* e){return e->audio?((Audio*)e->audio)->ch[0].name:"";}
const char* cpymo_audio_get_se_name(cpymo_engine* e){return e->audio?((Audio*)e->audio)->ch[1].name:"";}
error_t cpymo_audio_play_video(cpymo_engine* e,const char* path){(void)e;(void)path;return CPYMO_ERR_UNSUPPORTED;}
void cpymo_audio_copy_mixed_samples(void* dst,size_t len,cpymo_audio_system* s){(void)s;memset(dst,0,len);}
bool cpymo_audio_channel_get_samples(void** samples,size_t* len,size_t c,cpymo_audio_system* s){(void)s;(void)c;*samples=NULL;*len=0;return false;}
