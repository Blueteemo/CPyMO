/* CPyMO for NDS · experimental dual-screen routing for 1.2.
 * Original independent implementation; behavior informed by the 3DS backend's
 * draw-type partition. No upstream 3DS rendering code is copied.
 * L cycles classic top -> classic bottom -> enhanced dual screen.
 * SELECT still quick-loads; Y still hides/reveals the auxiliary LCD.
 */
#ifndef CPYMO_NDS_DUALSCREEN_H
#define CPYMO_NDS_DUALSCREEN_H
#include "display-mode-prefs-nds.h"
#include "dual-layout-nds.h"
/* Defined later in main.c, declared here for enhanced geometry. */
static float display_scale(void);
static float display_y(void);

/* 0: classic game top / auxiliary bottom
 * 1: classic game bottom / auxiliary top
 * 2: enhanced scene top / dialogue or menu bottom. */
static unsigned nds_display_mode;
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
static bool nds_dual_has_choice(void){
 return !cpymo_ui_enabled(&engine) && engine.select_img.selections!=NULL;
}
/* Match CPyMO's actual 3DS behavior: engine UI background remains on
 * the upper screen; individual interactive elements live below. */
static bool nds_dual_is_ui(enum cpymo_backend_image_draw_type type){
 return type==cpymo_backend_image_draw_type_ui_element ||
        type==cpymo_backend_image_draw_type_ui_element_bg;
}
static bool nds_dual_accept(enum cpymo_backend_image_draw_type type){
 if(!nds_dual_active())return true;
 /* Mirror 3DS: UI background and non-interactive graphics above,
    interactive menu elements below; story choices below on NDS. */
 bool lower = cpymo_ui_enabled(&engine)?nds_dual_is_ui(type):
     (nds_dual_is_say(type) ||
      (nds_dual_has_choice() && type==cpymo_backend_image_draw_type_sel_img));
 if(lower != nds_dual_bottom_pass){
  nds_dual_state[nds_dual_bottom_pass?8:7]++;
  return false;
 }
 return true;
}
/* 3DS-style: original game assets retain their size, but the complete
 * dialogue/name group starts around 14 physical pixels down, not 20% of
 * the logical game height. Detect oversized assets and avoid clipping. */
static float nds_dual_say_offset(void){
 if(!nds_dual_active() || !nds_dual_bottom_pass ||
    cpymo_ui_enabled(&engine))return 0;
 float height=(float)engine.gameconfig.imagesize_h;
 float box_h=height*.25f;
 if(engine.say.msgbox && engine.say.msgbox_h>0 && engine.say.msgbox_w>0)
  box_h=(float)engine.say.msgbox_h *
      ((float)engine.gameconfig.imagesize_w/(float)engine.say.msgbox_w);
 float name_offset=(float)engine.gameconfig.nameboxorg_y / 360.f * height;
 return nds_dual_dialogue_shift(height,box_h,
     cpymo_gameconfig_font_size(&engine.gameconfig),name_offset,
     engine.say.name!=NULL,display_scale(),display_y());
}
static void nds_dual_filename(char* path,size_t size,const char* gamedir){
 snprintf(path,size,"%s/save/nds-dualscreen-mode.txt",gamedir);
}
static void nds_dual_load(const char* gamedir){
 nds_display_mode=0;
 nds_dual_mode=false;
 nds_dual_bottom_pass=false;
 char path[512];nds_dual_filename(path,sizeof path,gamedir);
 FILE* f=fopen(path,"rb");
 if(f){
  char line[48]={0};
  if(fgets(line,sizeof line,f))
   nds_display_mode=nds_display_pref_parse(line);
  fclose(f);
 }
 nds_dual_mode=nds_display_mode==2;
 nds_dual_state[1]=nds_display_mode;
 nds_dual_state[2]=0;
}
static void nds_dual_save(const char* gamedir){
 char path[512],line[32];
 nds_dual_filename(path,sizeof path,gamedir);
 bool ok=nds_display_pref_format(line,sizeof line,nds_display_mode);
 FILE* f=ok?fopen(path,"wb"):NULL;
 if(f){ok=fputs(line,f)!=EOF;if(fclose(f))ok=false;}
 else ok=false;
 nds_dual_state[4]++;
 if(!ok)nds_dual_state[5]++;
}
static void nds_display_start(void){
 nds_dual_mode=nds_display_mode==2;
 nds_dual_bottom_pass=false;
 /* The selected game may be launched after a previous game used lower LCD.
    Always establish a known physical mapping BEFORE applying its preference.
    Blind lcdSwap() would invert a still-swapped previous session. */
 lcdMainOnTop();screens_swapped=false;
 if(nds_display_mode==1){lcdSwap();screens_swapped=true;}
 presentation_state[1]=screens_swapped;
 nds_dual_state[1]=nds_display_mode;
}
static void nds_display_restore_launcher(void){
 /* Launcher always owns the regular upper-game/lower-helper assignment.
    Do not leak per-game LCD routing, split drawing or hidden helper state. */
 nds_dual_mode=false;
 nds_dual_bottom_pass=false;
 nds_aux_override=false;
 lcdMainOnTop();
 screens_swapped=false;
 presentation_state[1]=0;
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
