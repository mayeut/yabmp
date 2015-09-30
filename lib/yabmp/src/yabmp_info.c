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

YABMP_API(yabmp_status, yabmp_create_info, (yabmp* instance, yabmp_info ** info))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (*info != NULL)) {
		yabmp_send_error(instance, "NULL info or non-NULL *info.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	*info = yabmp_malloc(instance, sizeof(struct yabmp_info_struct));
	if (*info == NULL) {
		return YABMP_ERR_ALLOCATION;
	}
	yabmp_init_info(*info);
	return YABMP_OK;
}

YABMP_IAPI(void, yabmp_destroy_info, (yabmp* instance, yabmp_info** info))
{
	assert(instance != NULL);
	
	if ((info == NULL) || (*info == NULL)) {
		return;
	}
	
	yabmp_free(instance, *info);
	*info = NULL;
}

YABMP_IAPI(void, yabmp_init_info, (yabmp_info* info))
{
	assert(info != NULL);
	
	memset(info, 0, sizeof(struct yabmp_info_struct));
	
}

YABMP_API(yabmp_status, yabmp_get_dimensions, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* width, yabmp_uint32* height))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (width == NULL) || (height == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL width or NULL height.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	*width = info->width;
	*height = info->height;
	
	return YABMP_OK;
}
YABMP_API(yabmp_status, yabmp_get_pixels_per_meter, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* x, yabmp_uint32* y))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (x == NULL) || (y == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL x or NULL y.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	*x = info->res_ppm_x;
	*y = info->res_ppm_y;

	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_bpp, (const yabmp* instance, const yabmp_info* info, unsigned int* bpp))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (bpp == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL bpp.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*bpp = info->bpp;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_color_mask, (const yabmp* instance, const yabmp_info* info, unsigned int * color_mask))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (color_mask == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL color_mask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*color_mask = (info->flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_scan_direction, (const yabmp* instance, const yabmp_info* info, unsigned int* scan_direction))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (scan_direction == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL colorMask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*scan_direction = (info->flags >> YABMP_SCAN_SHIFT) & YABMP_SCAN_MASK;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_compression, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* compression))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (compression == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL compression.");
		return YABMP_ERR_INVALID_ARGS;
	}
	/* TODO, deal with OS/2 diff */
	switch (info->compression) {
		case YABMP_COMPRESSION_NONE:
		case YABMP_COMPRESSION_RLE4:
		case YABMP_COMPRESSION_RLE8:
			*compression = info->compression; /* initialized to 0 if only core header */
			break;
		default:
			yabmp_send_warning(instance, "Unsupported compression %" YABMP_PRIu32 ".", info->compression);
			*compression = info->compression; /* initialized to 0 if only core header */
			break;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_bitfields, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* blue_mask, yabmp_uint32* green_mask, yabmp_uint32 * red_mask, yabmp_uint32 * alpha_mask))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (blue_mask == NULL) || (green_mask == NULL) || (red_mask == NULL) || (alpha_mask == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL *_mask.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	if (((info->flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_BITFIELDS) == 0U) {
		yabmp_send_error(instance, "No bitfields.");
		return YABMP_ERR_UNKNOW;
	}
	
	*blue_mask = info->mask_blue;
	*green_mask = info->mask_green;
	*red_mask = info->mask_red;
	*alpha_mask = info->mask_alpha; /* 0 if not initialised */
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_bits, (const yabmp* instance, const yabmp_info* info, unsigned int* blue_bits, unsigned int* green_bits, unsigned int * red_bits, unsigned int * alpha_bits))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (blue_bits == NULL) || (green_bits == NULL) || (red_bits == NULL) || (alpha_bits == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL *_bits.");
		return YABMP_ERR_INVALID_ARGS;
	}
	*blue_bits  = info->bpc_blue;
	*green_bits = info->bpc_green;
	*red_bits   = info->bpc_red;
	*alpha_bits = info->bpc_alpha;
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_get_palette, (const yabmp* instance, const yabmp_info* info, unsigned int * color_count, yabmp_color const** palette))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((info == NULL) || (color_count == NULL) || (palette == NULL)) {
		yabmp_send_error(instance, "NULL info or NULL color_count or NULL palette.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	if (((info->flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_PALETTE) == 0U) {
		yabmp_send_error(instance, "No palette.");
		return YABMP_ERR_UNKNOW;
	}

	*color_count = info->num_palette;
	*palette     = info->palette;
	
	return YABMP_OK;
}
