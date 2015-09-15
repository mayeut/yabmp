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

typedef struct {
	yabmp_uint16 type;       /* 'BM' for Win Bitmap, 'BA' for OS/2 */
	yabmp_uint32 fileSize;   /* File size in bytes         */
	yabmp_uint16 reserved1;  /* Reserved                   */
	yabmp_uint16 reserved2;  /* Reserved                   */
	yabmp_uint32 dataOffset; /* Image data offset in bytes */
} yabmp_file_header;

typedef struct {
	yabmp_uint32 size;                /* Size of the dib header */
	yabmp_uint32 width;               /* Width of the image in pixels */
	yabmp_uint32 height;              /* Heigth of the image in pixels */
	yabmp_uint16 colorPlaneCount;     /* Number of color planes */
	yabmp_uint16 bpp;                 /* Number of bits per pixels */
} yabmp_dib_core_info;

typedef struct {
	yabmp_uint32 compression;         /* Compression method */
	yabmp_uint32 rawDataSize;         /* Size of the raw data in bytes */
	yabmp_uint32 resXppm;             /* Horizontal resolution in pixels/meter */
	yabmp_uint32 resYppm;             /* Vertical resolution in pixels/meter */
	yabmp_uint32 pltColorCount;       /* Number of colors in the palette */
	yabmp_uint32 importantColorCount; /* Number of important color */
} yabmp_dib_v1_info;

typedef struct {
	yabmp_uint32 redMask;             /* Red channel bit mask */
	yabmp_uint32 greenMask;           /* Green channel bit mask */
	yabmp_uint32 blueMask;            /* Blue channel bit mask */
} yabmp_dib_v2_info;

typedef struct {
	yabmp_uint32 alphaMask;           /* Red channel bit mask */
} yabmp_dib_v3_info;

typedef struct {
	yabmp_uint32 colorSpaceType;   /* Color space type */
	yabmp_uint8  colorSpaceEP[36]; /* Color space end points */
	yabmp_uint32 redGamma;         /* Red channel gamma */
	yabmp_uint32 greenGamma;       /* Green channel gamma */
	yabmp_uint32 blueGamma;        /* Blue channel gamma */
} yabmp_dib_v4_info;

typedef struct {
	yabmp_uint32 intent;           /* Intent */
	yabmp_uint32 iccProfileData;   /* ICC profile data */
	yabmp_uint32 iccProfileSize;   /* ICC profile size */
	yabmp_uint32 reserved;         /* Reserved */
} yabmp_dib_v5_info;


struct yabmp_info_struct
{
	/* FILE cache */
	yabmp_file_header   file;
	yabmp_dib_core_info core;
	yabmp_dib_v1_info   v1;
	yabmp_dib_v2_info   v2;
	yabmp_dib_v3_info   v3;
	yabmp_dib_v4_info   v4;
	yabmp_dib_v5_info   v5;
	
	yabmp_uint8 lutR[256];
	yabmp_uint8 lutG[256];
	yabmp_uint8 lutB[256];
	yabmp_uint8 lutA[256];
	
	/* Computed cache */
	yabmp_uint32 colorMask;
	yabmp_uint8  expanded_bpp; /* maximum bit count for one color when expanded (8, 16 or 32) */
};

YABMP_IAPI(void, yabmp_init_info, (yabmp_info* info));

#endif
