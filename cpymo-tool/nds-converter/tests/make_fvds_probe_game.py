"""Create a tiny original PyMO 1.2 FVDS *preflight* test game.

It deliberately contains synthetic *container-only* clips; they do not encode
visible movie frames or real music. Official converter generates the actual NDS
cache and verifies each packet; media decoder integration is a separate gate.
"""
import struct
import sys
from pathlib import Path
from PIL import Image, ImageDraw

ROOT=Path(sys.argv[1]).resolve()
for d in ("bg","script","video","save"):
    (ROOT/d).mkdir(parents=True,exist_ok=True)
(ROOT/"gameconfig.txt").write_text(
    "gametitle,FVDS视频资源检查\n"
    "platform,s60v3\nimagesize,320,240\nscripttype,pymo\n"
    "startscript,START\nengineversion,0.5\n"
    "bgformat,.png\ncharaformat,.png\ncharamaskformat,.png\n"
    "bgmformat,.wav\nvoiceformat,.wav\nseformat,.wav\n"
    "bgmvolume,5\nvovolume,5\nsevolume,5\nplayvideo,1\n"
    "fontsize,16\ntextcolor,#ffffff\n",encoding="utf-8")
script=[
    "#label LOOP",
    "#bg TEST,BG_NOFADE,0",
    "#say 系统,这是 CPyMO 1.2 视频资源测试。",
    "#say 系统,继续后检查第一段短视频容器。",
    "#movie CHECK1",
    "#say 系统,第一段返回剧情，请稍后查看检测报告。",
    "#say 系统,继续后检查第二段短视频容器。",
    "#movie CHECK3",
    "#say 系统,第二段返回剧情，本轮检测完成。",
    "#say 系统,本游戏尚不具备视频画面播放能力。",
    "#say 系统,检查游戏目录内两份 nds-fvds-probe 报告。",
    "#goto LOOP",
]
(ROOT/"script"/"START.txt").write_text("\n".join(script)+"\n",encoding="utf-8")
im=Image.new("RGB",(320,240),(16,32,55))
dr=ImageDraw.Draw(im)
for y in range(240):
    dr.line((0,y,319,y),fill=(16+y//15,32+y//10,55+y//7))
for x in range(12,309,17):
    dr.line((x,26,x,212),fill=(31,66,93),width=1)
for y in range(26,213,17):
    dr.line((12,y,307,y),fill=(31,66,93),width=1)
dr.rectangle((28,55,291,177),outline=(144,192,200),width=3)
dr.rectangle((42,82,278,154),fill=(32,51,77))
im.save(ROOT/"bg"/"TEST.png")
# The opt-in flag is placed in converted save/ AFTER conversion.
def synthetic_fvds(frames):
    data=bytearray(struct.pack("<4sHHIIHHII",b"FVDS",256,192,12,1,47605,2,frames,1))
    data+=struct.pack("<II",0,36)
    for i in range(frames):
        blocks=16 if i%2 else 15
        video=bytes([i&255])*32
        data+=struct.pack("<I",(blocks<<17)|len(video))
        data+=video+bytes(blocks*264)
    return data
(ROOT/"video"/"CHECK1.fv").write_bytes(synthetic_fvds(12))
(ROOT/"video"/"CHECK3.fv").write_bytes(synthetic_fvds(36))
print("Created synthetic FVDS preflight source:",ROOT)
