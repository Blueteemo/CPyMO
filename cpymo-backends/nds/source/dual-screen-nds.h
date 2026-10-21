/* CPyMO for NDS · experimental dual-screen routing for 1.2.
 * Original independent implementation; behavior informed by the 3DS backend's
 * draw-type partition. No upstream 3DS rendering code is copied.
 * L cycles classic top -> classic bottom -> enhanced dual screen.
 * SELECT still quick-loads; Y still hides/reveals the auxiliary LCD.
 */
#ifndef CPYMO_NDS_DUALSCREEN_H
#define CPYMO_NDS_DUALSCREEN_H

/* 0: classic game top / auxiliary bottom
 * 1: classic game bottom / auxiliary top
 * 2: enhanced scene top / dialogue or menu bottom. */
static unsigned nds_display_mode;
static bool nds_dual_mode, nds_dual_bottom_pass, nds_dual_force_scene;
/* magic, selected, enabled, toggles, saves, save failures, bottom draws,
 * skipped top, skipped bottom */
volatile unsigned nds_dual_state[9]={0x4e445332};

static bool nds_dual_active(void){
 return nds_dual_mode && nds_playing && !error_ui_active && !nds_aux_hidden();
}
static bool nds_dual_is_say(enum cpymo_backend_image_draw_type type){
 return type==cpymo_backend_image_draw_type_text_say ||
        type==cpymo_backend_image_draw_type_text_say_textbox;
}
static bool nds_dual_has_choice(void){
 return !cpymo_ui_enabled(&engine) && engine.select_img.selections!=NULL;
}
static bool nds_dual_is_ui(enum cpymo_backend_image_draw_type type){
 return type==cpymo_backend_image_draw_type_ui_bg ||
        type==cpymo_backend_image_draw_type_ui_element ||
        type==cpymo_backend_image_draw_type_ui_element_bg;
}
static bool nds_dual_accept(enum cpymo_backend_image_draw_type type){
 if(!nds_dual_active())return true;
 /* A paused in-game menu does not own the scenery. Repaint its scene
    explicitly on top; the interactive menu itself belongs to the bottom. */
 if(nds_dual_force_scene)return !nds_dual_bottom_pass;
 bool lower = cpymo_ui_enabled(&engine)?nds_dual_is_ui(type):
     (nds_dual_is_say(type) ||
      (nds_dual_has_choice() && type==cpymo_backend_image_draw_type_sel_img));
 if(lower != nds_dual_bottom_pass){
  nds_dual_state[nds_dual_bottom_pass?8:7]++;
  return false;
 }
 return true;
}
/* Match the 3DS enhanced non-menu principle: pull the original dialogue
 * panel toward the middle of the lower LCD, leaving room for name + lines.
 * All computations are in the game's own logical coordinates. */
static float nds_dual_say_offset(void){
 if(!nds_dual_active() || !nds_dual_bottom_pass ||
    cpymo_ui_enabled(&engine))return 0;
 float height=(float)engine.gameconfig.imagesize_h;
 float box_h=height*.25f;
 if(engine.say.msgbox && engine.say.namebox && engine.say.msgbox_h>0 && engine.say.msgbox_w>0){
  box_h=(float)engine.say.msgbox_h *
    ((float)engine.gameconfig.imagesize_w/(float)engine.say.msgbox_w);
 }
 float font_size=cpymo_gameconfig_font_size(&engine.gameconfig);
 float name_height=font_size*1.4f;
 float top=height-box_h-name_height;
 /* Keep dialogue in a comfortable lower-LCD upper-middle position. */
 float destination_top=height*.20f;
 return destination_top-top;
}
static void nds_dual_filename(char* path,size_t size,const char* gamedir){
 snprintf(path,size,"%s/save/nds-dualscreen-mode.txt",gamedir);
}
static void nds_dual_load(const char* gamedir){
 nds_display_mode=0;
 nds_dual_mode=false;
 nds_dual_bottom_pass=false;
 nds_dual_force_scene=false;
 char path[512];nds_dual_filename(path,sizeof path,gamedir);
 FILE* f=fopen(path,"rb");
 if(f){
  char line[48]={0};
  if(fgets(line,sizeof line,f)){
   /* fgets retains the line ending; trim it before validating %n. Without
      this, every normally saved NDSDISPLAY2 file silently loads as classic. */
   line[strcspn(line,"\r\n")]=0;
   int parsed=0,mode=-1;
   if(sscanf(line,"NDSDISPLAY2 %d %n",&mode,&parsed)==1 &&
      parsed>0 && line[parsed]==0 && mode>=0 && mode<=2){
    nds_display_mode=(unsigned)mode;
   }else if(line[0]=='1' && (line[1]==0||line[1]=='\n')){
    /* First 1.2 dual prototype wrote '1' for enhanced mode. */
    nds_display_mode=2;
   }
  }
  fclose(f);
 }
 nds_dual_mode=nds_display_mode==2;
 nds_dual_state[1]=nds_display_mode;
 nds_dual_state[2]=0;
}
static void nds_dual_save(const char* gamedir){
 char path[512];nds_dual_filename(path,sizeof path,gamedir);
 FILE* f=fopen(path,"wb");bool ok=false;
 if(f){ok=fprintf(f,"NDSDISPLAY2 %u\n",nds_display_mode)>0;
       if(fclose(f))ok=false;}
 nds_dual_state[4]++;
 if(!ok)nds_dual_state[5]++;
}
static void nds_display_start(void){
 nds_dual_mode=nds_display_mode==2;
 nds_dual_bottom_pass=false;
 nds_dual_force_scene=false;
 if(nds_display_mode==1){
  lcdSwap();screens_swapped=true;
 }else{
  lcdMainOnTop();screens_swapped=false;
 }
 presentation_state[1]=screens_swapped;
 nds_dual_state[1]=nds_display_mode;
}
static void nds_display_cycle(void){
 unsigned next=(nds_display_mode+1)%3;
 /* LCD swap applies only to classic lower-game. Enhanced always has
    the scene on the physical upper LCD and interactive UI below. */
 if(next==1){lcdSwap();screens_swapped=true;}
 else if(nds_display_mode==1 || screens_swapped){
  lcdMainOnTop();screens_swapped=false;
 }
 nds_display_mode=next;
 nds_dual_mode=next==2;
 nds_dual_bottom_pass=false;
 nds_dual_force_scene=false;
 presentation_state[1]=screens_swapped;
 /* A newly selected display must be visible, even when the previous
    auxiliary LCD was temporarily hidden with Y. */
 nds_aux_override=false;
 nds_aux_refresh();
 nds_dual_state[1]=nds_display_mode;
 nds_dual_state[3]++;
 nds_dual_save(engine.assetloader.gamedir);
 cpymo_engine_request_redraw(&engine);
}
#endif
