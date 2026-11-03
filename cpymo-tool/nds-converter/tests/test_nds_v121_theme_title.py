"""v1.2.1: host-level guards for themes, title help and rendering."""
from pathlib import Path
import re
import unittest

ROOT=Path(__file__).resolve().parents[3]
SRC=ROOT/"cpymo-backends/nds/source"
def load(name):return (SRC/name).read_text(encoding="utf8")

class NdsV121Tests(unittest.TestCase):
 def test_ipl_favorite_colors_exact(self):
  theme=load("theme-nds.h")
  match=re.search(r'nds_ipl_theme_rgb\[16\]\[3\]=\{(.*?)\};',theme,re.S)
  self.assertIsNotNone(match)
  rgb=[tuple(map(int,x)) for x in re.findall(r'\{(\d+),(\d+),(\d+)\}',match.group(1))]
  self.assertEqual(rgb,[
   (12,16,19),(23,9,0),(31,0,3),(31,17,31),
   (31,18,0),(30,28,0),(21,31,0),(0,31,0),
   (0,20,7),(9,27,17),(6,23,30),(0,11,30),
   (0,0,18),(17,0,26),(26,0,29),(31,0,18)])
  self.assertIn('PersonalData->theme',theme)
  self.assertIn('/pymo/nds-theme.cfg',theme)
 def test_launcher_shortcuts(self):
  launcher=load("launcher.h")
  for text in ('if(down&(KEY_A|KEY_START))start=true;',
               'if(down&KEY_Y){launcher_theme_menu',
               'nds_ui("A / START 启动 · 轻触两次启动")',
               'nds_ui("上下选择 · B 关于 · X 语言 · Y 主题")',
               'launcher_rect(x,y,57,21,nds_theme_color(i))'):
   self.assertIn(text,launcher)
 def test_title_help_and_alpha_rounding(self):
  launcher=load("launcher.h")
  main=load("main.c")
  self.assertIn('nds_dual_title_allow_split(&nds_title_gate)',launcher)
  self.assertIn('nds_ui("UP / DOWN：选择项目")',launcher)
  self.assertIn('nds_ui("L：切换显示模式")',launcher)
  self.assertIn('else if(!debug_trace)launcher_play_panel(NULL)',main)
  self.assertIn('0x4010u',main)
  self.assertIn('0x0200u',main)

 def test_nondefault_colors_are_true_theme_families(self):
  theme=load("theme-nds.h")
  self.assertIn('nds_theme_tint(unsigned idx,unsigned strength,unsigned neutral)',theme)
  for expected in ('i<0?RGB15(3,5,8):nds_theme_tint((unsigned)i,6,1)',
                   'i<0?RGB15(5,10,16):nds_theme_tint((unsigned)i,11,3)',
                   'i<0?RGB15(7,15,23):nds_theme_tint((unsigned)i,11,4)'):
   self.assertIn(expected,theme)
  mat=re.search(r'nds_ipl_theme_rgb\[16\]\[3\]=\{(.*?)\};',theme,re.S)
  palette=[tuple(map(int,x)) for x in re.findall(r'\{(\d+),(\d+),(\d+)\}',mat.group(1))]
  def luminance(rgb):
   def linear(x):
    v=x/31
    return v/12.92 if v<=0.04045 else ((v+.055)/1.055)**2.4
   return sum(a*linear(b) for a,b in zip((.2126,.7152,.0722),rgb))
  for color in palette:
   bg=tuple(1+(c*6+15)//31 for c in color)
   selected=tuple(4+(c*11+15)//31 for c in color)
   self.assertNotEqual(bg,(3,5,8))
   self.assertGreaterEqual(1.05/(luminance(selected)+.05),4.5)
  red=tuple(1+(c*6+15)//31 for c in palette[2])
  self.assertGreater(red[0],red[2]) # no navy overlay on the red theme
 def test_default_and_auto_labels_are_centered(self):
  launcher=load("launcher.h")
  self.assertIn('launcher_text_pixel_width(',launcher)
  self.assertIn('launcher_text_centered(nds_theme_choice_name(i),x,w,36,white)',launcher)
  self.assertNotIn('nds_theme_choice_name(i),x+30',launcher)
 def test_restart_callback_rearms_title_guide(self):
  main=load("main.c");rmenu=load("cpymo_rmenu.c")
  self.assertIn('if (!err) nds_request_title_guide_reset();',rmenu)
  self.assertIn('if(!err&&nds_title_restart_pending)',main)
  self.assertIn('nds_dual_title_reset(&nds_title_gate);',main)
  self.assertIn('nds_aux_refresh();',main)

if __name__=="__main__":
 unittest.main()
