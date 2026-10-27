"""Pixel geometry and action-hint regression for NDS 256x192 screens.

The final library footer used to start at y=178 with up to 16-pixel UI
glyphs; its descenders could extend below the 192-pixel LCD.
"""
from pathlib import Path
import re
import unittest

ROOT=Path(__file__).resolve().parents[3]
BACKEND=ROOT/"cpymo-backends/nds/source"
LAUNCHER=(BACKEND/"launcher.h").read_text(encoding="utf8")
MAIN=(BACKEND/"main.c").read_text(encoding="utf8")
L10N=(BACKEND/"ui-language-nds.c").read_text(encoding="utf8")
EXTRA=(BACKEND/"ui-language-extra.h").read_text(encoding="utf8")

class LauncherHelpLayoutTests(unittest.TestCase):
 def test_two_library_footer_rows_have_bottom_padding_and_no_card_overlap(self):
  self.assertIn("int y=33+row*31",LAUNCHER)
  self.assertIn("launcher_rect(6,y,244,26",LAUNCHER)
  self.assertIn("t.py>=33&&t.py<154",LAUNCHER)
  for y in (155,173):
   self.assertRegex(LAUNCHER,rf"launcher_text\\([^;]+,7,{y},")
  self.assertGreaterEqual(155-(33+3*31+26),3)
  self.assertGreaterEqual(192-(173+16),3)
  self.assertGreaterEqual(173-(155+16),2)
 def test_all_enhanced_locales_explain_down_without_new_arrow_glyph(self):
  self.assertIn('nds_ui("A / DOWN / 轻触：继续")',MAIN)
  self.assertIn('{"A / DOWN / 轻触：继续","A / DOWN / Touch: Next"}',L10N)
  self.assertIn('"A / DOWN / 輕觸：繼續"',EXTRA)
  self.assertIn('"A / DOWN / タッチ: 進む"',EXTRA)
  self.assertNotIn('nds_ui("A / 轻触：继续")',MAIN)

if __name__=="__main__":
 unittest.main()
