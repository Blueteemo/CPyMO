/* v1.2.1 test: NDS/DS Lite IPL theme palette, no game-asset changes. */
#ifndef CPYMO_NDS_THEME_H
#define CPYMO_NDS_THEME_H
#include <calico/nds/env.h>
/* NITRO Programming Manual, IPL favorite colors: 5-bit RGB 0..15. */
static const unsigned char nds_ipl_theme_rgb[16][3]={
 {12,16,19},{23,9,0},{31,0,3},{31,17,31},
 {31,18,0},{30,28,0},{21,31,0},{0,31,0},
 {0,20,7},{9,27,17},{6,23,30},{0,11,30},
 {0,0,18},{17,0,26},{26,0,29},{31,0,18}
};
static const char* const nds_ipl_theme_names[4][16]={
 {"灰蓝","棕色","红色","粉色","橙色","黄色","青柠","绿色","深绿","青绿","浅蓝","蓝色","深蓝","深紫","淡紫","洋红"},
 {"Grey blue","Brown","Red","Pink","Orange","Yellow","Lime","Green","Dark green","Sea green","Cyan","Blue","Dark blue","Purple","Violet","Magenta"},
 {"灰藍","棕色","紅色","粉色","橘色","黃色","青檸","綠色","深綠","青綠","淺藍","藍色","深藍","深紫","淡紫","洋紅"},
 {"グレー","ブラウン","赤","ピンク","オレンジ","黄色","ライム","緑","深緑","青緑","水色","青","濃紺","紫","薄紫","マゼンタ"}
};
static unsigned nds_theme_preference; /* 0=default, 1=firmware, 2..17=IPL colors */
static unsigned nds_theme_firmware(void){
 if(g_envUserSettings->version!=5||PersonalData->settingsLost||PersonalData->theme>15)return 16;
 return PersonalData->theme;
}
static int nds_theme_active_index(void){
 if(!nds_theme_preference)return -1;
 if(nds_theme_preference==1){unsigned v=nds_theme_firmware();return v<16?(int)v:-1;}
 return nds_theme_preference<=17?(int)nds_theme_preference-2:-1;
}
static void nds_theme_load(void){
 nds_theme_preference=0;FILE* f=fopen("/pymo/nds-theme.cfg","rb");
 if(!f)return;
 char line[32]={0};unsigned v=18;int n=0;
 if(fgets(line,sizeof line,f)&&sscanf(line,"NDSTHEME1 %u %n",&v,&n)==1
    && n>0&&line[n]==0&&fgetc(f)==EOF&&v<=17)nds_theme_preference=v;
 fclose(f);
}
static int nds_theme_set(unsigned v){
 if(v>17)return 0;
 mkdir("/pymo",0777);FILE* f=fopen("/pymo/nds-theme.cfg","wb");int ok=0;
 if(f){ok=fprintf(f,"NDSTHEME1 %u\n",v)>0;if(fclose(f))ok=0;}
 if(ok)nds_theme_preference=v;return ok;
}
static unsigned nds_theme_get(void){return nds_theme_preference;}
static u16 nds_theme_color(unsigned id){
 if(id>15)id=0;
 return RGB15(nds_ipl_theme_rgb[id][0],nds_ipl_theme_rgb[id][1],nds_ipl_theme_rgb[id][2]);
}
static const char* nds_theme_choice_name(unsigned v){
 if(v==0)return nds_ui("默认");if(v==1)return nds_ui("自动");
 if(v>17)return "?";
 return nds_ipl_theme_names[nds_language_effective()][v-2];
}
/* 1.2.1-test2: independent IPL color families rather than blue overlays.
 * Default is byte-identical to 1.2.0. All 16 IPL colors use neutral, not
 * blue, shading; selected rows keep >4.5:1 white-text contrast. */
static u16 nds_theme_tint(unsigned idx,unsigned strength,unsigned neutral){
 const unsigned char* c=nds_ipl_theme_rgb[idx];
 return RGB15(neutral+(c[0]*strength+15)/31,
              neutral+(c[1]*strength+15)/31,
              neutral+(c[2]*strength+15)/31);
}
static u16 nds_theme_bg(void){
 int i=nds_theme_active_index();
 return i<0?RGB15(3,5,8):nds_theme_tint((unsigned)i,6,1);
}
static u16 nds_theme_header(void){
 int i=nds_theme_active_index();
 return i<0?RGB15(5,10,16):nds_theme_tint((unsigned)i,11,3);
}
static u16 nds_theme_card(void){
 int i=nds_theme_active_index();
 return i<0?RGB15(4,7,11):nds_theme_tint((unsigned)i,9,3);
}
static u16 nds_theme_selected(void){
 int i=nds_theme_active_index();
 return i<0?RGB15(7,15,23):nds_theme_tint((unsigned)i,11,4);
}
static u16 nds_theme_accent(void){
 int i=nds_theme_active_index();
 return i<0?RGB15(10,24,31):nds_theme_tint((unsigned)i,15,16);
}
static u16 nds_theme_muted(void){
 return nds_theme_active_index()<0?RGB15(19,22,25):RGB15(22,22,22);
}
static u16 nds_theme_detail(void){
 return nds_theme_active_index()<0?RGB15(26,27,29):RGB15(27,27,27);
}
static u16 nds_theme_clock_ink(void){
 return nds_theme_active_index()<0?RGB15(26,28,30):RGB15(28,28,28);
}
static u16 nds_theme_aux_clock_ink(void){
 return nds_theme_active_index()<0?RGB15(21,24,27):RGB15(24,24,24);
}
#endif
