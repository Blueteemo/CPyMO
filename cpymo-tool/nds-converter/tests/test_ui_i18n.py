"""Regression checks for release-bound CN/EN conversion assistant strings."""
import sys
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from ui_i18n import tr, tr_log, LANGUAGES


class ConverterLanguageTests(unittest.TestCase):
    def test_player_facing_language_choices(self):
        self.assertEqual(LANGUAGES,('简体中文','English'))
        self.assertEqual(tr('选压缩包'), '选压缩包')
        self.assertEqual(tr('选压缩包','English'),'Open ZIP / APK')
        self.assertEqual(tr('转换视频 NVL1（推荐：兼容优先）','English'),
                         'Convert to NVL1 (recommended)')
        self.assertEqual(tr('转换视频 NVA1（压缩音频）','English'),
                         'Convert to NVA1 (compressed audio)')

    def test_versioned_caption_is_not_hardcoded(self):
        self.assertIn('1.2.0',tr('安装包已包含 CPyMO 1.2.0；缓存压缩不会进一步降低转换后素材质量。','English'))
        self.assertIn('1.3.0',tr('安装包已包含 CPyMO 1.3.0；缓存压缩不会进一步降低转换后素材质量。','English'))

    def test_no_r4_only_onboarding_or_old_video_default(self):
        app=(ROOT/'app.py').read_text(encoding='utf8')
        guide=(ROOT/'helper_core.py').read_text(encoding='utf8')
        self.assertNotIn('生成可复制到 R4 的游戏目录',app)
        self.assertNotIn('视频可选择转换。',app)
        self.assertIn("self.video=tk.StringVar(value=self.video_modes[1])",app)
        self.assertIn("'missing_policy':'box'",app)
        self.assertIn("'ui_language':self.language.get()",app)
        self.assertIn('INSTALL-READ-ME.txt',guide)
        self.assertNotIn('保留R4内核',guide)

    def test_known_diagnostic_prefixes_are_translated_only_for_view(self):
        raw='字库缺字以□显示，原脚本未改：U+1F984\\nError detail unknown: x'
        translated=tr_log(raw,'English')
        self.assertIn('Missing glyphs displayed as □',translated)
        self.assertIn('U+1F984',translated)
        self.assertIn('Error detail unknown: x',translated)
        self.assertEqual(tr_log(raw,'简体中文'),raw)

    def test_unknown_diagnostics_are_preserved(self):
        raw='Unmapped PyMO error for source investigation: /tmp/foo'
        self.assertEqual(tr(raw,'English'),raw)


if __name__=='__main__':unittest.main()
