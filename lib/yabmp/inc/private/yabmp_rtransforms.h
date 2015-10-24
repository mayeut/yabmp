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
#ifndef YABMP_RTRANSFORMS_H
#define YABMP_RTRANSFORMS_H

#include "yabmp_api.h"

#if defined(YABMP_BIG_ENDIAN)
YABMP_IAPI(void, yabmp_swap16u, (const yabmp* instance, yabmp_uint16* pSrcDst));
YABMP_IAPI(void, yabmp_swap32u, (const yabmp* instance, yabmp_uint32* pSrcDst));
#endif

YABMP_IAPI(void, yabmp_bf16u_to_bgr24,  (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint8*  pDst ));
YABMP_IAPI(void, yabmp_bf16u_to_bgr48,  (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint16* pDst ));
YABMP_IAPI(void, yabmp_bf16u_to_bgra32, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint8*  pDst ));
YABMP_IAPI(void, yabmp_bf16u_to_bgra64, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint16* pDst ));
YABMP_IAPI(void, yabmp_bf32u_to_bgr24,  (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint8*  pDst ));
YABMP_IAPI(void, yabmp_bf32u_to_bgr48,  (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint16* pDst ));
YABMP_IAPI(void, yabmp_bf32u_to_bgra32, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint8*  pDst ));
YABMP_IAPI(void, yabmp_bf32u_to_bgra64, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint16* pDst ));
YABMP_IAPI(void, yabmp_bitfield_get_shift_and_bits, (yabmp_uint32 mask, unsigned int* shift, unsigned int* bits));

YABMP_IAPI(void, yabmp_pal1_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal2_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal4_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal8_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));

YABMP_IAPI(void, yabmp_pal1_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal2_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal4_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));
YABMP_IAPI(void, yabmp_pal8_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ));

#endif /* YABMP_RTRANSFORMS_H */
