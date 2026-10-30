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
static u16 nds_theme_bg(void){
 int i=nds_theme_active_index();if(i<0)return RGB15(3,5,8);
 const unsigned char* c=nds_ipl_theme_rgb[i];
 return RGB15(2+c[0]/10,3+c[1]/10,4+c[2]/10);
}
static u16 nds_theme_header(void){
 int i=nds_theme_active_index();if(i<0)return RGB15(5,10,16);
 const unsigned char* c=nds_ipl_theme_rgb[i];
 return RGB15(4+c[0]/7,7+c[1]/7,11+c[2]/7);
}
static u16 nds_theme_card(void){
 int i=nds_theme_active_index();if(i<0)return RGB15(4,7,11);
 const unsigned char* c=nds_ipl_theme_rgb[i];
 return RGB15(3+c[0]/11,5+c[1]/11,8+c[2]/11);
}
static u16 nds_theme_selected(void){
 int i=nds_theme_active_index();if(i<0)return RGB15(7,15,23);
 const unsigned char* c=nds_ipl_theme_rgb[i];
 return RGB15(6+c[0]/6,11+c[1]/6,16+c[2]/6);
}
static u16 nds_theme_accent(void){
 int i=nds_theme_active_index();if(i<0)return RGB15(10,24,31);
 const unsigned char* c=nds_ipl_theme_rgb[i];
 return RGB15(14+c[0]*17/31,14+c[1]*17/31,14+c[2]*17/31);
}
#endif
