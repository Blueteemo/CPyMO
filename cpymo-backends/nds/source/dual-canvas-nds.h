/* CPyMO 1.2 enhanced lower-LCD canvas, source-identical host/ARM9 drawing.
 * This is BACKDROP ONLY: game-authored textbox, fonts, choices and menus
 * are drawn by CPyMO on top of it. No generated illustrations or stand-ins.
 * Standard C; no libnds headers, heap allocation or disk I/O.
 */
#ifndef CPYMO_NDS_DUAL_CANVAS_H
#define CPYMO_NDS_DUAL_CANVAS_H
#include <stdint.h>
#define NDS_DUAL_CANVAS_W 256
#define NDS_DUAL_CANVAS_H 192

static inline uint16_t nds_dual_canvas_color(unsigned r,unsigned g,unsigned b){
 return (uint16_t)(0x8000u | (r&31u) | ((g&31u)<<5) | ((b&31u)<<10));
}
/* Caller supplies at least 256*192 uint16_t. Draw precisely what the
 * enhanced LCD shows before CPyMO's actual textbox/menu UI and hint glyphs.
 * A subdued footer is present only when a narrative dialogue is visible. */
static inline void nds_dual_canvas_draw(uint16_t *pixels,
                                        int interactive,int dialogue_visible){
 if(!pixels)return;
 for(unsigned y=0;y<NDS_DUAL_CANVAS_H;y++){
  unsigned band=y/48;
  uint16_t col=band==0?nds_dual_canvas_color(5,9,13):
          band==1?nds_dual_canvas_color(4,7,11):
          band==2?nds_dual_canvas_color(3,6,9):
                   nds_dual_canvas_color(2,4,7);
  for(unsigned x=0;x<NDS_DUAL_CANVAS_W;x++)
   pixels[y*NDS_DUAL_CANVAS_W+x]=col;
 }
 uint16_t bright=nds_dual_canvas_color(12,18,23);
 uint16_t muted=nds_dual_canvas_color(7,12,17);
 for(unsigned x=12;x<244;x++)pixels[7*NDS_DUAL_CANVAS_W+x]=muted;
 for(unsigned x=12;x<38;x++)pixels[7*NDS_DUAL_CANVAS_W+x]=bright;
 if(!interactive&&dialogue_visible){
  for(unsigned y=162;y<NDS_DUAL_CANVAS_H;y++)
   for(unsigned x=0;x<NDS_DUAL_CANVAS_W;x++)
    pixels[y*NDS_DUAL_CANVAS_W+x]=nds_dual_canvas_color(3,6,9);
  for(unsigned x=12;x<244;x++)pixels[162*NDS_DUAL_CANVAS_W+x]=muted;
 }
}
#endif
