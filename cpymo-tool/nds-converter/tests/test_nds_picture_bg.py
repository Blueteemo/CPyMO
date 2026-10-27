"""Exercise the real optional lower-LCD artwork reader on a host filesystem.
A missing/corrupt asset must not harm gameplay and valid images stay cached.
"""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[3]
SRC=ROOT/"cpymo-backends/nds/source"
C=r"""
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nds-picture-bg.h"
#define CHECK(x) do{if(!(x))abort();}while(0)
int main(int argc,char** argv){
 if(argc!=2)return 2;
 /* Source file must be placed in a test-mapped /pymo path by a host macro. */
 const uint16_t* px=nds_picture_get();
 CHECK(px!=NULL);
 CHECK(px[0]==0x8001u);
 CHECK(px[256u*192u-1u]==0xFFFFu);
 CHECK(nds_picture_get()==px);
 CHECK(nds_picture_checked);
 return 0;
}
"""
# The standard source uses /pymo in production. For tests we replace only
# its root-file-path literal in a temporary copy of the actual header.
@unittest.skipUnless(shutil.which("gcc"),"GCC unavailable")
class NativePictureTests(unittest.TestCase):
 def test_exact_image_header_cache_and_reject_bad_input(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp)
   header=(SRC/"nds-picture-bg.h").read_text(encoding="utf8")
   image_path=root/"pic.rgb555"
   header=header.replace('"/pymo/nds-dual-bg.rgb555"',
       '"'+str(image_path).replace('\\','/')+'"')
   (root/"nds-picture-bg.h").write_text(header,encoding="utf8")
   c=root/"picture.c";c.write_text(C,encoding="utf8")
   exe=root/"picture"
   subprocess.run(["gcc","-std=c11","-Wall","-Wextra","-Werror","-O2",
                   "-fsanitize=undefined","-I",str(root),str(c),"-o",str(exe)],
                  capture_output=True,text=True,check=True)
   image_path.write_bytes(b"CPYBG12\\0"+b"\\x01\\x80" +
       b"\\xff\\xff"*(256*192-2)+b"\\xff\\xff")
   subprocess.run([str(exe),"valid"],check=True,
                  capture_output=True,text=True)
   image_path.write_bytes(b"bad-data")
   proc=subprocess.run([str(exe),"bad"],capture_output=True,text=True)
   self.assertNotEqual(proc.returncode,0)
