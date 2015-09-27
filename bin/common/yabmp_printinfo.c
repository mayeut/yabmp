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

#include <assert.h>
#include "yabmp_printinfo.h"

void yabmp_printinfo(FILE* outstream, yabmp* bmp_reader, const yabmp_info* info)
{
	yabmp_uint32 l_width, l_height, l_compression, l_res_x, l_res_y;
	unsigned int l_bpp, l_scan_direction, l_color_mask;
	unsigned int l_blue_bits, l_green_bits, l_red_bits, l_alpha_bits;
	
	assert(outstream != NULL);
	assert(bmp_reader != NULL);
	assert(info != NULL);

	/* Those calls can't fail with proper arguments */
	(void)yabmp_get_dimensions(bmp_reader, info, &l_width, &l_height);
	(void)yabmp_get_pixels_per_meter(bmp_reader, info, &l_res_x, &l_res_y);
	(void)yabmp_get_bpp(bmp_reader, info, &l_bpp);
	(void)yabmp_get_color_mask(bmp_reader, info, &l_color_mask);
	(void)yabmp_get_compression(bmp_reader, info, &l_compression);
	(void)yabmp_get_scan_direction(bmp_reader, info, &l_scan_direction);
	(void)yabmp_get_bits(bmp_reader, info, &l_blue_bits, &l_green_bits, &l_red_bits, &l_alpha_bits);
		
	fprintf(outstream, "Dimensions (WxH): %" YABMP_PRIu32 "x%" YABMP_PRIu32 "\n", l_width, l_height);
	if ((l_res_x != 0U) || (l_res_y != 0U)) {
		fprintf(outstream, "Pixels Per Meter (XxY): %" YABMP_PRIu32 "x%" YABMP_PRIu32 "\n", l_res_x, l_res_y);
	}
	fprintf(outstream, "Bits Per Pixel: %u\n", l_bpp);
	fprintf(outstream, "Bits Per Channel: B%u.G%u.R%u.A%u\n", l_blue_bits, l_green_bits, l_red_bits, l_alpha_bits);
	fputs("Color mask: ", outstream);
	if ((l_color_mask & YABMP_COLOR_MASK_PALETTE) != 0U) {
		fputs(" PALETTE", outstream);
	}
	if ((l_color_mask & YABMP_COLOR_MASK_COLOR) != 0U) {
		fputs(" COLOR", outstream);
	}
	if ((l_color_mask & YABMP_COLOR_MASK_ALPHA) != 0U) {
		fputs(" ALPHA", outstream);
	}
	fputc('\n', outstream);
		
	switch (l_compression) {
		case YABMP_COMPRESSION_NONE:
			fputs("Compression: NONE\n", outstream);
			break;
		case YABMP_COMPRESSION_RLE4:
			fputs("Compression: RLE4\n", outstream);
			break;
		case YABMP_COMPRESSION_RLE8:
			fputs("Compression: RLE8\n", outstream);
			break;
		case YABMP_COMPRESSION_BITFIELDS:
			{
				yabmp_uint32 blue_mask, green_mask, red_mask, alpha_mask;
				(void)yabmp_get_bitfields(bmp_reader, info, &blue_mask, &green_mask, &red_mask, &alpha_mask);
				fprintf(outstream, "Compression: BITFIELDS B:0x%08" YABMP_PRIX32 " G:0x%08" YABMP_PRIX32 " R:0x%08" YABMP_PRIX32 " A:0x%08" YABMP_PRIX32 "\n", blue_mask, green_mask, red_mask, alpha_mask);
			}
			break;
		default:
			fputs("Compression: UNKNOWN\n", outstream);
			break;
	}
	switch (l_scan_direction) {
		case YABMP_SCAN_BOTTOM_UP:
			fputs("Scan direction: BOTTOM-UP\n", outstream);
			break;
		case YABMP_SCAN_TOP_DOWN:
			fputs("Scan direction: TOP-DOWN\n", outstream);
			break;
		default:
			fputs("Scan direction: UNKNOWN\n", outstream);
			break;
	}
}
