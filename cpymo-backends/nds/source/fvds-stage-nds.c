#include "fvds-stage-nds.h"

/* Normalize callback codes while leaving our published stage result stable. */
static int cb_result(int value){
 return value>0?NDS_FVDS_STEP_SKIPPED:
        value<0?NDS_FVDS_STEP_ERROR:NDS_FVDS_STEP_FRAME;
}
static int poll_result(const nds_fvds_stage_callbacks *c, void *ctx){
 return c->poll?cb_result(c->poll(ctx)):NDS_FVDS_STEP_FRAME;
}
int nds_fvds_stage_step(nds_fvds_stream *file,nds_fvds_stage *stage,
                        const nds_fvds_stage_callbacks *callbacks,void *ctx){
 if(!file || !file->file || !stage || !callbacks ||
    !callbacks->video_begin || !callbacks->video_chunk ||
    !callbacks->video_end || !callbacks->audio_pcm)return NDS_FVDS_STEP_ERROR;
 nds_fvds_packet packet;
 int next=nds_fvds_next(file,&packet);
 if(next==1)return NDS_FVDS_STEP_EOF;
 if(next!=0)return NDS_FVDS_STEP_ERROR;
 int status=poll_result(callbacks,ctx);
 if(status)goto close_packet;
 /* 64-bit integer PTS, avoiding drift in 12 fps / 47605 Hz streams. */
 uint64_t pts=((uint64_t)packet.frame * 1000000u * file->fps_den) /
               file->fps_num;
 status=cb_result(callbacks->video_begin(ctx,packet.frame,pts));
 if(status)goto close_packet;
 for(uint32_t remain=packet.video_bytes;remain;){
  status=poll_result(callbacks,ctx);
  if(status)goto close_packet;
  size_t amount=remain<sizeof(stage->video)?remain:sizeof(stage->video);
  size_t got=nds_fvds_read_video(file,stage->video,amount);
  if(got!=amount){status=NDS_FVDS_STEP_ERROR;goto close_packet;}
  remain-=(uint32_t)got;
  status=cb_result(callbacks->video_chunk(ctx,stage->video,got));
  if(status)goto close_packet;
 }
 status=cb_result(callbacks->video_end(ctx,packet.frame));
 if(status)goto close_packet;
 for(uint32_t i=0;i<packet.audio_blocks;i++){
  status=poll_result(callbacks,ctx);
  if(status)goto close_packet;
  if(nds_fvds_read_audio(file,stage->audio,sizeof(stage->audio))!=
     sizeof(stage->audio) ||
     nds_fvds_adpcm_pair(stage->audio,sizeof(stage->audio),
                         stage->pcm,512)!=256){
   status=NDS_FVDS_STEP_ERROR;goto close_packet;
  }
  status=cb_result(callbacks->audio_pcm(ctx,stage->pcm,256));
  if(status)goto close_packet;
  stage->audio_frames+=256;
 }
 if(nds_fvds_finish(file))return NDS_FVDS_STEP_ERROR;
 stage->completed_frames++;
 return NDS_FVDS_STEP_FRAME;
close_packet:
 /* On cancellation the owner will normally close the movie immediately.
  * Still restore parser to a valid packet boundary for orderly teardown. */
 if(nds_fvds_finish(file))return NDS_FVDS_STEP_ERROR;
 return status;
}
