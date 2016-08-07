/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Matthieu DARBOIS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Multiple inclusion protection */
#ifndef YABMP_INFO_H
#define YABMP_INFO_H

#include "yabmp_api.h"

#define YABMP_FILE_TYPE(a, b) ((yabmp_uint16)a | (((yabmp_uint16)b) << 8))

#define YABMP_COLOR_SHIFT 0
#define YABMP_COLOR_MASK  (YABMP_COLOR_MASK_PALETTE | YABMP_COLOR_MASK_COLOR | YABMP_COLOR_MASK_ALPHA | YABMP_COLOR_MASK_BITFIELDS)

#define YABMP_SCAN_SHIFT 4
#define YABMP_SCAN_MASK  (YABMP_SCAN_TOP_DOWN)

struct yabmp_info_struct
{
	size_t        rowbytes;         /* Number of bytes needed for 1 line */
	yabmp_uint8*  icc_profile;      /* ICC profile */
	yabmp_uint32  icc_profile_size; /* ICC profile size */
	yabmp_uint32  width;            /* Width of the image in pixels  */
	yabmp_uint32  height;           /* Height of the image in pixels */
	yabmp_uint32  res_ppm_x;        /* Horizontal resolution in pixels/meter */
	yabmp_uint32  res_ppm_y;        /* Vertical resolution in pixels/meter */
	yabmp_uint32  compression;      /* Compression method */
	yabmp_uint32  mask_blue;        /* blue-channel bit mask */
	yabmp_uint32  mask_green;       /* green-channel bit mask */
	yabmp_uint32  mask_red;         /* red-channel bit mask */
	yabmp_uint32  mask_alpha;       /* alpha-channel bit mask */
	yabmp_cie_xyz cie_r;            /* CIE XYZ for R */
	yabmp_cie_xyz cie_g;            /* CIE XYZ for G */
	yabmp_cie_xyz cie_b;            /* CIE XYZ for B */
	yabmp_q16d16  gamma_r;          /* gamma for red channel */
	yabmp_q16d16  gamma_g;          /* gamma for green channel */
	yabmp_q16d16  gamma_b;          /* gamma for blue channel */
	unsigned int  num_palette;      /* Number of colors in the palette */
	yabmp_uint8   bpp;              /* Number of bits per pixel */
	yabmp_uint8   bpc_blue;         /* blue-channel bpp */
	yabmp_uint8   bpc_green;        /* green-channel bpp */
	yabmp_uint8   bpc_red;          /* red-channel bpp */
	yabmp_uint8   bpc_alpha;        /* alpha-channel bpp */
	yabmp_uint8   cp_type;          /* color profile type */
	yabmp_uint8   cp_intent;        /* color profile intent */
	yabmp_uint8   flags;            /* color mask, top-down */
	yabmp_color   palette[256];     /* palette */
	yabmp_uint8   expanded_bps;     /* maximum bit count for one color when expanded (8, 16 or 32) */
};

YABMP_IAPI(void, yabmp_init_info, (yabmp_info* info));
YABMP_IAPI(void, yabmp_destroy_info, (yabmp* instance, yabmp_info** info));

#endif
