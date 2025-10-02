#!/usr/bin/env python3
# png_to_rgb332_raw.py
# Convert an image to 8-bit RGB332 raw (row-major, top-down), default 320x240.

import argparse

from PIL import Image

def rgb332(r, g, b):
    # 3 bits R, 3 bits G, 2 bits B  -> RRR GGG BB
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)

def main():
    p = argparse.ArgumentParser(description="Convert image to 8-bit RGB332 raw")
    p.add_argument("input", help="input image (PNG/JPG/etc.)")
    p.add_argument("output", help="output .raw path")
    p.add_argument("--size", default="320x240",
                   help="output size WxH (default 320x240)")
    p.add_argument("--filter", choices=["nearest","bilinear"], default="nearest",
                   help="resize filter (default nearest for crisp pixel art)")
    args = p.parse_args()

    # Parse size
    try:
        W, H = map(int, args.size.lower().split("x"))
    except Exception:
        raise SystemExit("Bad --size. Use like: 320x240")

    # Load & convert to RGB
    img = Image.open(args.input).convert("RGB")
    if img.size != (W, H):
        resample = Image.NEAREST if args.filter == "nearest" else Image.BILINEAR
        img = img.resize((W, H), resample)

    # Pack to RGB332
    pix = img.load()
    buf = bytearray(W * H)
    k = 0
    for y in range(H):
        for x in range(W):
            r, g, b = pix[x, y]
            buf[k] = rgb332(r, g, b)
            k += 1

    with open(args.output, "wb") as f:
        f.write(buf)

    print(f"wrote {len(buf)} bytes to {args.output} (format: 8-bit RGB332, {W}x{H}, row-major, top-down)")

if __name__ == "__main__":
    main()