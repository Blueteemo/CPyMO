/* DS wall clock, AGPL-3.0-or-later. Calico's ARM7 RTC service supplies time().
   Its epoch represents the user's local DS clock; never apply a second timezone. */
#include <time.h>
/* magic, valid, minute, reads, changes, menu paints, panel paints, minute patches,
   hidden updates, menu visible, hour, minute-of-hour */
volatile unsigned clock_state[12]={0x4e434c4b};
static char nds_clock_text[6]="--:--";
static u64 nds_clock_checked;
static bool nds_clock_initialized,nds_clock_pending,nds_menu_clock_visible,nds_registering_rmenu;
static cpymo_ui_drawer nds_rmenu_original_drawer;
static bool nds_clock_sample(void){
 u64 tick=tickGetCount();
 if(nds_clock_initialized&&tick-nds_clock_checked<TICK_FREQ)return false;
 nds_clock_checked=tick;clock_state[3]++;
 time_t stamp=time(NULL);
 bool valid=(u64)stamp>=946684800ULL&&(u64)stamp<4102444800ULL;
 unsigned minute=valid?(unsigned)((u64)stamp/60):0;
 bool changed=!nds_clock_initialized||valid!=clock_state[1]||minute!=clock_state[2];
 nds_clock_initialized=true;
 if(!changed)return false;
 nds_clock_pending=true;clock_state[1]=valid;clock_state[2]=minute;clock_state[4]++;
 if(valid){unsigned hour=(minute/60)%24,min=minute%60;
  nds_clock_text[0]='0'+hour/10;nds_clock_text[1]='0'+hour%10;
  nds_clock_text[2]=':';nds_clock_text[3]='0'+min/10;nds_clock_text[4]='0'+min%10;
  clock_state[10]=hour;clock_state[11]=min;
 }else{memcpy(nds_clock_text,"--:--",6);clock_state[10]=clock_state[11]=0;}
 return true;
}
typedef struct {
 int glyphs[5],advance,left,right,top,bottom;
} NdsClockLayout;

static NdsClockLayout nds_clock_layout(void){
 NdsClockLayout layout={.left=256,.top=16};
 for(unsigned i=0;i<5;++i){
  int g=layout.glyphs[i]=ui_fallback_glyph(nds_clock_text[i]);
  if(g<0)continue;
  for(int row=0;row<16;++row){
   unsigned bits=ui_fallback_rows[g][row];
   if(!bits)continue;
   int left=layout.advance+__builtin_ctz(bits);
   int right=layout.advance+32-__builtin_clz(bits);
   if(left<layout.left)layout.left=left;
   if(right>layout.right)layout.right=right;
   if(row<layout.top)layout.top=row;
   if(row>=layout.bottom)layout.bottom=row+1;
  }
  layout.advance+=ui_fallback_advances[g];
 }
 return layout;
}
static void nds_clock_draw(u16* pixels,const NdsClockLayout* layout,int pen,int top,u16 ink){
 ink|=BIT(15);
 for(unsigned i=0;i<5;++i){
  int g=layout->glyphs[i];
  if(g<0)continue;
  for(int row=0;row<16;++row){
   unsigned bits=ui_fallback_rows[g][row];
   while(bits){
    int column=pen+__builtin_ctz(bits);
    pixels[(top+row)*256+column]=ink;
    bits&=bits-1;
   }
  }
  pen+=ui_fallback_advances[g];
 }
}
static void nds_clock_paint_impl(u16* pixels,int x,int y,u16 ink){
 const NdsClockLayout layout=nds_clock_layout();
 nds_clock_draw(pixels,&layout,x+(48-layout.advance)/2,y+1,ink);
}
static void nds_clock_paint_menu(int x,int y){
 const NdsClockLayout layout=nds_clock_layout();
 if(x<0||y<0||x+layout.right-layout.left>256||y+layout.bottom-layout.top>192)return;
 nds_clock_draw(framebuffer,&layout,x-layout.left,y-layout.top,RGB15(31,31,31));
 clock_state[5]++;
}
static void nds_rmenu_draw_clock(const cpymo_engine* e,const void* data){
 nds_clock_collect_menu=true;nds_clock_menu_found=false;
 nds_rmenu_original_drawer(e,data);nds_clock_collect_menu=false;nds_clock_sample();
 nds_menu_clock_visible=true;clock_state[9]=1;
 if(nds_clock_menu_found)
  nds_clock_paint_menu(nds_clock_menu_x+6,nds_clock_menu_y+6);
}
extern error_t __real_cpymo_ui_enter(void**,cpymo_engine*,size_t,cpymo_ui_updater,cpymo_ui_drawer,cpymo_ui_deleter);
error_t __wrap_cpymo_ui_enter(void** out,cpymo_engine* e,size_t size,cpymo_ui_updater update,cpymo_ui_drawer draw,cpymo_ui_deleter del){
 /* Wrap only the drawer registered while entering the right-click menu.
    Settings, saves and script menus keep their original draw callbacks. */
 if(nds_registering_rmenu){nds_rmenu_original_drawer=draw;draw=nds_rmenu_draw_clock;}
 return __real_cpymo_ui_enter(out,e,size,update,draw,del);
}
