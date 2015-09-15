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

#include "../inc/private/yabmp_internal.h"

YABMP_IAPI(void, yabmp_init_info, (yabmp_info* info))
{
	assert(info != NULL);
	
	/* All set to 0 by default */
	
	/* set default values */
	info->colorMask = YABMP_COLOR_MASK_COLOR;
}

YABMP_API(yabmp_status, yabmp_get_dimensions, (const yabmp* instance, yabmp_uint32* width, yabmp_uint32* height))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if ((width == NULL) && (height == NULL)) {
		yabmp_send_error(instance, "NULL width and NULL height.");
		return YABMP_ERR_INVALID_ARGS;
	}
	if (width != NULL) {
		*width = info->core.width;
	} else {
		yabmp_send_warning(instance, "NULL width.");
	}
	if (height != NULL) {
		if (info->core.height < 0) {
			*height = (yabmp_uint32)-info->core.height;
		} else {
			*height = (yabmp_uint32)info->core.height;
		}
	} else {
		yabmp_send_warning(instance, "NULL height.");
	}
	return YABMP_OK;
}
YABMP_API(yabmp_status, yabmp_get_bpp, (const yabmp* instance, yabmp_uint16* bpp))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if (bpp == NULL) {
		yabmp_send_error(instance, "NULL info or NULL bpp.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*bpp = info->core.bpp;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_color_mask, (const yabmp* instance, yabmp_uint32* color_mask))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if (color_mask == NULL) {
		yabmp_send_error(instance, "NULL info or NULL color_mask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*color_mask = info->colorMask;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_scan_direction, (const yabmp* instance, yabmp_uint32* scan_direction))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if (scan_direction == NULL) {
		yabmp_send_error(instance, "NULL info or NULL colorMask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	if (info->core.height < 0) {
		*scan_direction = YABMP_SCAN_TOP_DOWN;
	} else {
		*scan_direction = YABMP_SCAN_BOTTOM_UP;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_compression, (const yabmp* instance, yabmp_uint32* compression))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if (compression == NULL) {
		yabmp_send_error(instance, "NULL info or NULL compression.");
		return YABMP_ERR_INVALID_ARGS;
	}
	/* TODO, deal with OS/2 diff */
	switch (info->v1.compression) {
		case YABMP_COMPRESSION_NONE:
		case YABMP_COMPRESSION_BITFIELDS: /* Bit fields */
		case YABMP_COMPRESSION_RLE4:
		case YABMP_COMPRESSION_RLE8:
			*compression = info->v1.compression; /* initialized to 0 if only core header */
			break;
		default:
			yabmp_send_warning(instance, "Unsupported compression %" YABMP_PRIu32 ".", info->v1.compression);
			*compression = info->v1.compression; /* initialized to 0 if only core header */
			break;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_bitfields, (const yabmp* instance, yabmp_uint32* blue_mask, yabmp_uint32* green_mask, yabmp_uint32 * red_mask, yabmp_uint32 * alpha_mask))
{
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if ((blue_mask == NULL) || (green_mask == NULL) || (red_mask == NULL) || (alpha_mask == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL *_mask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	/* TODO, deal with OS/2 diff */
	switch (info->v1.compression) {
		case YABMP_COMPRESSION_BITFIELDS: /* Bit fields */
			*blue_mask = info->v2.blueMask;
			*green_mask = info->v2.greenMask;
			*red_mask = info->v2.redMask;
			*alpha_mask = info->v3.alphaMask; /* 0 if not initialised */
			break;
		default:
			yabmp_send_error(instance, "Compression is not bitfields");
			return YABMP_ERR_UNKNOW;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_bits, (const yabmp* instance, unsigned int* blue_bits, unsigned int* green_bits, unsigned int * red_bits, unsigned int * alpha_bits))
{
	unsigned int l_dummy_shift;
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if ((blue_bits == NULL) || (green_bits == NULL) || (red_bits == NULL) || (alpha_bits == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL *_bits.");
		return YABMP_ERR_INVALID_ARGS;
	}
	/* TODO, deal with OS/2 diff */
	switch (info->v1.compression) {
		case YABMP_COMPRESSION_BITFIELDS: /* Bit fields */
			yabmp_bitfield_get_shift_and_bits(info->v2.blueMask,  &l_dummy_shift, blue_bits);
			yabmp_bitfield_get_shift_and_bits(info->v2.greenMask, &l_dummy_shift, green_bits);
			yabmp_bitfield_get_shift_and_bits(info->v2.redMask,   &l_dummy_shift, red_bits);
			yabmp_bitfield_get_shift_and_bits(info->v3.alphaMask, &l_dummy_shift, alpha_bits); /* 0 if not initialised */
			break;
		default:
			if (instance->info.core.bpp == 16) {
				*blue_bits = 5;
				*green_bits = 5;
				*red_bits = 5;
			} else {
				*blue_bits = 8;
				*green_bits = 8;
				*red_bits = 8;
			}
			yabmp_bitfield_get_shift_and_bits(info->v3.alphaMask, &l_dummy_shift, alpha_bits); /* 0 if not initialised */
			break;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_palette, (const yabmp* instance, yabmp_uint32 * count, yabmp_uint8 const** blue_lut, yabmp_uint8 const** green_lut, yabmp_uint8 const** red_lut, yabmp_uint8 const** alpha_lut))
{
	yabmp_uint32 l_count;
	const yabmp_info* info = NULL;
	
	YABMP_CHECK_INSTANCE(instance);
	
	info = &instance->info;
	
	if ((count == NULL) || (blue_lut == NULL) || (green_lut == NULL) || (red_lut == NULL) || (alpha_lut == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL count or NULL *_lut.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	if ((info->colorMask & YABMP_COLOR_MASK_PALETTE) == 0U) {
		yabmp_send_error(instance, "No palette.");
		return YABMP_ERR_UNKNOW;
	}
	
	l_count = info->v1.pltColorCount;
	if (l_count == 0U) {
		l_count = 1U << info->core.bpp;
	}
	else if (l_count > 256U) {
		l_count = 256U;
	}
	
	*count = l_count;
	*blue_lut  = info->lutB;
	*green_lut = info->lutG;
	*red_lut   = info->lutR;
	*alpha_lut = info->lutA;
	
	return YABMP_OK;
}
