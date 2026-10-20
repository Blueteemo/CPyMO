/* DS front end, AGPL-3.0-or-later. Included after the shared font renderer. */
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#include "app-icon.h"
typedef struct {char path[256],title[256],platform[8];unsigned status;u16 width,height;bool title_ok,meta_ok,icon_ok,list_title_ok;char intro[2][128];unsigned char title_bits[350],meta_bits[1050],list_title_bits[400];u16 icon[24*24];} LauncherGame;
static LauncherGame launcher_games[64];
/* magic, visible, count, selected, launches, scan truncated, last init error */
volatile unsigned launcher_state[7]={0x4c41554e};
static unsigned launcher_count,launcher_selected;
static u16* launcher_sub;
static bool launcher_about,launcher_restored,launcher_language_menu;
/* Runtime CPU mode is distinct from console model and memory link layout.
 * An NTR binary does not gain 16 MiB merely because it runs on a DSi. */
static const char* nds_runtime_mode_label(void){
 if(!isDSiMode())return "NTR / DS mode · 4 MiB";
#ifdef CPYMO_DSI_MEMORY_LAYOUT
 return "TWL / DSi mode · 16 MiB";
#else
 return "TWL detected · DS layout";
#endif
}
static unsigned launcher_language_cursor;
static char launcher_font_path[256],launcher_icon_path[256];
static u16 selected_icon[48*48];static bool selected_icon_ok;
static u64 toast_started;
/* magic, about, font reads, title reads, restore success, selection writes, write errors */
volatile unsigned frontend_state[7]={0x4e465231};
/* magic, visible, presentations, completed library presentations */
volatile unsigned splash_state[4]={0x4e53504c};
static void launcher_remember(void){
 if(!launcher_count)return;mkdir("/pymo",0777);FILE* f=fopen("/pymo/last-game.txt","wb");bool ok=false;
 if(f){ok=fprintf(f,"%s\n",launcher_games[launcher_selected].path)>0;if(fclose(f))ok=false;}
 frontend_state[5]++;if(!ok)frontend_state[6]++;
}
static bool launcher_file(const char* dir,const char* suffix){char path[512];struct stat s;snprintf(path,sizeof path,"%s/%s",dir,suffix);return !stat(path,&s)&&S_ISREG(s.st_mode);}
static void launcher_add(const char* dir){
 if(!dir||dir[0]!='/'||strlen(dir)>=256||!launcher_file(dir,"gameconfig.txt"))return;
 for(unsigned i=0;i<launcher_count;i++)if(!strcasecmp(launcher_games[i].path,dir))return;
 if(launcher_count==64){launcher_state[5]=1;return;}
 LauncherGame* game=&launcher_games[launcher_count++];memset(game,0,sizeof *game);snprintf(game->path,sizeof game->path,"%s",dir);
 cpymo_gameconfig config;char path[512];snprintf(path,sizeof path,"%s/gameconfig.txt",dir);
 bool parsed=!cpymo_gameconfig_parse_from_file(&config,path);
 if(parsed){snprintf(game->title,sizeof game->title,"%s",config.gametitle);snprintf(game->platform,sizeof game->platform,"%s",config.platform);game->width=config.imagesize_w;game->height=config.imagesize_h;}
 else game->status=3;
 char* line=game->title;for(unsigned i=0;i<3;i++){
 char* next=strstr(line,"\\n");unsigned delimiter=2;if(!next){next=strchr(line,'\n');delimiter=1;}
 if(next)*next=0;if(i)snprintf(game->intro[i-1],sizeof game->intro[0],"%s",line);
 if(!next)break;line=next+delimiter;
 }
 if(!game->title[0])snprintf(game->title,sizeof game->title,"%s",strrchr(dir,'/')+1);
 if(!parsed)return;snprintf(path,sizeof path,"%s/nds-cache/format.txt",dir);FILE* f=fopen(path,"rb");if(f){char marker[32]={0};fread(marker,1,31,f);fclose(f);game->status=(!strcmp(marker,"CPYMO-NDS-CACHE-1\n")||!strcmp(marker,"CPYMO-NDS-CACHE-2\n")||!strcmp(marker,"CPYMO-NDS-CACHE-3\n")||!strcmp(marker,"CPYMO-NDS-CACHE-4\n")||!strcmp(marker,"CPYMO-NDS-CACHE-5\n")||!strcmp(marker,"CPYMO-NDS-CACHE-6\n"))&&launcher_file(dir,"nds-cache/font.bin")?1:2;}
 snprintf(path,sizeof path,"%s/nds-cache/launcher-title.bin",dir);f=fopen(path,"rb");if(f){
 struct {char magic[4];u16 w,h;} h;game->title_ok=fread(&h,1,8,f)==8&&!memcmp(h.magic,"NLT1",4)&&h.w==200&&h.h==14&&fread(game->title_bits,1,350,f)==350&&fgetc(f)==EOF;fclose(f);frontend_state[3]++;}
 snprintf(path,sizeof path,"%s/nds-cache/launcher-meta.bin",dir);f=fopen(path,"rb");if(f){
 struct {char magic[4];u16 w,h;} h;game->meta_ok=fread(&h,1,8,f)==8&&!memcmp(h.magic,"NLM1",4)&&h.w==200&&h.h==42&&fread(game->meta_bits,1,1050,f)==1050&&fgetc(f)==EOF;fclose(f);
 if(game->meta_ok){memcpy(game->title_bits,game->meta_bits,350);game->title_ok=true;}}
 snprintf(path,sizeof path,"%s/nds-cache/launcher-icon.bin",dir);f=fopen(path,"rb");if(f){
 struct {char magic[4];u16 w,h;} h;u16 row[48];bool ok=fread(&h,1,8,f)==8&&!memcmp(h.magic,"NLI1",4)&&h.w==48&&h.h==48;
 for(unsigned y=0;ok&&y<48;y++){ok=fread(row,2,48,f)==48;if(!(y&1))for(unsigned x=0;x<24;x++)game->icon[(y/2)*24+x]=row[x*2];}
 game->icon_ok=ok&&fgetc(f)==EOF;fclose(f);}


}
static void launcher_scan_dir(const char* root){
 DIR* d=opendir(root);if(!d)return;struct dirent* entry;char path[256];unsigned scanned=0;
 while((entry=readdir(d))&&scanned++<1024){if(entry->d_name[0]=='.')continue;int n=snprintf(path,sizeof path,"%s%s%s",root,!strcmp(root,"/")?"":"/",entry->d_name);if(n>0&&n<(int)sizeof path)launcher_add(path);}
 if(scanned>=1024)launcher_state[5]=1;closedir(d);
}
static int launcher_compare(const void* a,const void* b){return strcasecmp(((const LauncherGame*)a)->path,((const LauncherGame*)b)->path);}
static void launcher_scan(const char* hint){
 char previous[256]={0};if(launcher_count)snprintf(previous,sizeof previous,"%s",launcher_games[launcher_selected].path);
 launcher_count=0;launcher_state[5]=0;launcher_scan_dir("/pymo/games");launcher_scan_dir("/");launcher_add(hint);
 const char* configs[]={"/cpymo-nds/cache-game-path.txt","/cpymo-nds/game-path.txt"};
 for(unsigned i=0;i<2;i++){FILE* f=fopen(configs[i],"rb");if(f){char path[256];if(fgets(path,sizeof path,f)){path[strcspn(path,"\r\n")]=0;launcher_add(path);}fclose(f);}}
 qsort(launcher_games,launcher_count,sizeof *launcher_games,launcher_compare);if(launcher_selected>=launcher_count)launcher_selected=0;launcher_state[2]=launcher_count;
 if(!launcher_restored){FILE* f=fopen("/pymo/last-game.txt","rb");if(f){if(fgets(previous,sizeof previous,f))previous[strcspn(previous,"\r\n")]=0;fclose(f);}launcher_restored=true;}
 for(unsigned i=0;i<launcher_count;i++)if(!strcmp(previous,launcher_games[i].path)){launcher_selected=i;frontend_state[4]=1;break;}

}
static void launcher_rect(int x,int y,int w,int h,u16 color){for(int yy=y;yy<y+h&&yy<192;yy++)for(int xx=x;xx<x+w&&xx<256;xx++)if(xx>=0&&yy>=0)framebuffer[yy*256+xx]=color|BIT(15);}
static void launcher_text_mode(const char* text,int x,int y,u16 color,int max_x,unsigned mode){
 const char* p=text;int start=x;while(*p){unsigned cp=decode(&p);if(cp==13)continue;if(cp==10|| (cp=='\\'&&*p=='n')){if(cp=='\\')p++;x=start;y+=17;continue;}if(y>178)break;
 int g=text_glyph(mode,cp),fallback=g<0?ui_fallback_glyph(cp):-1;if(g<0&&fallback<0)g=text_glyph(mode,'?');if(g<0&&fallback<0)continue;unsigned advance=fallback>=0?ui_fallback_advances[fallback]:text_advance(mode,g);if(x+(int)advance>max_x){while(*p&&*p!='\n')p++;continue;}
 for(int yy=0;yy<(fallback>=0?16:(int)font_height(mode));yy++){unsigned bits=fallback>=0?ui_fallback_rows[fallback][yy]:text_row(mode,g,yy);for(int xx=0;xx<(fallback>=0?16:(int)font_width(mode));xx++)if(bits&(1u<<xx))launcher_rect(x+xx,y+yy,1,1,color);}x+=advance;}
}
static void launcher_text(const char* text,int x,int y,u16 color,int max_x){launcher_text_mode(text,x,y,color,max_x,nds_font_choices_available()?1:0);}
static void launcher_publish(u16* dest){DC_FlushRange(framebuffer,sizeof framebuffer);dmaCopy(framebuffer,dest,sizeof framebuffer);}
/* Keep both LCDs black while changing VRAM roles. Old console font/tile data
   must never be exposed as RGB555 pixels, including during FAT/library I/O. */
static void launcher_video_init(void){
 setBrightness(3,-16);lcdMainOnTop();screens_swapped=false;presentation_state[1]=0;
 videoSetMode(MODE_5_2D);vramSetBankA(VRAM_A_MAIN_BG);int main_bg=bgInit(3,BgType_Bmp16,BgSize_B16_256x256,0,0);vram=bgGetGfxPtr(main_bg);
 dmaFillHalfWords(BIT(15),vram,256*256*2);
 videoSetModeSub(MODE_5_2D);vramSetBankC(VRAM_C_SUB_BG);int sub_bg=bgInitSub(3,BgType_Bmp16,BgSize_B16_256x256,0,0);launcher_sub=bgGetGfxPtr(sub_bg);
 dmaFillHalfWords(BIT(15),launcher_sub,256*256*2);
}
static void launcher_video_show(void){swiWaitForVBlank();setBrightness(3,0);}
static bool launcher_title_bitmap(const LauncherGame* game,int x,int y,u16 color){
 if(!game->title_ok)return false;for(unsigned yy=0;yy<14;yy++)for(unsigned xx=0;xx<200;xx++)if(game->title_bits[yy*25+xx/8]&(1u<<(xx%8)))launcher_rect(x+xx,y+yy,1,1,color);return true;
}
static void launcher_list_title(LauncherGame* game,int x,int y,u16 color){
 /* Use the exact native 12px dialogue font, including its full glyph grid.
    Older on-card title/meta bitmaps may still contain a pre-pixel-font style.
    Generate once from this game's own subset, never the selected game's. */
 if(!game->list_title_ok){
  PixelFont own={0};
  if(read_pixel_font(&own,game->path,"small",12)){
   memset(game->list_title_bits,0,sizeof game->list_title_bits);const char* p=game->title;unsigned pen=0;
   while(*p){unsigned cp=decode(&p);int lo=0,hi=own.count-1,found=-1;
    while(lo<=hi){int mid=(lo+hi)/2;if(own.data[mid].code==cp){found=mid;break;}if(own.data[mid].code<cp)lo=mid+1;else hi=mid-1;}
    int fallback=found<0?ui_fallback_glyph(cp):-1;
    if(found<0&&fallback<0){fallback=ui_fallback_glyph('?');if(fallback<0)continue;}
    unsigned advance=found>=0?own.data[found].advance:ui_fallback_advances[fallback];if(pen+advance>200)break;
    for(unsigned yy=0;yy<16;yy++){unsigned bits=found>=0?own.data[found].rows[yy]:ui_fallback_rows[fallback][yy];
     for(unsigned xx=0;xx<16&&pen+xx<200;xx++)if(bits&(1u<<xx))game->list_title_bits[yy*25+(pen+xx)/8]|=1u<<((pen+xx)%8);}
    pen+=advance;
   }
   free(own.data);game->list_title_ok=true;
  }
 }
 if(game->list_title_ok){for(unsigned yy=0;yy<16;yy++)for(unsigned xx=0;xx<200;xx++)if(game->list_title_bits[yy*25+xx/8]&(1u<<(xx%8)))launcher_rect(x+xx,y+yy,1,1,color);}
 else if(!launcher_title_bitmap(game,x,y,color))launcher_text_mode(game->title,x,y,color,244,0);
}
/* Fit complete UTF-8 glyphs on one line, reserving dots when text overflows. */
static void launcher_line(const char* text,int x,int y,u16 color,int max_x,unsigned mode){
 const char* p=text;unsigned width=0;while(*p&&*p!='\n'&&*p!='\r'&&!(p[0]=='\\'&&p[1]=='n')){unsigned cp=decode(&p);int g=text_glyph(mode,cp);if(g<0)g=text_glyph(mode,'?');if(g>=0)width+=text_advance(mode,g);}
 if(width<=(unsigned)(max_x-x)){launcher_text_mode(text,x,y,color,max_x,mode);return;}
 int dot=text_glyph(mode,'.');unsigned dots=dot>=0?3*text_advance(mode,dot):0;int limit=max_x-x-(int)dots;
 char fitted[256];unsigned used=0;int pixels=0;p=text;
 while(*p&&*p!='\n'&&*p!='\r'&&!(p[0]=='\\'&&p[1]=='n')){const char* begin=p;unsigned cp=decode(&p);int g=text_glyph(mode,cp);if(g<0)g=text_glyph(mode,'?');unsigned advance=g>=0?text_advance(mode,g):0;unsigned bytes=p-begin;if(pixels+(int)advance>limit||used+bytes+4>sizeof fitted)break;memcpy(fitted+used,begin,bytes);used+=bytes;pixels+=advance;}
 memcpy(fitted+used,dots?"...":"",dots?4:1);launcher_text_mode(fitted,x,y,color,max_x,mode);
}
static void launcher_title(const LauncherGame* game,int x,int y,u16 color,int max_x){
 if(nds_font_choices_available())launcher_line(game->title,x,y,color,max_x,2);
 else if(!launcher_title_bitmap(game,x,y,color))launcher_line(game->title,x,y,color,max_x,0);
}
static void launcher_background(void){for(unsigned i=0;i<256*192;i++)framebuffer[i]=RGB15(3,5,8)|BIT(15);}
static void launcher_header(const char* title){launcher_rect(0,0,256,28,RGB15(5,10,16));launcher_text(title,10,7,RGB15(31,31,31),248);}
static void launcher_icon(int x,int y){for(int yy=0;yy<32;yy++)for(int xx=0;xx<32;xx++){u16 c=app_icon[yy*32+xx];if(c)launcher_rect(x+xx,y+yy,1,1,c);}}
/* ROM-resident glyphs work before FAT or a game's font cache is available. */
static void launcher_splash_text(const char* text,int y,bool large,u16 color){
 const char* p=text;unsigned width=0;while(*p){int g=ui_fallback_glyph(decode(&p));if(g>=0)width+=large?ui_large_advances[g]:ui_fallback_advances[g];}
 int x=(256-(int)width)/2;p=text;while(*p){int g=ui_fallback_glyph(decode(&p));if(g<0)continue;
 for(int yy=0;yy<16;yy++){unsigned bits=large?ui_large_rows[g][yy]:ui_fallback_rows[g][yy];for(int xx=0;xx<16;xx++)if(bits&(1u<<xx))launcher_rect(x+xx,y+yy,1,1,color);}
 x+=large?ui_large_advances[g]:ui_fallback_advances[g];}
}
static void launcher_show_splash(void){
 launcher_background();
 for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++){u16 c=app_icon[y*32+x];if(c)launcher_rect(96+x*2,38+y*2,2,2,c);}
 launcher_splash_text("CPyMO for NDS",121,true,RGB15(31,31,31));launcher_publish(vram);
 launcher_background();launcher_splash_text(nds_ui("正在读取游戏列表..."),88,false,RGB15(28,29,31));
 launcher_splash_text(nds_runtime_mode_label(),166,false,RGB15(19,22,25));launcher_publish(launcher_sub);
 /* Both planes are complete already. No timer, key wait or extra VBlank. */
 setBrightness(3,0);splash_state[1]=1;splash_state[2]++;
}
static void launcher_selected_icon(void){
 if(!launcher_count)return;LauncherGame* g=&launcher_games[launcher_selected];if(!strcmp(launcher_icon_path,g->path))return;
 snprintf(launcher_icon_path,sizeof launcher_icon_path,"%s",g->path);selected_icon_ok=false;
 char path[512];snprintf(path,sizeof path,"%s/nds-cache/launcher-icon.bin",g->path);FILE* f=fopen(path,"rb");if(f){
 struct {char magic[4];u16 w,h;} h;selected_icon_ok=fread(&h,1,8,f)==8&&!memcmp(h.magic,"NLI1",4)&&h.w==48&&h.h==48&&fread(selected_icon,2,48*48,f)==48*48&&fgetc(f)==EOF;fclose(f);}
 presentation_state[6]=selected_icon_ok;presentation_state[7]=1+(g->intro[0][0]!=0)+(g->intro[1][0]!=0);
}
static void launcher_game_icon(const LauncherGame* game,int x,int y,bool large){
 if(large&&selected_icon_ok){for(unsigned yy=0;yy<48;yy++)for(unsigned xx=0;xx<48;xx++){u16 c=selected_icon[yy*48+xx];if(c&BIT(15))launcher_rect(x+xx,y+yy,1,1,c);}}
 else if(!large&&game->icon_ok){for(unsigned yy=0;yy<24;yy++)for(unsigned xx=0;xx<24;xx++){u16 c=game->icon[yy*24+xx];if(c&BIT(15))launcher_rect(x+xx,y+yy,1,1,c);}}
 else if(large)launcher_icon(x+8,y+8);
 else {for(unsigned yy=0;yy<24;yy++)for(unsigned xx=0;xx<24;xx++){u16 c=app_icon[(yy*32/24)*32+xx*32/24];if(c)launcher_rect(x+xx,y+yy,1,1,c);}}
}
static void launcher_intro(const LauncherGame* game,unsigned line,int x,int y,u16 color){
 /* Small dialogue font on its native grid; title remains the visual anchor. */
 launcher_line(game->intro[line-1],x,y,color,245,nds_font_choices_available()?1:0);
}
static bool nds_aux_clock_suppressed;
static void nds_clock_patch_aux(bool show){
 if(debug_trace||!launcher_sub||nds_aux_hidden())return;
 for(unsigned y=96;y<114;y++)for(unsigned x=196;x<244;x++)launcher_sub[y*256+x]=RGB15(3,5,8)|BIT(15);
 if(show){nds_clock_paint(launcher_sub,196,96,RGB15(21,24,27));clock_state[7]++;}
}
static void nds_clock_sync_aux_visibility(void){
 bool suppress=nds_menu_clock_visible;
 if(suppress==nds_aux_clock_suppressed)return;
 nds_aux_clock_suppressed=suppress;nds_clock_patch_aux(!suppress);
}
static void launcher_play_panel(const char* message){
 if(nds_aux_hidden()){if(launcher_sub)dmaFillHalfWords(BIT(15),launcher_sub,256*192*2);return;}
 u16 white=RGB15(31,31,31),muted=RGB15(19,22,25),accent=RGB15(10,24,31);
 launcher_background();launcher_header("CPyMO for NDS   " CPYMO_NDS_VERSION);
 launcher_text(nds_ui("A / 轻触：继续"),12,32,white,183);launcher_text(nds_ui("X：菜单 · B：返回"),12,50,white,183);launcher_text(nds_ui("B：按住隐藏文字"),12,68,white,183);
 if(launcher_count)launcher_game_icon(&launcher_games[launcher_selected],196,40,true);
 launcher_text(nds_ui("R：按设置快进"),12,86,white,245);launcher_text(nds_ui("L：交换上下屏"),12,104,white,245);launcher_text(nds_ui("Y：隐藏／唤回辅助屏"),12,122,white,245);
 launcher_text(nds_ui("START：快速保存到存档1"),12,140,muted,245);launcher_text(nds_ui("SELECT：快速读取存档1"),12,158,muted,245);
 launcher_text(message?message:nds_ui("显示、字号、音量：X 菜单"),12,176,accent,245);
 nds_clock_sample();nds_aux_clock_suppressed=nds_menu_clock_visible;
 if(!nds_aux_clock_suppressed)nds_clock_paint(framebuffer,196,96,RGB15(21,24,27));clock_state[6]++;
 launcher_publish(launcher_sub);
}
static void nds_clock_poll(void){
 nds_clock_sample();if(!nds_clock_pending)return;nds_clock_pending=false;
 if(nds_menu_clock_visible&&cpymo_ui_enabled(&engine))cpymo_engine_request_redraw(&engine);
 if(nds_aux_hidden()){clock_state[8]++;return;}
 if(!nds_menu_clock_visible)nds_clock_patch_aux(true);
}
static void launcher_toast(const char* message){toast_started=tickGetCount();presentation_state[2]=1;presentation_state[3]++;presentation_state[5]=0;launcher_play_panel(nds_ui(message));}
static void launcher_update_toast(void){if(presentation_state[2]){u64 elapsed=tickGetCount()-toast_started;presentation_state[5]=elapsed*1000/TICK_FREQ;if(elapsed>=2*TICK_FREQ){presentation_state[2]=0;presentation_state[4]++;launcher_play_panel(NULL);}}}
/* Error rendering is ROM-only: it never consults game fonts or FAT. */
volatile unsigned error_ui_state[8]={0x4e455231};
static unsigned launcher_rom_width(const char* text,bool large){
 unsigned w=0;while(*text&&*text!='\n'){unsigned cp=decode(&text);int g=ui_fallback_glyph(cp);if(g<0)g=ui_fallback_glyph('?');if(g>=0)w+=large?ui_large_advances[g]:ui_fallback_advances[g];}return w;
}
static void launcher_rom_box(const char* text,int x,int y,int width,int bottom,bool large,u16 color){
 int left=x,right=x+width;const char* p=text;unsigned lines=1;
 while(*p){unsigned cp=decode(&p);if(cp==13)continue;
  if(cp==10){x=left;y+=18;lines++;continue;}
  int g=ui_fallback_glyph(cp);if(g<0){error_ui_state[5]++;g=ui_fallback_glyph('?');}if(g<0)continue;
  unsigned advance=large?ui_large_advances[g]:ui_fallback_advances[g];
  if(x+(int)advance>right){x=left;y+=18;lines++;if(cp==' ')continue;}
  if(y+16>bottom){error_ui_state[6]++;break;}
  for(unsigned yy=0;yy<16;yy++){unsigned bits=large?ui_large_rows[g][yy]:ui_fallback_rows[g][yy];
   for(unsigned xx=0;xx<16;xx++)if((bits&(1u<<xx))&&x+(int)xx<right)launcher_rect(x+xx,y+yy,1,1,color);}
  x+=advance;
 }error_ui_state[7]+=lines;
}
static void launcher_rom_center(const char* text,int y,bool large,u16 color){
 unsigned width=launcher_rom_width(text,large);int x=width<=224?(256-(int)width)/2:16;
 launcher_rom_box(text,x,y,256-16-x,164,large,color);
}
static void launcher_failure_hint(const char* message,int error,const char* hint){
 splash_state[1]=0;bool fatal=!strcmp(message,"无法访问存储卡");
 /* Errors must remain readable after L swapped the game or Y hid its panel.
    This affects errors only; normal game screen routing is unchanged. */
 error_ui_active=true;
 if(nds_playing){lcdMainOnTop();screens_swapped=false;presentation_state[1]=0;nds_aux_refresh();}
 error_ui_state[1]=fatal;error_ui_state[2]=(unsigned)error;error_ui_state[3]=nds_language_effective();
 error_ui_state[5]=error_ui_state[6]=error_ui_state[7]=0;
 launcher_background();launcher_rect(0,0,256,28,RGB15(5,10,16));
 launcher_rom_box("CPyMO for NDS",10,7,236,28,false,RGB15(31,31,31));launcher_icon(112,43);
 launcher_rom_center(nds_ui(message),90,true,RGB15(31,31,31));
 char code[64];snprintf(code,sizeof code,nds_ui("错误码：%d"),error);
 launcher_rom_center(code,143,false,RGB15(31,23,18));launcher_publish(vram);
 launcher_background();launcher_rect(0,0,256,28,RGB15(5,10,16));
 launcher_rom_box(nds_ui("处理建议"),10,7,236,28,false,RGB15(31,31,31));
 const char* advice=fatal?"关机后重新插好TF卡。\n检查读卡与R4内核文件，再重试。\n此错误码不代表TF卡已损坏。":"请检查转换结果和游戏资源。\n原始错误码已保留，便于反馈。";
 launcher_rom_box(nds_ui(advice),16,46,224,155,false,RGB15(27,28,30));
 launcher_rom_box(nds_ui(hint),16,169,224,192,false,RGB15(10,24,31));launcher_publish(launcher_sub);error_ui_state[4]++;
}
static void launcher_failure(const char* message,int error){launcher_failure_hint(message,error,"按 B 返回游戏资料库");}
static void launcher_library_clock(u16* pixels,bool erase){
 /* Header-only minute patch: never re-read game metadata or repaint the list. */
 if(erase)for(int y=4;y<22;y++)for(int x=200;x<248;x++)pixels[y*256+x]=RGB15(5,10,16)|BIT(15);
 nds_clock_paint_impl(pixels,200,4,RGB15(26,28,30),false);
}
static void launcher_draw(const char* message){
 if(launcher_count&&launcher_games[launcher_selected].status==1&&strcmp(launcher_font_path,launcher_games[launcher_selected].path)){if(load_game_font(launcher_games[launcher_selected].path)&&load_pixel_fonts(launcher_games[launcher_selected].path)){snprintf(launcher_font_path,sizeof launcher_font_path,"%s",launcher_games[launcher_selected].path);frontend_state[2]++;}}
 u16 white=RGB15(31,31,31),muted=RGB15(19,22,25),accent=RGB15(10,24,31);
 launcher_selected_icon();
 launcher_background();launcher_header("CPyMO for NDS   " CPYMO_NDS_VERSION);
 if(launcher_about){launcher_icon(12,40);launcher_text(nds_ui("CPyMO / PYMO"),53,47,white,246);launcher_text(nds_ui("NDS 平台移植：千岚之夏"),12,78,white,245);launcher_text(nds_ui("引擎与后端：AGPL v3"),12,101,muted,245);launcher_text(nds_ui("转换工具：MIT"),12,124,muted,245);launcher_text(nds_ui("许可详情请见附带文档"),12,147,muted,245);}
 else if(launcher_count){LauncherGame* game=&launcher_games[launcher_selected];launcher_game_icon(game,12,39,true);launcher_text(nds_ui("游戏资料库"),74,46,accent,245);
 launcher_title(game,12,90,white,245);
 launcher_intro(game,1,12,114,RGB15(26,27,29));launcher_intro(game,2,12,132,RGB15(26,27,29));
 const char* path=game->path;while(strlen(path)>36)path++;launcher_text(path,12,153,muted,245);
 launcher_text(nds_ui(game->status==1?"已转换，可以启动":game->status==2?"缓存无效，请重新转换":game->status==3?"游戏配置无效":"需要先使用转换脚本"),12,175,accent,245);}
 else {launcher_text(nds_ui("没有找到游戏"),12,58,white,245);launcher_text(nds_ui("请先转换游戏，再放入"),12,88,muted,245);launcher_text(nds_ui("/pymo/games/"),12,114,accent,245);}
 if(!launcher_count||launcher_about)launcher_text(nds_ui(nds_runtime_mode_label()),12,177,muted,245);launcher_publish(vram);
 launcher_background();
 if(!launcher_about&&!launcher_language_menu){
  launcher_rect(0,0,256,28,RGB15(5,10,16));
  launcher_line(nds_ui("选择游戏"),10,7,white,194,1);
  nds_clock_sample();launcher_library_clock(framebuffer,false);
 }else launcher_header(nds_ui(launcher_about?"关于与许可":"界面语言"));
 if(launcher_language_menu){
 for(unsigned i=0;i<NDS_LANG_COUNT;i++){int y=34+i*25;launcher_rect(8,y,240,23,i==launcher_language_cursor?RGB15(7,15,23):RGB15(4,7,11));launcher_text(nds_language_choice_name(i),18,y+3,white,240);}
 launcher_text(nds_ui("上下选择 · A 确定 · B 返回"),7,169,accent,250);
 }
 else if(launcher_about){
 launcher_text(nds_ui("PyMO：chen_xin_ming"),10,47,white,248);launcher_text(nds_ui("CPyMO：Seng Jik 及贡献者"),10,75,muted,248);launcher_text(nds_ui("游戏与汉化署名照原样保留"),10,103,muted,248);launcher_text(nds_ui("新游戏请先使用转换助手"),10,131,muted,248);
 launcher_text(nds_ui("按 B 返回游戏列表"),10,169,accent,248);}

 else {unsigned first=launcher_selected/4*4;
 for(unsigned row=0;row<4&&first+row<launcher_count;row++){unsigned index=first+row;int y=33+row*31;LauncherGame* game=&launcher_games[index];
 launcher_rect(6,y,244,28,index==launcher_selected?RGB15(7,15,23):RGB15(4,7,11));launcher_game_icon(game,10,y+2,false);
 launcher_list_title(game,40,y+6,game->status==1?white:muted);}
 launcher_text(message&&*message?nds_ui(message):nds_ui("A 启动 · 上下选择 · SELECT 刷新"),7,160,accent,250);launcher_text(nds_ui("轻触两次启动 · B 关于 · X 语言"),7,178,muted,250);}
 launcher_publish(launcher_sub);launcher_state[3]=launcher_selected;frontend_state[1]=launcher_about;
 if(splash_state[1]){splash_state[1]=0;splash_state[3]++;}
}
static void launcher_release_keys(void){do{nds_power_poll();swiWaitForVBlank();scanKeys();}while(keysHeld()&(KEY_A|KEY_B|KEY_X|KEY_Y|KEY_L|KEY_TOUCH|KEY_START|KEY_SELECT));}
static void launcher_choose(char* out,size_t size,const char* hint){
 error_ui_active=false;
 if(!splash_state[1]){launcher_video_init();launcher_show_splash();}
 nds_playing=false;nds_aux_override=false;nds_aux_refresh();lcdMainOnTop();screens_swapped=false;presentation_state[1]=0;presentation_state[2]=0;launcher_icon_path[0]=0;
 free(game_glyphs);game_glyphs=NULL;game_glyph_count=0;launcher_font_path[0]=0;free_pixel_fonts();clear_menu_images();nds_cache_enabled=false;
 nds_menu_clock_visible=false;clock_state[9]=0;
 launcher_about=false;launcher_language_menu=false;launcher_state[1]=1;launcher_scan(hint);launcher_draw(NULL);launcher_video_show();launcher_release_keys();
 while(1){nds_power_poll();swiWaitForVBlank();scanKeys();int down=keysDown();bool redraw=false,start=false;
 if(nds_clock_sample()&&!launcher_about&&!launcher_language_menu)launcher_library_clock(launcher_sub,true);
 if(down&KEY_X){launcher_language_menu=!launcher_language_menu;launcher_about=false;launcher_language_cursor=nds_language_choice_get();launcher_draw(NULL);continue;}
 if(down&KEY_B){if(launcher_language_menu)launcher_language_menu=false;else launcher_about=!launcher_about;launcher_draw(NULL);continue;}
 if(launcher_language_menu){
  if(!(down&(KEY_UP|KEY_DOWN|KEY_A)))continue;
  if(down&KEY_UP)launcher_language_cursor=(launcher_language_cursor+NDS_LANG_COUNT-1)%NDS_LANG_COUNT;
  if(down&KEY_DOWN)launcher_language_cursor=(launcher_language_cursor+1)%NDS_LANG_COUNT;
  const char* notice=NULL;if(down&KEY_A){notice=nds_language_choice_set(launcher_language_cursor)?"语言设置已保存":"设置保存失败";nds_language_take_refresh();launcher_language_menu=false;}
  launcher_draw(notice);continue;
 }
 if(launcher_about)continue;
 if(down&KEY_SELECT){launcher_font_path[0]=0;launcher_scan(hint);redraw=true;}
 if(launcher_count){if(down&KEY_UP){launcher_selected=(launcher_selected+launcher_count-1)%launcher_count;redraw=true;}if(down&KEY_DOWN){launcher_selected=(launcher_selected+1)%launcher_count;redraw=true;}
 if(down&KEY_LEFT){launcher_selected=launcher_selected>=4?launcher_selected-4:0;redraw=true;}if(down&KEY_RIGHT){launcher_selected+=4;if(launcher_selected>=launcher_count)launcher_selected=launcher_count-1;redraw=true;}
 if(down&KEY_TOUCH){touchPosition t;touchRead(&t);if(t.py>=33&&t.py<157){unsigned index=launcher_selected/4*4+(t.py-33)/31;if(index<launcher_count){start=index==launcher_selected;launcher_selected=index;redraw=true;}}}
 if(down&KEY_A)start=true;
 if(start){LauncherGame* game=&launcher_games[launcher_selected];if(game->status==1){launcher_remember();snprintf(out,size,"%s",game->path);launcher_release_keys();launcher_state[1]=0;launcher_state[4]++;return;}launcher_draw("请在电脑上先转换或修复缓存");continue;}}
 if(redraw)launcher_draw(NULL);
 }
}
