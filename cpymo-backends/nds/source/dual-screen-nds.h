/* CPyMO for NDS · experimental dual-screen routing for 1.2.
 * Original independent implementation; behavior informed by the 3DS backend's
 * draw-type partition. No upstream 3DS rendering code is copied.
 * Enabled in-game with L+SELECT; SELECT alone remains quick-load, L alone
 * remains LCD swap, Y alone remains auxiliary hide.
 */
#ifndef CPYMO_NDS_DUALSCREEN_H
#define CPYMO_NDS_DUALSCREEN_H

static bool nds_dual_mode, nds_dual_bottom_pass;
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
static bool nds_dual_is_ui(enum cpymo_backend_image_draw_type type){
 return type==cpymo_backend_image_draw_type_ui_bg ||
        type==cpymo_backend_image_draw_type_ui_element ||
        type==cpymo_backend_image_draw_type_ui_element_bg;
}
static bool nds_dual_accept(enum cpymo_backend_image_draw_type type){
 if(!nds_dual_active())return true;
 bool lower = cpymo_ui_enabled(&engine)?nds_dual_is_ui(type):nds_dual_is_say(type);
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
 if(engine.say.msgbox && engine.say.namebox && engine.say.msgbox_h>0){
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
 nds_dual_mode=false;nds_dual_bottom_pass=false;
 char path[512];nds_dual_filename(path,sizeof path,gamedir);
 FILE* f=fopen(path,"rb");
 if(f){nds_dual_mode=fgetc(f)=='1';fclose(f);}
 nds_dual_state[1]=nds_dual_mode;
 nds_dual_state[2]=0;
}
static void nds_dual_save(const char* gamedir){
 char path[512];nds_dual_filename(path,sizeof path,gamedir);
 FILE* f=fopen(path,"wb");bool ok=false;
 if(f){ok=fputc(nds_dual_mode?'1':'0',f)!=EOF;if(fclose(f))ok=false;}
 nds_dual_state[4]++;
 if(!ok)nds_dual_state[5]++;
}
static void nds_dual_toggle(void){
 nds_dual_mode=!nds_dual_mode;
 nds_dual_bottom_pass=false;
 if(nds_dual_mode){
  /* Enhanced layout always means physical top = scene, bottom = dialogue. */
  lcdMainOnTop();
  screens_swapped=false;
  presentation_state[1]=0;
  nds_aux_override=false;
 }
 nds_aux_refresh(); /* Repaint classic help panel only when leaving enhanced. */
 nds_dual_state[1]=nds_dual_mode;
 nds_dual_state[3]++;
 nds_dual_save(engine.assetloader.gamedir);
 cpymo_engine_request_redraw(&engine);
}
#endif
