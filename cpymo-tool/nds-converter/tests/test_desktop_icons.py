"""Desktop converter artwork: immutable approved PNG, no further geometry edits."""
from __future__ import annotations
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from tools.build_desktop_icons import (build, SOURCE_SHA256, APPROVED_RGBA_SHA256,
                                      ARTWORK_ID, WINDOWS_SIZES, OPTICAL_WIDTH_SCALE,
                                      ico_frame_hashes, pixel_sha)

class DesktopIconBuildTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory()
        cls.source=ROOT/"assets/converter-icon-source.png"
        cls.out=Path(cls.temp.name)/"production"
        cls.manifest=build(cls.source,cls.out)
    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()
    def test_source_hash_and_size_are_pinned(self):
        self.assertEqual(hashlib.sha256(self.source.read_bytes()).hexdigest(),SOURCE_SHA256)
        with Image.open(self.source) as im:
            self.assertEqual(im.size,(1024,1024))
            self.assertEqual(pixel_sha(im),APPROVED_RGBA_SHA256)
    def test_master_is_byte_identical_to_approved_png(self):
        self.assertEqual(self.source.read_bytes(),(self.out/"converter-icon.png").read_bytes())
        self.assertEqual(OPTICAL_WIDTH_SCALE,1.0)
        self.assertFalse(self.manifest["reapplied_crop_or_mask"])
        self.assertFalse(self.manifest["lossy_reencoding"])
        self.assertFalse((self.out/"converter-icon-source-clean.png").exists())
    def test_windows_ico_has_seven_exact_source_resizes(self):
        with Image.open(self.source) as source, Image.open(self.out/"converter-icon.ico") as icon:
            self.assertEqual(set(icon.info["sizes"]),set(WINDOWS_SIZES))
            for size in WINDOWS_SIZES:
                actual=icon.ico.getimage(size).convert("RGBA")
                expected=source.convert("RGBA").resize(size,Image.Resampling.LANCZOS)
                self.assertEqual(actual.tobytes(),expected.tobytes(),size)
        frames=ico_frame_hashes(self.out/"converter-icon.ico")
        self.assertEqual(frames,self.manifest["windows_ico_frames_sha256"])
        self.assertEqual(len(frames),7)
    def test_macos_largest_icns_retains_all_source_pixels(self):
        with Image.open(self.out/"converter-icon.icns") as icon:
            self.assertEqual(icon.size,(1024,1024))
            self.assertEqual(pixel_sha(icon),APPROVED_RGBA_SHA256)
            self.assertIn((512,512,2),icon.info["sizes"])
    def test_gui_png_matches_direct_resample(self):
        with Image.open(self.source) as src, Image.open(self.out/"converter-icon-64.png") as small:
            expected=src.convert("RGBA").resize((64,64),Image.Resampling.LANCZOS)
            self.assertEqual(small.convert("RGBA").tobytes(),expected.tobytes())
    def test_manifest_hashes_cover_actual_outputs(self):
        manifest=json.loads((self.out/"icon-manifest.json").read_text())
        self.assertEqual(manifest["artwork_id"],ARTWORK_ID)
        for item in manifest["outputs"].values():
            data=(self.out/item["file"]).read_bytes()
            self.assertEqual(len(data),item["bytes"])
            self.assertEqual(hashlib.sha256(data).hexdigest(),item["sha256"])
    def test_alternate_image_is_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            f=Path(td)/"fake.png"
            Image.new("RGBA",(1024,1024),(0,0,0,0)).save(f)
            with self.assertRaises(ValueError):build(f,Path(td)/"out")
    def test_wrong_dimensions_and_opaque_source_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            f=Path(td)/"fixture.png"
            Image.new("RGBA",(32,32),(0,0,0,0)).save(f)
            with self.assertRaises(ValueError):build(f,Path(td)/"out",allow_alternate_source=True)
            Image.new("RGBA",(1024,1024),(10,20,30,255)).save(f)
            with self.assertRaises(ValueError):build(f,Path(td)/"out",allow_alternate_source=True)
    def test_packagers_embed_and_preserve_new_manifest(self):
        for name in ("build_portable.py","build_portable_macos.py"):
            text=(ROOT/name).read_text(encoding="utf8")
            self.assertIn("assets/converter-icon-source.png",text)
            self.assertIn("ICON-MANIFEST.json",text)
            self.assertNotIn('desktop_icon="approved-PyMO-04"',text)
            self.assertIn('desktop_icon_pixel_sha256=',text)
    def test_app_icon_resource_is_nonfatal_when_running_source(self):
        text=(ROOT/"app.py").read_text(encoding="utf8")
        self.assertIn("def apply_desktop_icon(window):",text)
        self.assertIn("window._cpymo_icon_photo=photo",text)
        self.assertNotIn("cpymo-backends/nds/assets/icon.png",text)

if __name__=="__main__":unittest.main()
