"""Packaging regression for PyMO 04 desktop icon; never rewrites the NDS banner."""
from __future__ import annotations
import sys
import tempfile
from pathlib import Path
import unittest
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from tools.build_desktop_icons import build, SOURCE_SHA256, WINDOWS_SIZES, OPTICAL_WIDTH_SCALE, APPROVED_CLEAN_RGBA_SHA256


class DesktopIconBuildTests(unittest.TestCase):
    def test_source_hash_is_pinned(self):
        self.assertEqual(len(SOURCE_SHA256),64)
        self.assertIn("assets/converter-icon-source.png", (ROOT/"build_portable_macos.py").read_text(encoding="utf8"))
        self.assertIn("--require-desktop-icon", (ROOT/"build_portable.py").read_text(encoding="utf8"))

    def test_generator_produces_actual_multi_resolution_icons(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            # Synthetic drawing is ONLY a test fixture. A public release must
            # use the real pinned image; the fixture cannot pass that check.
            synthetic=Image.new("RGBA",(1254,1254),(0,0,0,0))
            d=ImageDraw.Draw(synthetic)
            d.ellipse((90,95,1165,1155),fill=(225,171,95,255))
            source=root/"fixture.png"
            synthetic.save(source)
            with self.assertRaises(ValueError):
                build(source,root/"forbidden")
            output=root/"generated"
            manifest=build(source,output,allow_alternate_source=True)
            self.assertIn("outputs",manifest)
            self.assertEqual(manifest["optical_width_scale"],0.976)
            self.assertAlmostEqual(OPTICAL_WIDTH_SCALE,0.976)
            for item in ("png","png_64","ico","icns"):
                self.assertGreater(manifest["outputs"][item]["bytes"],100)
            self.assertTrue((output/"converter-icon-source-clean.png").is_file())
            with Image.open(output/"converter-icon.png") as im:
                self.assertEqual(im.size,(1024,1024))
                bbox=im.getchannel("A").getbbox()
                self.assertLessEqual(abs((bbox[2]-bbox[0])-(bbox[3]-bbox[1])),18)
            with Image.open(output/"converter-icon.ico") as im:
                self.assertTrue(set(WINDOWS_SIZES)<=set(im.info["sizes"]))
            with Image.open(output/"converter-icon.icns") as im:
                self.assertEqual(im.size,(1024,1024))
                self.assertIn((512,512,2),im.info["sizes"])

    def test_approved_production_master_matches_pixel_hash(self):
        source=ROOT/"assets/converter-icon-source.png"
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/"approved"
            manifest=build(source,out)
            self.assertEqual(manifest["approved_clean_pixel_sha256"],
                             APPROVED_CLEAN_RGBA_SHA256)
            with Image.open(source) as original, Image.open(
                out/"converter-icon-source-clean.png"
            ) as modified:
                self.assertEqual(original.convert("RGB").tobytes(),
                                 modified.convert("RGB").tobytes())
                self.assertEqual(modified.getchannel("A").getbbox(),
                                 (84,99,1171,1159))

    def test_app_icon_resource_is_nonfatal_when_running_source(self):
        app=(ROOT/"app.py").read_text(encoding="utf8")
        self.assertIn("def apply_desktop_icon(window):",app)
        self.assertIn("window._cpymo_icon_photo=photo",app)
        self.assertIn("apply_desktop_icon(window)",app)
        self.assertNotIn("cpymo-backends/nds/assets/icon.png",app)


if __name__=="__main__":
    unittest.main()
