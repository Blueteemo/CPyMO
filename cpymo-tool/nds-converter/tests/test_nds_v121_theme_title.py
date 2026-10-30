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

if __name__=="__main__":
 unittest.main()
