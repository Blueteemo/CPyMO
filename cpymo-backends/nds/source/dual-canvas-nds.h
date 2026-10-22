/* CPyMO 1.2 enhanced lower-LCD canvas, source-identical host/ARM9 drawing.
 * This is BACKDROP ONLY: game-authored textbox, fonts, choices and menus
 * are drawn by CPyMO on top of it. No generated illustrations or stand-ins.
 * Standard C; no libnds headers, heap allocation or disk I/O.
 */
#ifndef CPYMO_NDS_DUAL_CANVAS_H
#define CPYMO_NDS_DUAL_CANVAS_H
#include <stdint.h>
#include <stddef.h>
#define NDS_DUAL_CANVAS_W 256
#define NDS_DUAL_CANVAS_H 192

static inline uint16_t nds_dual_canvas_color(unsigned r,unsigned g,unsigned b){
 return (uint16_t)(0x8000u | (r&31u) | ((g&31u)<<5) | ((b&31u)<<10));
}
/* A+B refinement: quiet, RGB555-native navy gradient instead of four
 * abrupt 48-line stripes. Dialogue only occupies the TOP portion of the
 * touch LCD in actual PyMO games, so the bottom stays intentionally clean:
 * no fabricated footer or horizontal bar beneath the original textbox.
 * Integer-only math; a scanline uses ONE color, avoiding spatial dithering
 * noise over game-owned text. Keep this backdrop platform-independent. */
static inline uint16_t nds_dual_canvas_row(unsigned y){
 if(y>=NDS_DUAL_CANVAS_H)y=NDS_DUAL_CANVAS_H-1;
 unsigned bottom=(NDS_DUAL_CANVAS_H-1)-y;
 const unsigned half=(NDS_DUAL_CANVAS_H-1)/2;
 return nds_dual_canvas_color(
    (5u*bottom+2u*y+half)/(NDS_DUAL_CANVAS_H-1),
    (9u*bottom+4u*y+half)/(NDS_DUAL_CANVAS_H-1),
    (13u*bottom+7u*y+half)/(NDS_DUAL_CANVAS_H-1));
}
static inline void nds_dual_canvas_draw_yield(uint16_t *pixels,
                                        int interactive,int dialogue_visible,
                                        void (*audio_tick)(void*),void* audio_ctx){
 (void)interactive;
 (void)dialogue_visible;
 if(!pixels)return;
 for(unsigned y=0;y<NDS_DUAL_CANVAS_H;y++){
  const uint16_t col=nds_dual_canvas_row(y);
  for(unsigned x=0;x<NDS_DUAL_CANVAS_W;x++)
   pixels[y*NDS_DUAL_CANVAS_W+x]=col;
  /* Preserve the six per-frame audio servicing opportunities on ARM9. */
  if(audio_tick && (y&31u)==0)audio_tick(audio_ctx);
 }
 /* Minimal upper guide rail, deliberately outside the dialogue content. */
 const uint16_t bright=nds_dual_canvas_color(12,18,23);
 const uint16_t muted=nds_dual_canvas_color(7,12,17);
 for(unsigned x=12;x<244;x++)pixels[7*NDS_DUAL_CANVAS_W+x]=muted;
 for(unsigned x=12;x<38;x++)pixels[7*NDS_DUAL_CANVAS_W+x]=bright;
}
static inline void nds_dual_canvas_draw(uint16_t *pixels,
                                        int interactive,int dialogue_visible){
 nds_dual_canvas_draw_yield(pixels,interactive,dialogue_visible,NULL,NULL);
}
#endif
