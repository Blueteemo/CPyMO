#include "release-version.h"
#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/iosupport.h>
#include "cpymo_engine.h"
#include "cpymo_save.h"
#include "cpymo_backend_masktrans.h"
#include "font.h"
#include "cache-nds.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
/* Keep audio supplied while the original JPEG/PNG decoder reads the card.
   This wrapper leaves upstream CPyMO and stb sources unchanged. */
extern stbi_uc* __real_stbi_load_from_callbacks(const stbi_io_callbacks*,void*,int*,int*,int*,int);
extern void nds_audio_update(void);
extern void nds_audio_pump_if_due(void);
typedef struct {const stbi_io_callbacks* cb;void* user;u64 last;} PumpIO;
static int pump_read(void* user,char* data,int size){PumpIO* io=user;int n=io->cb->read(io->user,data,size);u64 now=tickGetCount();if(now-io->last>TICK_FREQ/100){nds_audio_update();io->last=tickGetCount();}return n;}
static void pump_skip(void* user,int n){PumpIO* io=user;io->cb->skip(io->user,n);}
static int pump_eof(void* user){PumpIO* io=user;return io->cb->eof(io->user);}
stbi_uc* __wrap_stbi_load_from_callbacks(const stbi_io_callbacks* cb,void* user,int* w,int* h,int* channels,int requested){PumpIO io={cb,user,tickGetCount()};stbi_io_callbacks pump={pump_read,pump_skip,pump_eof};return __real_stbi_load_from_callbacks(&pump,&io,w,h,channels,requested);}

static cpymo_engine engine;
static u16 framebuffer[256*192] __attribute__((aligned(32)));
static u16 *vram;
static bool screens_swapped;
#include "preferences-nds.h"
/* magic, swapped, toast active/shown/expired/age ms, icon loaded, metadata lines, touch samples */
volatile unsigned presentation_state[9]={0x4e505232};
volatile unsigned engine_state[46]={0x4350454e};
/* magic, auto/hide/X/skip flags, current script line; bounded test telemetry */
volatile unsigned control_state[3]={0x4e43544c};
bool nds_cache_enabled;
static bool loaded_image;
extern void nds_audio_update(void);
volatile unsigned vblank_count;
static void on_vblank(void){++vblank_count;}
static float display_scale(void){
 int w=engine.gameconfig.imagesize_w,h=engine.gameconfig.imagesize_h;
 if(w<=0||h<=0)return 1;
 float sx=256.f/w,sy=192.f/h;return sx<sy?sx:sy;
}
static unsigned text_native_pixels(void);
int nds_font_choices_available(void);
static void enforce_text_readability(void){if(engine.gameconfig.imagesize_h>0){int min_font=(int)(text_native_pixels()*240.f/(engine.gameconfig.imagesize_h*.8f*display_scale())+.999f);if(nds_font_choices_available()||engine.gameconfig.fontsize<min_font)engine.gameconfig.fontsize=min_font;}}
static float display_x(void){return (256-engine.gameconfig.imagesize_w*display_scale())*.5f;}
static float display_y(void){return (192-engine.gameconfig.imagesize_h*display_scale())*.5f;}
typedef struct {int w,h,pw,ph;u16 *px;unsigned char *mask;unsigned refs;} Image;
static Image* menu_images[2];
static char menu_image_dir[256];
void nds_cache_path(char* out,size_t size,const char* dir,const char* kind,cpymo_str name,const char* ext){
 u64 hash=14695981039346656037ULL;for(size_t i=0;i<name.len;i++){unsigned char c=name.begin[i];if(c>='A'&&c<='Z')c+='a'-'A';hash^=c;hash*=1099511628211ULL;}
 snprintf(out,size,"%s/nds-cache/%s/%016llx%s",dir,kind,(unsigned long long)hash,ext);
}
static error_t cached_image(cpymo_backend_image* out,int* w,int* h,cpymo_str name,const cpymo_assetloader* loader,const char* kind){
 char path[512];nds_cache_path(path,sizeof path,loader->gamedir,kind,name,".npi");FILE* f=fopen(path,"rb");if(!f)return CPYMO_ERR_CAN_NOT_OPEN_FILE;
 struct {char magic[4];u16 w,h,pw,ph;u32 flags;} header;
 if(fread(&header,1,sizeof header,f)!=sizeof header||memcmp(header.magic,"NPI1",4)||!header.w||!header.h||!header.pw||!header.ph||header.pw>2048||header.ph>2048||(unsigned)header.pw*header.ph>262144||header.flags>1){fclose(f);return CPYMO_ERR_BAD_FILE_FORMAT;}
 unsigned count=(unsigned)header.pw*header.ph;
 if(fseek(f,0,SEEK_END)||ftell(f)!=(long)(sizeof header+count*(header.flags?3:2))||fseek(f,sizeof header,SEEK_SET)){fclose(f);return CPYMO_ERR_BAD_FILE_FORMAT;}
 Image* image=calloc(1,sizeof *image);if(!image){fclose(f);return CPYMO_ERR_OUT_OF_MEM;}
 image->refs=1;image->w=header.w;image->h=header.h;image->pw=header.pw;image->ph=header.ph;image->px=malloc(count*2);if(header.flags)image->mask=malloc(count);
 if(!image->px||(header.flags&&!image->mask)){fclose(f);free(image->px);free(image->mask);free(image);return CPYMO_ERR_OUT_OF_MEM;}
 unsigned bytes=count*2,done=0;bool ok=true;
 while(done<bytes){unsigned chunk=bytes-done;if(chunk>8192)chunk=8192;if(fread((unsigned char*)image->px+done,1,chunk,f)!=chunk){ok=false;break;}done+=chunk;nds_audio_update();}
 if(ok&&image->mask){done=0;while(done<count){unsigned chunk=count-done;if(chunk>8192)chunk=8192;if(fread(image->mask+done,1,chunk,f)!=chunk){ok=false;break;}done+=chunk;nds_audio_update();}}
 fclose(f);if(!ok){free(image->px);free(image->mask);free(image);return CPYMO_ERR_BAD_FILE_FORMAT;}
 *out=image;*w=image->w;*h=image->h;loaded_image=true;engine_state[40]++;return CPYMO_ERR_SUCC;
}
extern error_t __real_cpymo_assetloader_load_bg_image(cpymo_backend_image*,int*,int*,cpymo_str,const cpymo_assetloader*);
extern error_t __real_cpymo_assetloader_load_chara_image(cpymo_backend_image*,int*,int*,cpymo_str,const cpymo_assetloader*);
extern error_t __real_cpymo_assetloader_load_system_image(cpymo_backend_image*,int*,int*,cpymo_str,const cpymo_assetloader*,bool);
error_t __wrap_cpymo_assetloader_load_bg_image(cpymo_backend_image* out,int* w,int* h,cpymo_str name,const cpymo_assetloader* l){if(nds_cache_enabled)return cached_image(out,w,h,name,l,"bg");return __real_cpymo_assetloader_load_bg_image(out,w,h,name,l);}
error_t __wrap_cpymo_assetloader_load_chara_image(cpymo_backend_image* out,int* w,int* h,cpymo_str name,const cpymo_assetloader* l){if(nds_cache_enabled)return cached_image(out,w,h,name,l,"chara");return __real_cpymo_assetloader_load_chara_image(out,w,h,name,l);}
static void clear_menu_images(void){for(unsigned i=0;i<2;i++){if(menu_images[i])cpymo_backend_image_free(menu_images[i]);menu_images[i]=NULL;}menu_image_dir[0]=0;}
error_t __wrap_cpymo_assetloader_load_system_image(cpymo_backend_image* out,int* w,int* h,cpymo_str name,const cpymo_assetloader* l,bool mask){
 if(!nds_cache_enabled)return __real_cpymo_assetloader_load_system_image(out,w,h,name,l,mask);
 int slot=mask&&name.len==4&&!memcmp(name.begin,"menu",4)?0:mask&&name.len==13&&!memcmp(name.begin,"sel_highlight",13)?1:-1;
 if(slot<0)return cached_image(out,w,h,name,l,mask?"system1":"system0");
 if(strcmp(menu_image_dir,l->gamedir)){clear_menu_images();snprintf(menu_image_dir,sizeof menu_image_dir,"%s",l->gamedir);}
 if(!menu_images[slot]){error_t err=cached_image(out,w,h,name,l,"system1");if(err)return err;menu_images[slot]=*out;}
 Image* image=menu_images[slot];image->refs++;*out=image;*w=image->w;*h=image->h;return CPYMO_ERR_SUCC;
}
static void warm_menu_images(void){for(unsigned i=0;i<2;i++){cpymo_backend_image image=NULL;int w,h;error_t err=__wrap_cpymo_assetloader_load_system_image(&image,&w,&h,cpymo_str_pure(i?"sel_highlight":"menu"),&engine.assetloader,true);if(!err)cpymo_backend_image_free(image);nds_audio_update();}}
static u16 color(cpymo_color c){return RGB15(c.r>>3,c.g>>3,c.b>>3)|BIT(15);}
static void pixel(int x,int y,cpymo_color c,float alpha){
 if(x<0||x>=256||y<0||y>=192||alpha<=0)return;
 unsigned idx=y*256+x;if(alpha>=.99f){framebuffer[idx]=color(c);return;}
 u16 old=framebuffer[idx];int a=(int)(alpha*32);if(a>32)a=32;
 int r=((old&31)*(32-a)+(c.r>>3)*a)>>5;
 int g=(((old>>5)&31)*(32-a)+(c.g>>3)*a)>>5;
 int b=(((old>>10)&31)*(32-a)+(c.b>>3)*a)>>5;
 framebuffer[idx]=RGB15(r,g,b)|BIT(15);
}
/* Integer area filter: load-time only, before RGB555 quantization.
   Alpha images accumulate premultiplied channels to avoid dark fringes. */
static void shrink_pixels(const unsigned char* src,int w,int h,unsigned char* dst,int nw,int nh,int channels){
 for(int y=0;y<nh;y++){
  if(!(y&7))nds_audio_update();
  if(channels==3&&w*4==nw*5&&h*4==nh*5){
   int sy=y*5/4,wy=4-(y&3);unsigned char* out=dst+y*nw*3;
   for(int x=0;x<nw;x++,out+=3){int sx=x*5/4,wx=4-(x&3);const unsigned char* p=src+(sy*w+sx)*3;
    for(int c=0;c<3;c++)out[c]=(p[c]*wx*wy+p[c+3]*(5-wx)*wy+p[c+w*3]*wx*(5-wy)+p[c+w*3+3]*(5-wx)*(5-wy)+12)/25;
   }
   continue;
  }
  for(int x=0;x<nw;x++){
  int x0=x*w*256/nw,x1=(x+1)*w*256/nw,y0=y*h*256/nh,y1=(y+1)*h*256/nh;
  unsigned sums[4]={0},area=(x1-x0)*(y1-y0);
  for(int sy=y0>>8;sy<(y1+255)>>8;sy++)for(int sx=x0>>8;sx<(x1+255)>>8;sx++){
   int lo=sx*256>x0?sx*256:x0,hi=(sx+1)*256<x1?(sx+1)*256:x1,wx=hi-lo;
   lo=sy*256>y0?sy*256:y0;hi=(sy+1)*256<y1?(sy+1)*256:y1;
   unsigned weight=wx*(hi-lo);const unsigned char* p=src+(sy*w+sx)*channels;
   for(int c=0;c<channels;c++){unsigned v=p[c];if(channels==4&&c<3)v=(v*p[3]+127)/255;sums[c]+=v*weight;}
  }
  unsigned char* out=dst+(y*nw+x)*channels;
  for(int c=0;c<channels;c++){if(channels==4&&c<3)out[c]=sums[3]?(unsigned)(((u64)sums[c]*255+sums[3]/2)/sums[3]):0;else out[c]=(sums[c]+area/2)/area;}
  }
 }
}
error_t cpymo_backend_image_load(cpymo_backend_image *out,void *px,int w,int h,enum cpymo_backend_image_format f){
 engine_state[43]++;
 loaded_image=true;
 Image *im=calloc(1,sizeof *im);if(!im)return CPYMO_ERR_OUT_OF_MEM;
 im->refs=1;im->w=w;im->h=h;
 float scale=display_scale();if(scale>1)scale=1;
 im->pw=(int)(w*scale+.5f);im->ph=(int)(h*scale+.5f);
 if(im->pw<1)im->pw=1;if(im->ph<1)im->ph=1;
 int channels=f==cpymo_backend_image_format_rgb?3:4;
 unsigned char *filtered=NULL;
 if(im->pw!=w||im->ph!=h){
  filtered=malloc(im->pw*im->ph*channels);
  if(!filtered){free(im);return CPYMO_ERR_OUT_OF_MEM;}
  shrink_pixels(px,w,h,filtered,im->pw,im->ph,channels);
 }
 im->px=malloc(im->pw*im->ph*2);
 if(channels==4)im->mask=malloc(im->pw*im->ph);
 if(!im->px||(channels==4&&!im->mask)){free(filtered);free(im->px);free(im->mask);free(im);return CPYMO_ERR_OUT_OF_MEM;}
 unsigned char* p=filtered?filtered:px;
 for(int i=0;i<im->pw*im->ph;i++,p+=channels){im->px[i]=RGB15(p[0]>>3,p[1]>>3,p[2]>>3)|BIT(15);if(im->mask)im->mask[i]=p[3];}
 free(filtered);
 free(px);*out=im;return CPYMO_ERR_SUCC;
}
error_t cpymo_backend_image_load_with_mask(cpymo_backend_image *out,void *px,void *mask,int w,int h,int mw,int mh){
 unsigned char* p=px;unsigned char* m=mask;
 for(int y=0;y<h;y++)for(int x=0;x<w;x++)p[(y*w+x)*4+3]=m[(y*mh/h)*mw+x*mw/w];
 error_t err=cpymo_backend_image_load(out,px,w,h,cpymo_backend_image_format_rgba);
 if(!err)free(mask);
 return err;
}
void cpymo_backend_image_free(cpymo_backend_image im){if(im){Image* image=im;if(--image->refs)return;free(image->px);free(image->mask);free(image);}}
/* Independent red/blue lanes are ten bits apart: sums stay <=31*32.
   Exact RGB555 blend, using two multiplies for both lanes instead of four. */
static u16 blend(u16 old,u16 col,int a){if(a>=32)return col;if(a<=0)return old;unsigned rb=(((old&0x7c1f)*(32-a)+(col&0x7c1f)*a)>>5)&0x7c1f;unsigned g=(((old&0x03e0)*(32-a)+(col&0x03e0)*a)>>5)&0x03e0;return rb|g|BIT(15);}
void cpymo_backend_image_draw(float dx,float dy,float dw,float dh,cpymo_backend_image image,int sx,int sy,int sw,int sh,float alpha,enum cpymo_backend_image_draw_type type){
 (void)type;Image* im=image;if(!im||dw<=0||dh<=0)return;
 float scale=display_scale();dx=dx*scale+display_x();dy=dy*scale+display_y();dw*=scale;dh*=scale;
 int a=(int)(alpha*32);if(a<=0)return;if(a>32)a=32;
 int left=(int)dx,top=(int)dy,right=(int)(dx+dw),bottom=(int)(dy+dh);
 int x0=left<0?0:left,y0=top<0?0:top,x1=right>256?256:right,y1=bottom>192?192:bottom;
 if(x1<=x0||y1<=y0)return;
 int startX=(int)(sx*(float)im->pw/im->w*65536.f),startY=(int)(sy*(float)im->ph/im->h*65536.f);
 unsigned char opacity[256];if(im->mask)for(unsigned i=0;i<256;i++)opacity[i]=(a*i+127)/255;
 int stepX=(int)(sw*(float)im->pw/im->w*65536.f/dw+.5f),stepY=(int)(sh*(float)im->ph/im->h*65536.f/dh+.5f);
 int columns[256];for(int x=x0;x<x1;x++){int ix=(startX+(x-left)*stepX)>>16;columns[x]=ix>=0&&ix<im->pw?ix:-1;}
 for(int y=y0;y<y1;y++){
  if(!(y&15))nds_audio_pump_if_due();
  int iy=(startY+(y-top)*stepY)>>16;if(iy<0||iy>=im->ph)continue;
  int sourceX=(startX>>16)+x0-left;
  if(!im->mask && a==32 && stepX==65536 && sourceX>=0 && sourceX+x1-x0<=im->pw){memcpy(framebuffer+y*256+x0,im->px+iy*im->pw+sourceX,(x1-x0)*2);continue;}
  int fixedX=startX+(x0-left)*stepX;
  for(int x=x0;x<x1;x++,fixedX+=stepX){int ix=columns[x];if(ix<0)continue;int idx=iy*im->pw+ix;int aa=im->mask?opacity[im->mask[idx]]:a;framebuffer[y*256+x]=blend(framebuffer[y*256+x],im->px[idx],aa);}
 }
}
void cpymo_backend_image_fill_rects(const float* r,size_t n,cpymo_color c,float alpha,enum cpymo_backend_image_draw_type type){
 (void)type;int a=(int)(alpha*32);u16 col=color(c);if(a<=0)return;
 float scale=display_scale(),ox=display_x(),oy=display_y();
 for(size_t i=0;i<n;i++,r+=4){int x0=(int)(r[0]*scale+ox),y0=(int)(r[1]*scale+oy),x1=(int)((r[0]+r[2])*scale+ox),y1=(int)((r[1]+r[3])*scale+oy);if(x0<0)x0=0;if(y0<0)y0=0;if(x1>256)x1=256;if(y1>192)y1=192;for(int y=y0;y<y1;y++){if(!(y&15))nds_audio_pump_if_due();for(int x=x0;x<x1;x++)framebuffer[y*256+x]=blend(framebuffer[y*256+x],col,a);}}
}
bool cpymo_backend_image_album_ui_writable(){return false;}
#include "masktrans-nds.h"
static unsigned decode(const char** p){unsigned c=(unsigned char)*(*p)++;if(c<128)return c;unsigned n=c<224?1:c<240?2:3;c&=(1u<<(6-n))-1;while(n--){c=(c<<6)|((unsigned char)*(*p)++&63);}return c;}
typedef struct {u32 code;u8 advance,pad[3];u16 rows[14];} GameGlyph;
static GameGlyph* game_glyphs;static unsigned game_glyph_count;
#define BUILTIN_GLYPHS (sizeof glyph_codes/sizeof *glyph_codes)
static int glyph(unsigned c){if(game_glyphs){int lo=0,hi=game_glyph_count-1;while(lo<=hi){int m=(lo+hi)/2;if(game_glyphs[m].code==c)return BUILTIN_GLYPHS+m;if(game_glyphs[m].code<c)lo=m+1;else hi=m-1;}}int lo=0,hi=BUILTIN_GLYPHS-1;while(lo<=hi){int m=(lo+hi)/2;if(glyph_codes[m]==c)return m;if(glyph_codes[m]<c)lo=m+1;else hi=m-1;}return -1;}
static unsigned glyph_advance(int g){return g<(int)BUILTIN_GLYPHS?glyph_advances[g]:game_glyphs[g-BUILTIN_GLYPHS].advance;}
static unsigned glyph_row(int g,int y){return g<(int)BUILTIN_GLYPHS?glyph_rows[g][y]:game_glyphs[g-BUILTIN_GLYPHS].rows[y];}
static bool load_game_font(const char* dir){char path[512];snprintf(path,sizeof path,"%s/nds-cache/font.bin",dir);FILE* f=fopen(path,"rb");if(!f)return false;u32 header[2];if(fread(header,1,8,f)!=8||memcmp(header,"NF13",4)||!header[1]||header[1]>20000){fclose(f);return false;}unsigned count=header[1];GameGlyph* data=malloc(count*sizeof *data);if(!data){fclose(f);return false;}bool ok=fread(data,sizeof *data,count,f)==count&&fgetc(f)==EOF;fclose(f);for(unsigned i=0;ok&&i<count;i++){if((i&&data[i-1].code>=data[i].code)||data[i].advance>32)ok=false;for(unsigned y=0;y<14;y++)if(data[i].rows[y]&~8191u)ok=false;}if(!ok){free(data);return false;}free(game_glyphs);game_glyphs=data;game_glyph_count=count;engine_state[41]=count;return true;}
static FILE* trace_open(const char* mode);
static void missing_glyph(unsigned cp){engine_state[20]++;FILE* file=trace_open("ab");if(file){fprintf(file,"MISSING_GLYPH U+%04X\n",cp);fclose(file);}}
#include "ui-fallback-font.h"
#include "pixel-font-nds.h"
typedef struct {float size;unsigned mode;char text[];} GlyphText;
float cpymo_backend_text_width(cpymo_str s,float size){unsigned mode=active_font_mode(),w=0,maxw=0;const char* p=s.begin;while(p<s.begin+s.len){unsigned c=decode(&p);if(c==13)continue;if(c==10){if(w>maxw)maxw=w;w=0;continue;}int g=text_glyph(mode,c);w+=g>=0?text_advance(mode,g):font_nominal(mode);}if(w>maxw)maxw=w;return maxw*(font_scale(size,mode)/display_scale());}
error_t cpymo_backend_text_create(cpymo_backend_text *out,float *width,cpymo_str s,float size){GlyphText* p=malloc(sizeof *p+s.len+1);if(!p)return CPYMO_ERR_OUT_OF_MEM;p->size=size;p->mode=active_font_mode();memcpy(p->text,s.begin,s.len);p->text[s.len]=0;*out=p;*width=cpymo_backend_text_width(s,size);return CPYMO_ERR_SUCC;}
void cpymo_backend_text_free(cpymo_backend_text p){free(p);}
void cpymo_backend_text_draw(cpymo_backend_text text,float x,float baseline,cpymo_color c,float alpha,enum cpymo_backend_image_draw_type type){
 (void)type;GlyphText* t=text;unsigned mode=t->mode;const char* p=t->text;float screen_scale=display_scale();x=x*screen_scale+display_x();baseline=baseline*screen_scale+display_y();float scale=font_scale(t->size,mode);unsigned gh=font_height(mode),gw=font_width(mode),gb=font_baseline(mode);
 int h=(int)(gh*scale+.5f),w=(int)(gw*scale+.5f);
 if(scale==1.f){int pen=(int)x,top=(int)(baseline-gb);int a=(int)(alpha*32);u16 col=color(c);
  while(*p){unsigned cp=decode(&p);if(cp==13)continue;if(cp==10){pen=(int)x;top+=font_nominal(mode)+3;continue;}int g=text_glyph(mode,cp);if(g<0){missing_glyph(cp);g=text_glyph(mode,'?');}if(g>=0){for(unsigned y=0;y<gh;y++){int yy=top+y;if(yy<0||yy>=192)continue;unsigned bits=text_row(mode,g,y);for(unsigned xx=0;xx<gw;xx++)if(bits&(1<<xx)){int px=pen+xx;if(px>=0&&px<256){unsigned idx=yy*256+px;framebuffer[idx]=blend(framebuffer[idx],col,a);}}}pen+=text_advance(mode,g);}else pen+=font_nominal(mode);}
  return;
 }
 float start_x=x;while(*p){unsigned cp=decode(&p);if(cp==13)continue;if(cp==10){x=start_x;baseline+=(font_nominal(mode)+3)*scale;continue;}int g=text_glyph(mode,cp);if(g<0){missing_glyph(cp);g=text_glyph(mode,'?');}
 if(g>=0)for(int y=0;y<h;y++)for(int xx=0;xx<w;xx++){unsigned gy=(unsigned)(y/scale),gx=(unsigned)(xx/scale);if(gy<gh&&gx<gw&&(text_row(mode,g,gy)&(1<<gx)))pixel((int)x+xx,(int)(baseline-gb*scale)+y,c,alpha);}x+=(g>=0?text_advance(mode,g):font_nominal(mode))*scale;}
}
cpymo_input cpymo_input_snapshot(){
 int k=keysHeld();touchPosition t={0};if(k&KEY_TOUCH){touchRead(&t);presentation_state[8]++;}
 bool ui=cpymo_ui_enabled(&engine);
 // Script menus use the core cancel rule; save-enabled story choices keep X for the menu.
 bool script_menu=engine.select_img.selections&&!engine.select_img.save_enabled;
 cpymo_input in={0};in.ok=k&KEY_A;in.cancel=k&((ui||script_menu)?KEY_B:KEY_X);
 in.up=k&KEY_UP;in.down=k&KEY_DOWN;in.left=k&KEY_LEFT;in.right=k&KEY_RIGHT;in.skip=k&KEY_R;
 in.hide_window=!ui&&!engine.select_img.selections&&(k&KEY_B);
 in.mouse_button=!nds_touch_blocked&&(k&KEY_TOUCH);in.mouse_position_useable=in.mouse_button;
 in.mouse_x=(t.px-display_x())/display_scale();in.mouse_y=(t.py-display_y())/display_scale();return in;
}
static FILE* savefile(const char* dir,const char* name,const char* mode){char path[512];snprintf(path,sizeof path,"%s/save/%s",dir,name);return fopen(path,mode);}
FILE* cpymo_backend_read_save(const char* dir,const char* name){return savefile(dir,name,"rb");}
FILE* cpymo_backend_write_save(const char* dir,const char* name){return savefile(dir,name,"wb");}
/* Movie wrapper is implemented after the power and framebuffer helpers. */
/* Release play does not synchronously open/write a card log for each line.
   Opt in with save/nds-debug.flag; telemetry remains available in RAM. */
volatile unsigned diagnostic_state[4]={0x4e444247};
static bool debug_trace;
/* Upstream stdout must not write tile-map bytes into our bitmap auxiliary
   screen. Diagnostic mode retains the original libnds console device. */
static const devoptab_t* console_output;
static devoptab_t filtered_output;
static ssize_t filtered_console_write(struct _reent* r,void* fd,const char* data,size_t size){
 if(debug_trace&&console_output&&console_output->write_r)return console_output->write_r(r,fd,data,size);
 return size;
}
static void filter_console_output(void){
 if(devoptab_list[STD_OUT]==&filtered_output)return;
 fflush(stdout);console_output=devoptab_list[STD_OUT];filtered_output=*console_output;
 filtered_output.write_r=filtered_console_write;devoptab_list[STD_OUT]=&filtered_output;
}
static FILE* trace_open(const char* mode){diagnostic_state[2]++;if(!debug_trace||!engine.assetloader.gamedir)return NULL;FILE* f=savefile(engine.assetloader.gamedir,"nds-trace.txt",mode);if(f)diagnostic_state[3]++;return f;}
void cpymo_backend_text_extract(const char* text){FILE* trace=trace_open("ab");if(trace){fprintf(trace,"TEXT %s\n",text);fclose(trace);}}
/* Keep a separate profiler so the existing 46-word acceptance ABI is stable. */
volatile unsigned menu_performance[5]={0x4d454e55};
extern error_t __real_cpymo_rmenu_enter(cpymo_engine*);
error_t __wrap_cpymo_rmenu_enter(cpymo_engine* e){
 nds_audio_update();u64 begin=tickGetCount();
 error_t result=__real_cpymo_rmenu_enter(e);
 unsigned us=(tickGetCount()-begin)*1000000/TICK_FREQ;
 menu_performance[1]++;menu_performance[2]=us;
 if(us>menu_performance[3])menu_performance[3]=us;
 nds_audio_update();FILE* trace=trace_open("ab");
 if(trace){fprintf(trace,"MENU_OPEN us=%u status=%d\n",us,result);fclose(trace);}
 return result;
}
/* magic, asleep, sleep/wake counts, last sleep ms, audio close/open,
   preserved tail samples, live source channels, underrun tails, wake dt us, polls */
volatile unsigned sleep_state[12]={0x4e534c31};
extern void nds_audio_suspend(void);
extern void nds_audio_resume(void);
static void nds_power_poll(void){sleep_state[11]++;unsigned wakes=sleep_state[3];if(!pmMainLoop())exit(0);if(wakes!=sleep_state[3])nds_aux_refresh();}
#include "launcher.h"
#include "aux-screen-nds.h"
static PmEventCookie sleep_cookie;
static u64 sleep_started;
static void nds_power_event(void* user,PmEvent event){
 (void)user;
 if(event==PmEvent_OnSleep){sleep_started=tickGetCount();nds_audio_suspend();sleep_state[1]=1;sleep_state[2]++;}
 else if(event==PmEvent_OnWakeup){u64 delta=tickGetCount()-sleep_started;sleep_state[4]=delta*1000/TICK_FREQ;if(presentation_state[2])toast_started+=delta;nds_audio_resume();sleep_state[1]=0;sleep_state[3]++;}
}

/* Standard NDS release intentionally skips video, including optional caches. */
error_t __wrap_cpymo_movie_play(cpymo_engine* e,const char* name){(void)e;(void)name;engine_state[45]++;return CPYMO_ERR_SUCC;}
extern void __real_cpymo_charas_wait(cpymo_engine*);
extern void cpymo_charas_gc(cpymo_charas*,bool);
void __wrap_cpymo_charas_wait(cpymo_engine* e){
 if(nds_chara_effects_get()){__real_cpymo_charas_wait(e);return;}
 /* Finish one complete character command before drawing its final state.
    Image masks remain active; only timed character interpolation is skipped. */
 for(struct cpymo_chara* c=e->charas.chara;c;c=c->next){cpymo_tween_finish(&c->pos_x);cpymo_tween_finish(&c->pos_y);cpymo_tween_finish(&c->alpha);}
 cpymo_charas_gc(&e->charas,false);chara_effect_state[2]++;cpymo_engine_request_redraw(e);
}

int main(int argc,char** argv){
 pmAddEventHandler(&sleep_cookie,nds_power_event,NULL);
 debug_trace=false;filter_console_output();launcher_video_init();launcher_show_splash();
 if(!fatInitDefault()){launcher_failure_hint("FAT failed",CPYMO_ERR_CAN_NOT_OPEN_FILE,"Check TF card, then restart.");launcher_video_show();while(1){nds_power_poll();swiWaitForVBlank();}}
 nds_global_preferences_load();
 const char* hint=argc>1?argv[1]:NULL;
 for(;;){
 debug_trace=false;filter_console_output();
 char game_dir[256];launcher_choose(game_dir,sizeof game_dir,hint);
 FILE* debug_flag=savefile(game_dir,"nds-debug.flag","rb");debug_trace=debug_flag!=NULL;if(debug_flag)fclose(debug_flag);diagnostic_state[1]=debug_trace;diagnostic_state[2]=diagnostic_state[3]=0;
 memset(&engine,0,sizeof engine);memset((void*)(engine_state+1),0,sizeof engine_state-sizeof engine_state[0]);
 if(debug_trace)consoleDemoInit();
 nds_cache_enabled=true;
 error_t err=(load_game_font(game_dir)&&load_pixel_fonts(game_dir))?CPYMO_ERR_SUCC:CPYMO_ERR_BAD_FILE_FORMAT;
 if(!err)err=cpymo_engine_init(&engine,game_dir);engine_state[1]=err;
 // DS readability policy is in memory only. Keep layout line spacing in
 // agreement with the selected native pixel size; never rewrite gameconfig.txt.
 if(!err){nds_preferences_load(game_dir);enforce_text_readability();if(nds_cache_enabled)warm_menu_images();}
 if(debug_trace)printf("Core init: %d\n",err);FILE* trace=trace_open("wb");if(trace){fprintf(trace,"INIT %d\n",err);fclose(trace);}
 if(err){launcher_state[6]=err;cpymo_audio_free(&engine.audio);if(debug_trace)printf("Game init failed: %d\nPress B to return to games.\n",err);else launcher_failure("启动失败，请检查转换结果",err);do{nds_power_poll();swiWaitForVBlank();scanKeys();}while(!(keysDown()&KEY_B));continue;}
 nds_playing=true;nds_aux_refresh();
 if(debug_trace){consoleClear();printf("CPyMO Nintendo DS\n\nA / Touch: Next\nX: Menu    B: Hold to hide text\nR: Skip read text\nSTART: Save slot 1\nSELECT: Load slot 1\n\nDS mode: 4 MiB\n");}else launcher_play_panel(NULL);
 // Count actual display interrupts; include missed frames during decoding/drawing.
 irqSet(IRQ_VBLANK,on_vblank);irqEnable(IRQ_VBLANK);
 u64 prev_tick=tickGetCount(),observed_tick=prev_tick;float elapsed=0;char last_script[64]={0};
 while(1){unsigned wakes=sleep_state[3];nds_power_poll();bool woke=sleep_state[3]!=wakes;if(woke)prev_tick=observed_tick=tickGetCount();swiWaitForVBlank();scanKeys();
 nds_audio_update();
 if(!debug_trace)launcher_update_toast();
 if(keysDown()&KEY_L){lcdSwap();screens_swapped=!screens_swapped;presentation_state[1]=screens_swapped;nds_aux_refresh();}
 nds_aux_poll();
 if(keysDown()&KEY_START){error_t save_err=cpymo_save_write(&engine,1);engine_state[10]=(unsigned)save_err;engine_state[12]++;if(!debug_trace)launcher_toast(save_err?"快速保存到存档1失败":"已快速保存到存档1");trace=trace_open("ab");if(trace){fprintf(trace,"SAVE slot=1 status=%d\n",save_err);fclose(trace);}}
 if(keysDown()&KEY_SELECT){FILE* saved=cpymo_save_open_read(&engine,1);error_t load_err=CPYMO_ERR_CAN_NOT_OPEN_FILE;if(saved){load_err=cpymo_save_load_savedata(&engine,saved);fclose(saved);}engine_state[11]=(unsigned)load_err;engine_state[13]++;if(!debug_trace)launcher_toast(load_err?"读取存档1失败或存档不存在":"已快速读取存档1");trace=trace_open("ab");if(trace){fprintf(trace,"LOAD slot=1 status=%d\n",load_err);fclose(trace);}if(saved&&load_err){engine_state[1]=(unsigned)load_err;break;}}
 unsigned now=vblank_count;u64 now_tick=tickGetCount();float dt=(float)(now_tick-prev_tick)/TICK_FREQ;prev_tick=now_tick;if(woke)sleep_state[10]=(unsigned)(dt*1000000);elapsed+=dt;bool redraw=false;
 bool saved_skip_policy=engine.config_skip_already_read_only;
 bool skip_override=false; /* L is now exclusively screen swap; skip policy comes from settings. */
 if(skip_override)engine.config_skip_already_read_only=false;
 unsigned interval_us=(now_tick-observed_tick)*1000000/TICK_FREQ;observed_tick=now_tick;
 engine_state[33+(interval_us<=20000?0:interval_us<=34000?1:interval_us<=50000?2:interval_us<=100000?3:4)]++;
 u64 update_start=tickGetCount();loaded_image=false;
 err=cpymo_engine_update(&engine,dt,&redraw);if(!err)enforce_text_readability();if(skip_override)engine.config_skip_already_read_only=saved_skip_policy;engine_state[1]=err;engine_state[2]++;
 nds_preferences_sync();
 control_state[1]=(engine.say.auto_mode?1:0)|(engine.say.hide_window?2:0)|(engine.input.hide_window?4:0)|((engine.skipping||engine.input.skip)?8:0)|(engine.config_skip_already_read_only?16:0);control_state[2]=engine.interpreter?engine.interpreter->checkpoint.cur_line:0;
 if(loaded_image){prev_tick=tickGetCount();engine_state[32]++;}
 unsigned update_us=(tickGetCount()-update_start)*1000000/TICK_FREQ;if(update_us>engine_state[31])engine_state[31]=update_us;
 engine_state[5]=(unsigned)(elapsed*1000);engine_state[6]=(unsigned)now;
 if(err==CPYMO_ERR_NO_MORE_CONTENT){trace=trace_open("ab");if(trace){fprintf(trace,"SESSION_END\n");fclose(trace);}engine_state[1]=0;break;}
 if(err){if(!debug_trace){launcher_failure("运行失败，请检查游戏资源",err);do{nds_power_poll();swiWaitForVBlank();scanKeys();}while(!(keysDown()&KEY_B));}else printf("Update error: %d\n",err);trace=trace_open("ab");if(trace){fprintf(trace,"ERROR %d\n",err);fclose(trace);}break;}
 if(engine.interpreter && engine.interpreter->script){
  engine_state[14]=(unsigned)engine.interpreter->checkpoint.cur_line;
  const char* name=engine.interpreter->script->script_name;
  if(strcmp(last_script,name)){snprintf(last_script,sizeof last_script,"%s",name);trace=trace_open("ab");if(trace){fprintf(trace,"SCRIPT %s\n",name);fclose(trace);}}
  /* Generic startup-menu marker; script names are recorded in the trace. */
  engine_state[7]=!strcmp(name,"MENU")?10:0;
 }
 if(redraw){u64 draw_start=tickGetCount();for(unsigned i=0;i<256*192;i++)framebuffer[i]=BIT(15);cpymo_engine_draw(&engine);
  // DMA reads main memory, not the ARM9 data cache. Publish the whole frame.
  DC_FlushRange(framebuffer,sizeof framebuffer);
  dmaCopy(framebuffer,vram,sizeof framebuffer);engine_state[29]++;if(engine.bg.transform_draw){engine_state[38]++;float p=cpymo_tween_value(&engine.bg.transform_progression);if(p>0&&p<1)engine_state[39]|=1u<<(unsigned)(p*31);}
  unsigned draw_us=(tickGetCount()-draw_start)*1000000/TICK_FREQ;if(draw_us>engine_state[30])engine_state[30]=draw_us;}
 nds_audio_update();
 struct mallinfo mi=mallinfo();engine_state[3]=mi.uordblks;if(engine_state[3]>engine_state[4])engine_state[4]=engine_state[3];
 if(false && engine_state[2]%60==0)printf("\x1b[6;0HFrame: %6u    \nHeap: %7u      \nPeak: %7u      ",engine_state[2],engine_state[3],engine_state[4]);
 if(engine_state[2]%120==0){trace=trace_open("ab");if(trace){fprintf(trace,"STATE frame=%u ms=%u vblank=%u bg=%s\n",engine_state[2],engine_state[5],engine_state[6],engine.bg.current_bg_name?engine.bg.current_bg_name:"none");fclose(trace);}}
 }
 nds_playing=false;nds_aux_refresh();cpymo_engine_free(&engine);
 }
}
