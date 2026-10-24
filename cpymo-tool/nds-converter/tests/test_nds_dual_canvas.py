"""Execute the exact NDS enhanced RGB555 canvas C source on host.
Checks pixel rows, dialogue/footer palette, all three states and six audio
yield calls per pass. Does not assert invented game artwork or text.
"""
from pathlib import Path
import shutil,subprocess,tempfile,unittest

ROOT=Path(__file__).resolve().parents[3]
SOURCE=ROOT/"cpymo-backends/nds/source"
PROGRAM=r"""
#include "dual-canvas-nds.h"
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x))abort();}while(0)
static uint16_t source_a[256*192],source_b[256*192];
static int ticks;
static void audio_tick(void *unused){CHECK(unused==NULL);ticks++;}
int main(void){
 const uint16_t bg=nds_dual_canvas_color(5,9,13);
 const uint16_t dark=nds_dual_canvas_color(2,4,7);
 const uint16_t bright=nds_dual_canvas_color(12,18,23);
 for(int interactive=0;interactive<=1;interactive++){
  for(int dialogue=0;dialogue<=1;dialogue++){
   ticks=0;
   nds_dual_canvas_draw(source_a,interactive,dialogue);
   nds_dual_canvas_draw_yield(source_b,interactive,dialogue,
                              audio_tick,NULL);
   CHECK(ticks==6);  /* scanlines y=0,32,64,96,128,160 */
   CHECK(!memcmp(source_a,source_b,sizeof source_a));
   CHECK(source_a[1*256+0]==nds_dual_canvas_row(1));
   CHECK(source_a[0]==bg);
   CHECK(source_a[191*256+0]==dark);
   CHECK(source_a[7*256+15]==bright);
   CHECK(source_a[155*256+2]==nds_dual_canvas_row(155));
   /* The bottom half is no longer an oversized fabricated textbox footer.
      Interactive/hidden dialogue paths share the same quiet background. */
   CHECK(source_a[162*256+20]==nds_dual_canvas_row(162));
   CHECK(source_a[170*256+2]==nds_dual_canvas_row(170));
   CHECK(source_a[191*256+200]==dark);
  }
 }
 return 0;
}
"""
@unittest.skipUnless(shutil.which("gcc"),"GCC unavailable")
class NDSDualCanvasTests(unittest.TestCase):
 def test_backdrop_and_cooperative_audio_cadence(self):
  with tempfile.TemporaryDirectory() as folder:
   d=Path(folder);code=d/"test.c";code.write_text(PROGRAM,encoding="utf-8")
   exe=d/"test"
   subprocess.run(["gcc","-std=c11","-O2","-Wall","-Wextra","-Werror",
                   "-fsanitize=undefined","-I",str(SOURCE),
                   str(code),"-o",str(exe)],
                  capture_output=True,text=True,check=True)
   subprocess.run([str(exe)],capture_output=True,text=True,check=True)
