#!/usr/bin/python

import sys
import math
import numpy as np
from PIL import Image

assert(len(sys.argv) > 2)

im = Image.open(sys.argv[1])
assert(im.mode == "RGBA" or im.mode == "RGB")
RGB = list(zip(
    np.array(im.getdata(0), np.float),
    np.array(im.getdata(1), np.float),
    np.array(im.getdata(2), np.float)
))

def cvt5_floor(val):
    return math.floor(val / 8)

def cvt5_round(val):
    return max(0, min(31, round(val / 8)))

def minimized_average_error(error, i):
    width,height = im.size
    x = i % width
    y = i // width

    def weighted_add(a, b, w):
        return (a[0] + b[0] * w, a[1] + b[1] * w, a[2] + b[2] * w)

    # Row 0
    if(x < width - 1):
        RGB[i + 1] = weighted_add(RGB[i + 1], error, 7/48)
    if(x < width - 2):
        RGB[i + 2] = weighted_add(RGB[i + 2], error, 5/48)

    # Row 1
    if(y < height - 1):
        if(x >= 2):
            RGB[i + width - 2] = weighted_add(RGB[i + width - 2], error, 3/48)
        if(x >= 1):
            RGB[i + width - 1] = weighted_add(RGB[i + width - 1], error, 5/48)
        RGB[i + width] = weighted_add(RGB[i + width], error, 7/48)
        if(x < width - 1):
            RGB[i + width + 1] = weighted_add(RGB[i + width + 1], error, 5/48)
        if(x < width - 2):
            RGB[i + width + 2] = weighted_add(RGB[i + width + 2], error, 3/48)

    # Row 2
    if(y < height - 2):
        if(x >= 2):
            RGB[i + width * 2 - 2] = weighted_add(RGB[i + width * 2 - 2], error, 1/48)
        if(x >= 1):
            RGB[i + width * 2 - 1] = weighted_add(RGB[i + width * 2 - 1], error, 3/48)
        RGB[i + width * 2] = weighted_add(RGB[i + width * 2], error, 5/48)
        if(x < width - 1):
            RGB[i + width * 2 + 1] = weighted_add(RGB[i + width * 2 + 1], error, 3/48)
        if(x < width - 2):
            RGB[i + width * 2 + 2] = weighted_add(RGB[i + width * 2 + 2], error, 1/48)
    return

out = np.ones(len(RGB), dtype=np.uint16)
for i in range(0,len(RGB)):
    in_rgb = RGB[i]
    out_rgb = (
        cvt5_round(in_rgb[0]),
        cvt5_round(in_rgb[1]),
        cvt5_round(in_rgb[2])
    )
    out[i] = (
        out_rgb[0] * 2048 +
        out_rgb[1] *   64 +
        out_rgb[2] *    2 +
        1
    )
    error = (
        in_rgb[0] - out_rgb[0] * 8,
        in_rgb[1] - out_rgb[1] * 8,
        in_rgb[2] - out_rgb[2] * 8
    )
    minimized_average_error(error, i)

out.byteswap().tofile(sys.argv[2])
