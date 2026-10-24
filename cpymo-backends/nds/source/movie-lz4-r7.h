/* Experimental NVL1 RGB555/LZ4 delta video; AGPL-3.0-or-later. */
volatile unsigned movie_state[24]={0x4e564c37};
extern bool nds_movie_audio_begin(const char*,unsigned,unsigned);
extern void nds_movie_audio_verify(bool);
extern bool nds_movie_audio_begin_nar(const char*,unsigned,unsigned);
extern unsigned nds_movie_audio_position(void);
extern void nds_movie_audio_end(void);
static bool nvl_read(FILE* f,void* dst,unsigned n){
 unsigned done=0;while(done<n){unsigned k=n-done;if(k>8192)k=8192;if(fread((unsigned char*)dst+done,1,k,f)!=k)return false;done+=k;nds_audio_pump_if_due();}return true;
}
error_t __wrap_cpymo_movie_play(cpymo_engine* e,cpymo_str name){
 FILE* audio_verify=savefile(e->assetloader.gamedir,"nds-video-verify.flag","rb");nds_movie_audio_verify(audio_verify!=NULL);if(audio_verify)fclose(audio_verify);
 movie_state[2]++;char path[512];nds_cache_path(path,sizeof path,e->assetloader.gamedir,"video",name,".nvl");
 FILE* f=fopen(path,"rb");if(!f){movie_state[8]++;engine_state[45]++;return CPYMO_ERR_SUCC;}
 unsigned h[8];bool header_read=fread(h,4,8,f)==8;bool compressed=header_read&&!memcmp(h,"NVA1",4);bool valid=header_read&&(!memcmp(h,"NVL1",4)||compressed)&&h[1]==256&&h[2]==192&&h[3]>0&&h[3]<=30&&h[4]>0&&h[4]<=h[3]*600&&h[5]==22050&&h[6]==(unsigned long long)h[4]*22050/h[3]&&h[7]>=32+h[4]*12&&h[7]<512*1024*1024;
 if(valid){long pos=ftell(f);valid=!fseek(f,0,SEEK_END);long size=valid?ftell(f):-1;if(valid)valid=compressed?(size>=(long)(h[7]+16)&&size<=(long)(h[7]+16+h[6]*2+((h[6]+1023)/1024)*12)):(size==(long)(h[7]+h[6]*2));valid=valid&&!fseek(f,pos,SEEK_SET);}
 unsigned char *packet=valid?malloc(98304):NULL,*delta=packet?malloc(98304):NULL;
 bool started=false,failed=!packet||!delta,skipped=false;unsigned last_samples=0,audio_errors_before=movie_state[7];
 if(failed)goto done;
 memset(framebuffer,0,sizeof framebuffer);movie_state[1]=1;movie_state[19]=h[4];movie_state[11]=(unsigned long long)h[4]*1000/h[3];
 for(unsigned frame=0;frame<h[4];frame++){
  u64 start=tickGetCount();unsigned p[3];if(!nvl_read(f,p,12)||!p[0]||p[0]>98304||(p[1]&~3)||(!frame&&(p[1]&2))||!nvl_read(f,packet,p[0])){failed=true;break;}
  if(p[1]&1){if(!npl_decode(packet,p[0],delta,98304)){failed=true;break;}}else{if(p[0]!=98304){failed=true;break;}memcpy(delta,packet,98304);}
  if(p[1]&2){u32* a=(u32*)framebuffer;u32* b=(u32*)delta;for(unsigned i=0;i<98304/4;i++)a[i]^=b[i];}else memcpy(framebuffer,delta,98304);
  if(npl_crc((unsigned char*)framebuffer,98304)!=p[2]){failed=true;break;}
  unsigned us=(tickGetCount()-start)*1000000/TICK_FREQ;if(us>movie_state[9])movie_state[9]=us;movie_state[3]++;
  if(frame){while((unsigned long long)nds_movie_audio_position()*h[3]<(unsigned long long)frame*22050){nds_power_poll();scanKeys();if(keysDown()&KEY_B){skipped=true;goto done;}nds_audio_update();swiWaitForVBlank();}}
  nds_audio_update();DC_FlushRange(framebuffer,sizeof framebuffer);dmaCopy(framebuffer,vram,sizeof framebuffer);movie_state[4]++;movie_state[15]=frame;
  if(!started){if(!(compressed?nds_movie_audio_begin_nar(path,h[7],h[6]):nds_movie_audio_begin(path,h[7],h[6]))){failed=true;break;}started=true;}
  if(movie_state[7]!=audio_errors_before){failed=true;break;}
  unsigned samples=nds_movie_audio_position();unsigned target=(unsigned long long)frame*22050/h[3];unsigned late=samples>target?(unsigned long long)(samples-target)*1000000/22050:0;
  if(late>movie_state[16])movie_state[16]=late;if(late>1000000/h[3])movie_state[5]++;
  if(late>500000){failed=true;break;}
 }
 if(started&&!failed&&!skipped)while(nds_movie_audio_position()<h[6]){nds_power_poll();scanKeys();if(keysDown()&KEY_B){skipped=true;break;}nds_audio_update();swiWaitForVBlank();}
done:
 if(movie_state[7]!=audio_errors_before)failed=true;
 if(started){last_samples=nds_movie_audio_position();nds_movie_audio_end();}movie_state[18]=last_samples;movie_state[10]=(unsigned long long)last_samples*1000/22050;
 if(failed){movie_state[7]++;engine_state[45]++;}else if(skipped)movie_state[12]++;else movie_state[6]++;
 free(delta);free(packet);fclose(f);movie_state[1]=0;movie_state[21]++;loaded_image=true;cpymo_engine_request_redraw(e);return CPYMO_ERR_SUCC;
}
