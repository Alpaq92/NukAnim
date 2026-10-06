#!/usr/bin/env python3
"""gif.py CLIPS OUTDIR SCENE...

Runs the clips example for each scene and turns its raw frames into
OUTDIR/SCENE.gif. Needs Pillow. Run from the repository's root.
"""
import os
import struct
import subprocess
import sys
import tempfile

from PIL import Image


def frames(path):
    with open(path, "rb") as f:
        magic, w, h, ms = struct.unpack("<4I", f.read(16))
        if magic != 0x464B414E:
            sys.exit(f"{path}: not a clips file")
        size = w * h * 4
        while True:
            data = f.read(size)
            if len(data) < size:
                return
            yield ms, Image.frombytes("RGBA", (w, h), data).convert("RGB")


def main(argv):
    if len(argv) < 3:
        sys.exit(__doc__)
    clips, outdir, scenes = argv[0], argv[1], argv[2:]
    os.makedirs(outdir, exist_ok=True)
    for scene in scenes:
        with tempfile.TemporaryDirectory() as tmp:
            raw = os.path.join(tmp, scene + ".raw")
            subprocess.run([clips, scene, raw], check=True)
            images, ms = [], 30
            for ms, image in frames(raw):
                images.append(image)
        picks = images[:: max(1, len(images) // 16)]
        w, h = picks[0].size
        mosaic = Image.new("RGB", (w, h * len(picks)))
        for k, image in enumerate(picks):
            mosaic.paste(image, (0, k * h))
        palette = mosaic.quantize(colors=255, method=Image.Quantize.MEDIANCUT)
        quantized = [image.quantize(palette=palette, dither=Image.Dither.NONE) for image in images]
        out = os.path.join(outdir, scene + ".gif")
        quantized[0].save(out, save_all=True, append_images=quantized[1:], duration=ms, loop=0, optimize=True)
        print(f"{out}: {len(images)} frames, {os.path.getsize(out) // 1024} KiB")


if __name__ == "__main__":
    main(sys.argv[1:])
