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

#include "../inc/private/yabmp_rtransforms.h"
#include "../inc/private/yabmp_struct.h"

YABMP_IAPI(void, yabmp_bitfield_get_shift_and_bits, (yabmp_uint32 mask, unsigned int* shift, unsigned int* bits))
{
	unsigned int l_shift, l_bits;
	
	assert(shift != NULL);
	assert(bits != NULL);
	
	l_shift = l_bits = 0U;
	
	if (mask != 0U) {
		/* get shift */
		while ((mask & 1U) == 0U) {
			mask >>= 1;
			l_shift++;
		}
		
		/* count bits */
		mask = mask - ((mask >> 1) & 0x55555555U);
		mask = (mask & 0x33333333U) + ((mask >> 2) & 0x33333333U);
		l_bits = ((mask + (mask >> 4) & 0xF0F0F0FU) * 0x1010101U) >> 24;
	}
	*shift = l_shift;
	*bits = l_bits;
}

YABMP_IAPI(void, yabmp_bf32u_to_bgr24, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint8* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift;
	yabmp_uint32 l_blue_mask, l_green_mask, l_red_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = instance->info.v2.blueMask;
	l_green_mask = instance->info.v2.greenMask;
	l_red_mask   = instance->info.v2.redMask;
	
	if ((l_blue_mask | l_green_mask | l_red_mask) == 0U) {
		/* default color mask - no compression */
		l_blue_mask  = 0x0000FFU;
		l_green_mask = 0x00FF00U;
		l_red_mask   = 0xFF0000U;
	}
	
	yabmp_bitfield_get_shift_and_bits(l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_red_mask, &l_red_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint32 l_value = pSrc[x];
		
		pDst[3*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[3*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[3*x+2] = (l_value & l_red_mask)   >> l_red_shift;
	}
}

YABMP_IAPI(void, yabmp_bf32u_to_bgr48, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint16* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift;
	yabmp_uint32 l_blue_mask, l_green_mask, l_red_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = instance->info.v2.blueMask;
	l_green_mask = instance->info.v2.greenMask;
	l_red_mask   = instance->info.v2.redMask;
	
	yabmp_bitfield_get_shift_and_bits(l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_red_mask, &l_red_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint32 l_value = pSrc[x];
		
		pDst[3*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[3*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[3*x+2] = (l_value & l_red_mask)   >> l_red_shift;
	}
}

YABMP_IAPI(void, yabmp_bf32u_to_bgra32, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint8* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift, l_alpha_shift;
	yabmp_uint32 l_blue_mask, l_green_mask, l_red_mask, l_alpha_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = instance->info.v2.blueMask;
	l_green_mask = instance->info.v2.greenMask;
	l_red_mask   = instance->info.v2.redMask;
	l_alpha_mask = instance->info.v3.alphaMask;
	
	yabmp_bitfield_get_shift_and_bits(l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_red_mask, &l_red_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_alpha_mask, &l_alpha_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint32 l_value = pSrc[x];
		
		pDst[4*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[4*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[4*x+2] = (l_value & l_red_mask)   >> l_red_shift;
		pDst[4*x+3] = (l_value & l_alpha_mask) >> l_alpha_shift;
	}
}

YABMP_IAPI(void, yabmp_bf32u_to_bgra64, (const yabmp* instance, const yabmp_uint32* pSrc, yabmp_uint16* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift, l_alpha_shift;
	yabmp_uint32 l_blue_mask, l_green_mask, l_red_mask, l_alpha_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = instance->info.v2.blueMask;
	l_green_mask = instance->info.v2.greenMask;
	l_red_mask   = instance->info.v2.redMask;
	l_alpha_mask = instance->info.v3.alphaMask;
	
	yabmp_bitfield_get_shift_and_bits(l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_red_mask, &l_red_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits(l_alpha_mask, &l_alpha_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint32 l_value = pSrc[x];
		
		pDst[4*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[4*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[4*x+2] = (l_value & l_red_mask)   >> l_red_shift;
		pDst[4*x+3] = (l_value & l_alpha_mask) >> l_alpha_shift;
	}
}

YABMP_IAPI(void, yabmp_bf16u_to_bgr24, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint8* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift;
	yabmp_uint16 l_blue_mask, l_green_mask, l_red_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = (yabmp_uint16)instance->info.v2.blueMask;
	l_green_mask = (yabmp_uint16)instance->info.v2.greenMask;
	l_red_mask   = (yabmp_uint16)instance->info.v2.redMask;
	
	if ((l_blue_mask | l_green_mask | l_red_mask) == 0U) {
		/* default color mask - no compression */
		l_blue_mask  = 0x001FU;
		l_green_mask = 0x03E0U;
		l_red_mask   = 0x7C00U;
	}
	
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_red_mask, &l_red_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint16 l_value = pSrc[x];
		
		pDst[3*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[3*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[3*x+2] = (l_value & l_red_mask)   >> l_red_shift;
	}
}
YABMP_IAPI(void, yabmp_bf16u_to_bgr48, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint16* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift;
	yabmp_uint16 l_blue_mask, l_green_mask, l_red_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = (yabmp_uint16)instance->info.v2.blueMask;
	l_green_mask = (yabmp_uint16)instance->info.v2.greenMask;
	l_red_mask   = (yabmp_uint16)instance->info.v2.redMask;
	
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_red_mask, &l_red_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint16 l_value = pSrc[x];
		
		pDst[3*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[3*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[3*x+2] = (l_value & l_red_mask)   >> l_red_shift;
	}
}
YABMP_IAPI(void, yabmp_bf16u_to_bgra32, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint8* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift, l_alpha_shift;
	yabmp_uint16 l_blue_mask, l_green_mask, l_red_mask, l_alpha_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = (yabmp_uint16)instance->info.v2.blueMask;
	l_green_mask = (yabmp_uint16)instance->info.v2.greenMask;
	l_red_mask   = (yabmp_uint16)instance->info.v2.redMask;
	l_alpha_mask = (yabmp_uint16)instance->info.v3.alphaMask;
	
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_red_mask, &l_red_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_alpha_mask, &l_alpha_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint16 l_value = pSrc[x];
		
		pDst[4*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[4*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[4*x+2] = (l_value & l_red_mask)   >> l_red_shift;
		pDst[4*x+3] = (l_value & l_alpha_mask) >> l_alpha_shift;
	}
}

YABMP_IAPI(void, yabmp_bf16u_to_bgra64, (const yabmp* instance, const yabmp_uint16* pSrc, yabmp_uint16* pDst ))
{
	unsigned int l_dummy_bits;
	unsigned int l_blue_shift, l_green_shift, l_red_shift, l_alpha_shift;
	yabmp_uint16 l_blue_mask, l_green_mask, l_red_mask, l_alpha_mask;
	
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	l_blue_mask  = (yabmp_uint16)instance->info.v2.blueMask;
	l_green_mask = (yabmp_uint16)instance->info.v2.greenMask;
	l_red_mask   = (yabmp_uint16)instance->info.v2.redMask;
	l_alpha_mask = (yabmp_uint16)instance->info.v3.alphaMask;
	
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_blue_mask, &l_blue_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_green_mask, &l_green_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_red_mask, &l_red_shift, &l_dummy_bits);
	yabmp_bitfield_get_shift_and_bits((yabmp_uint32)l_alpha_mask, &l_alpha_shift, &l_dummy_bits);
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint16 l_value = pSrc[x];
		
		pDst[4*x+0] = (l_value & l_blue_mask)  >> l_blue_shift;
		pDst[4*x+1] = (l_value & l_green_mask) >> l_green_shift;
		pDst[4*x+2] = (l_value & l_red_mask)   >> l_red_shift;
		pDst[4*x+3] = (l_value & l_alpha_mask) >> l_alpha_shift;
	}
}

YABMP_IAPI(void, yabmp_pal1_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 8U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		
		l_current = (l_value >> 7) & 0x01;
		pDst[24*x+0] = instance->info.lutB[l_current];
		pDst[24*x+1] = instance->info.lutG[l_current];
		pDst[24*x+2] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 6) & 0x01;
		pDst[24*x+3] = instance->info.lutB[l_current];
		pDst[24*x+4] = instance->info.lutG[l_current];
		pDst[24*x+5] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 5) & 0x01;
		pDst[24*x+6] = instance->info.lutB[l_current];
		pDst[24*x+7] = instance->info.lutG[l_current];
		pDst[24*x+8] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 4) & 0x01;
		pDst[24*x+9] = instance->info.lutB[l_current];
		pDst[24*x+10] = instance->info.lutG[l_current];
		pDst[24*x+11] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 3) & 0x01;
		pDst[24*x+12] = instance->info.lutB[l_current];
		pDst[24*x+13] = instance->info.lutG[l_current];
		pDst[24*x+14] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 2) & 0x01;
		pDst[24*x+15] = instance->info.lutB[l_current];
		pDst[24*x+16] = instance->info.lutG[l_current];
		pDst[24*x+17] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 1) & 0x01;
		pDst[24*x+18] = instance->info.lutB[l_current];
		pDst[24*x+19] = instance->info.lutG[l_current];
		pDst[24*x+20] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 0) & 0x01;
		pDst[24*x+21] = instance->info.lutB[l_current];
		pDst[24*x+22] = instance->info.lutG[l_current];
		pDst[24*x+23] = instance->info.lutR[l_current];
	}
	
	if (l_width & 7U) {
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		int l_remaining = (int)l_width & 7U;
		
		l_current = (l_value >> 7) & 0x01;
		pDst[24*x+0] = instance->info.lutB[l_current];
		pDst[24*x+1] = instance->info.lutG[l_current];
		pDst[24*x+2] = instance->info.lutR[l_current];
		
		if (l_remaining > 1) {
			l_current = (l_value >> 6) & 0x01;
			pDst[24*x+3] = instance->info.lutB[l_current];
			pDst[24*x+4] = instance->info.lutG[l_current];
			pDst[24*x+5] = instance->info.lutR[l_current];
		}
		if (l_remaining > 2) {
			l_current = (l_value >> 5) & 0x01;
			pDst[24*x+6] = instance->info.lutB[l_current];
			pDst[24*x+7] = instance->info.lutG[l_current];
			pDst[24*x+8] = instance->info.lutR[l_current];
		}
		if (l_remaining > 3) {
			l_current = (l_value >> 4) & 0x01;
			pDst[24*x+9] = instance->info.lutB[l_current];
			pDst[24*x+10] = instance->info.lutG[l_current];
			pDst[24*x+11] = instance->info.lutR[l_current];
		}
		if (l_remaining > 4) {
			l_current = (l_value >> 3) & 0x01;
			pDst[24*x+12] = instance->info.lutB[l_current];
			pDst[24*x+13] = instance->info.lutG[l_current];
			pDst[24*x+14] = instance->info.lutR[l_current];
		}
		if (l_remaining > 5) {
			l_current = (l_value >> 2) & 0x01;
			pDst[24*x+15] = instance->info.lutB[l_current];
			pDst[24*x+16] = instance->info.lutG[l_current];
			pDst[24*x+17] = instance->info.lutR[l_current];
		}
		if (l_remaining > 6) {
			l_current = (l_value >> 1) & 0x01;
			pDst[24*x+18] = instance->info.lutB[l_current];
			pDst[24*x+19] = instance->info.lutG[l_current];
			pDst[24*x+20] = instance->info.lutR[l_current];
		}
	}
}

YABMP_IAPI(void, yabmp_pal2_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 4U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		
		l_current = (l_value >> 6) & 0x03;
		pDst[12*x+0] = instance->info.lutB[l_current];
		pDst[12*x+1] = instance->info.lutG[l_current];
		pDst[12*x+2] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 4) & 0x03;
		pDst[12*x+3] = instance->info.lutB[l_current];
		pDst[12*x+4] = instance->info.lutG[l_current];
		pDst[12*x+5] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 2) & 0x03;
		pDst[12*x+6] = instance->info.lutB[l_current];
		pDst[12*x+7] = instance->info.lutG[l_current];
		pDst[12*x+8] = instance->info.lutR[l_current];
		
		l_current = (l_value >> 0) & 0x03;
		pDst[12*x+9] = instance->info.lutB[l_current];
		pDst[12*x+10] = instance->info.lutG[l_current];
		pDst[12*x+11] = instance->info.lutR[l_current];
	}
	
	if (l_width & 3U) {
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		int l_remaining = (int)l_width & 3U;
		
		l_current = (l_value >> 6) & 0x03;
		pDst[12*x+0] = instance->info.lutB[l_current];
		pDst[12*x+1] = instance->info.lutG[l_current];
		pDst[12*x+2] = instance->info.lutR[l_current];
		
		if (l_remaining > 1) {
			l_current = (l_value >> 4) & 0x03;
			pDst[12*x+3] = instance->info.lutB[l_current];
			pDst[12*x+4] = instance->info.lutG[l_current];
			pDst[12*x+5] = instance->info.lutR[l_current];
		}
		if (l_remaining > 2) {
			l_current = (l_value >> 2) & 0x03;
			pDst[12*x+6] = instance->info.lutB[l_current];
			pDst[12*x+7] = instance->info.lutG[l_current];
			pDst[12*x+8] = instance->info.lutR[l_current];
		}
	}
}

YABMP_IAPI(void, yabmp_pal4_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 2U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_value_lo = l_value & 0x0F;
		
		l_value >>= 4;
		
		pDst[6*x+0] = instance->info.lutB[l_value];
		pDst[6*x+1] = instance->info.lutG[l_value];
		pDst[6*x+2] = instance->info.lutR[l_value];
		
		pDst[6*x+3] = instance->info.lutB[l_value_lo];
		pDst[6*x+4] = instance->info.lutG[l_value_lo];
		pDst[6*x+5] = instance->info.lutR[l_value_lo];
	}
	
	if (l_width & 1U) {
		yabmp_uint8 l_value = pSrc[x] >> 4;
		
		pDst[6*x+0] = instance->info.lutB[l_value];
		pDst[6*x+1] = instance->info.lutG[l_value];
		pDst[6*x+2] = instance->info.lutR[l_value];
	}
}

YABMP_IAPI(void, yabmp_pal8_to_bgr24, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		
		pDst[3*x+0] = instance->info.lutB[l_value];
		pDst[3*x+1] = instance->info.lutG[l_value];
		pDst[3*x+2] = instance->info.lutR[l_value];
	}
}

YABMP_IAPI(void, yabmp_pal1_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 8U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		
		l_current = (l_value >> 7) & 0x01;
		pDst[8*x+0] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 6) & 0x01;
		pDst[8*x+1] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 5) & 0x01;
		pDst[8*x+2] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 4) & 0x01;
		pDst[8*x+3] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 3) & 0x01;
		pDst[8*x+4] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 2) & 0x01;
		pDst[8*x+5] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 1) & 0x01;
		pDst[8*x+6] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 0) & 0x01;
		pDst[8*x+7] = instance->info.lutB[l_current];
	}
	
	if (l_width & 7U) {
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		int l_remaining = (int)l_width & 7U;
		
		l_current = (l_value >> 7) & 0x01;
		pDst[8*x+0] = instance->info.lutB[l_current];
		
		if (l_remaining > 1) {
			l_current = (l_value >> 6) & 0x01;
			pDst[8*x+1] = instance->info.lutB[l_current];
		}
		if (l_remaining > 2) {
			l_current = (l_value >> 5) & 0x01;
			pDst[8*x+2] = instance->info.lutB[l_current];
		}
		if (l_remaining > 3) {
			l_current = (l_value >> 4) & 0x01;
			pDst[8*x+3] = instance->info.lutB[l_current];
		}
		if (l_remaining > 4) {
			l_current = (l_value >> 3) & 0x01;
			pDst[8*x+4] = instance->info.lutB[l_current];
		}
		if (l_remaining > 5) {
			l_current = (l_value >> 2) & 0x01;
			pDst[8*x+5] = instance->info.lutB[l_current];
		}
		if (l_remaining > 6) {
			l_current = (l_value >> 1) & 0x01;
			pDst[8*x+6] = instance->info.lutB[l_current];
		}
	}
}

YABMP_IAPI(void, yabmp_pal2_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 4U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		
		l_current = (l_value >> 6) & 0x03;
		pDst[4*x+0] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 4) & 0x03;
		pDst[4*x+1] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 2) & 0x03;
		pDst[4*x+2] = instance->info.lutB[l_current];
		
		l_current = (l_value >> 0) & 0x03;
		pDst[4*x+3] = instance->info.lutB[l_current];
	}
	
	if (l_width & 3U) {
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_current;
		int l_remaining = (int)l_width & 3U;
		
		l_current = (l_value >> 6) & 0x03;
		pDst[4*x+0] = instance->info.lutB[l_current];
		
		if (l_remaining > 1) {
			l_current = (l_value >> 4) & 0x03;
			pDst[4*x+1] = instance->info.lutB[l_current];
		}
		if (l_remaining > 2) {
			l_current = (l_value >> 2) & 0x03;
			pDst[4*x+2] = instance->info.lutB[l_current];
		}
	}
}

YABMP_IAPI(void, yabmp_pal4_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width / 2U; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		yabmp_uint8 l_value_lo = l_value & 0x0F;
		
		l_value >>= 4;
		
		pDst[2*x+0] = instance->info.lutB[l_value];
		
		pDst[2*x+1] = instance->info.lutB[l_value_lo];
	}
	
	if (l_width & 1U) {
		yabmp_uint8 l_value = pSrc[x] >> 4;
		
		pDst[2*x+0] = instance->info.lutB[l_value];
	}
}

YABMP_IAPI(void, yabmp_pal8_to_y8, (const yabmp* instance, const yabmp_uint8* pSrc, yabmp_uint8* pDst ))
{
	yabmp_uint32 x, l_width;
	
	assert(instance != NULL);
	assert(pSrc != NULL);
	assert(pDst != NULL);
	
	l_width      = (yabmp_uint32)instance->info.core.width;
	
	for(x = 0U; x < l_width; ++x)
	{
		yabmp_uint8 l_value = pSrc[x];
		
		pDst[x] = instance->info.lutB[l_value];
	}
}
