/* NDS 1.2 physical touch routing, pure C for ROM + host regression.
 * A display mode is not itself proof of a visible interactive touch
 * layer: enhanced mode's hidden lower LCD cannot accept menu taps.
 */
#ifndef CPYMO_NDS_DUAL_INPUT_H
#define CPYMO_NDS_DUAL_INPUT_H
typedef struct {
 float x,y;
 unsigned mouse_button,ok;
} nds_touch_sample;

/* 0 classic upper: touch bottom auxiliary to advance.
 * 1 classic lower: native game touch coordinates.
 * 2 enhanced: native coordinate choice/UI hit-tests, narrative tap=A.
 * hidden enhanced lower: first touch is separately consumed as wake-up. */
static inline unsigned nds_touch_uses_coordinates(unsigned mode,
  unsigned enhanced_visible,unsigned ui,unsigned choice){
 return mode==1 || (mode==2 && enhanced_visible && (ui||choice));
}
static inline nds_touch_sample nds_touch_project(unsigned mode,
 unsigned enhanced_visible,unsigned ui,unsigned choice,
 unsigned physical_touch,unsigned blocked,unsigned key_a,
 int screen_x,int screen_y,float screen_scale,float letterbox_x,
 float letterbox_y){
 nds_touch_sample out={0,0,0,0};
 unsigned touch=physical_touch&&!blocked;
 unsigned coordinates=nds_touch_uses_coordinates(mode,enhanced_visible,ui,choice);
 out.ok=!!key_a || (touch&&!coordinates);
 out.mouse_button=touch&&coordinates;
 if(out.mouse_button && screen_scale>0.f){
  out.x=(screen_x-letterbox_x)/screen_scale;
  out.y=(screen_y-letterbox_y)/screen_scale;
 }
 return out;
}
#endif
