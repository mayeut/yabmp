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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <yabmp.h>
#include <png.h>

#include "yabmpconvert.h"

static void PNGCBAPI print_png_error(png_structp png_struct, png_const_charp message)
{
	(void)png_struct;
	fprintf(stderr, "PNG ERROR: %s\n", message);
}
static void PNGCBAPI print_png_warning(png_structp png_struct, png_const_charp message)
{
	(void)png_struct;
	fprintf(stderr, "PNG WARNING: %s\n", message);
}

int convert_topng(const yabmpconvert_parameters* parameters, yabmp* bmp_reader)
{
	int result = 1; /* default is fail */
	void* l_buffer = NULL;
	size_t l_buffer_size;
	int32_t i;
	yabmp_uint32 l_width, l_height;
	unsigned int l_bpp;
	unsigned int l_color_mask;
	yabmp_uint32 l_compression;
	unsigned int l_scan_direction;
	yabmp_uint32 l_png_color_mask;
	int l_png_has_sBIT = 0;
	unsigned int blue_bits, green_bits, red_bits, alpha_bits;
		
	png_structp l_png_writer = NULL;
	png_infop l_png_info = NULL;
	FILE* l_output = NULL;
	int l_png_bit_depth = 8; /* default to 8 bits */
	
	assert(yabmp_get_dimensions(bmp_reader, &l_width, &l_height) == YABMP_OK);
	assert(yabmp_get_bpp(bmp_reader, &l_bpp) == YABMP_OK);
	assert(yabmp_get_color_mask(bmp_reader, &l_color_mask) == YABMP_OK);
	assert(yabmp_get_compression(bmp_reader, &l_compression) == YABMP_OK);
	assert(yabmp_get_scan_direction(bmp_reader, &l_scan_direction) == YABMP_OK);
	assert(yabmp_get_bits(bmp_reader, &blue_bits, &green_bits, &red_bits, &alpha_bits) == YABMP_OK);
	
	if ((blue_bits > 16U) || (green_bits > 16U) || (red_bits > 16U) || (alpha_bits > 16U)) {
		printf("ERROR: PNG does not support more than 16 bits per channel\n");
		return 1;
	}
	
	if ((blue_bits > 8U) || (green_bits > 8U) || (red_bits > 8U) || (alpha_bits > 8U)) {
		l_png_bit_depth = 16;
	}
	
	if (
		(blue_bits != l_png_bit_depth) ||
		(green_bits != l_png_bit_depth) ||
		(red_bits != l_png_bit_depth) ||
		((alpha_bits != l_png_bit_depth) && (alpha_bits != 0))) {
			l_png_has_sBIT = 1;
	}
	
	switch (l_color_mask)
	{
		case YABMP_COLOR_MASK_COLOR:
			yabmp_set_expand_to_bgrx(bmp_reader); /* always expand to BGR(A) */
			l_png_color_mask = PNG_COLOR_TYPE_RGB;
			break;
		case YABMP_COLOR_MASK_COLOR | YABMP_COLOR_MASK_ALPHA:
			yabmp_set_expand_to_bgrx(bmp_reader); /* always expand to BGR(A) */
			l_png_color_mask = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		case YABMP_COLOR_MASK_PALETTE | YABMP_COLOR_MASK_COLOR:
			if (parameters->expandPalette) {
				yabmp_set_expand_to_bgrx(bmp_reader); /* expand to BGR(A) */
				l_png_color_mask = PNG_COLOR_TYPE_RGB;
			} else {
				l_png_color_mask = PNG_COLOR_TYPE_PALETTE;
				l_png_bit_depth = l_bpp;
			}
			break;
		case YABMP_COLOR_MASK_PALETTE:
			if (parameters->expandPalette) {
				yabmp_set_expand_to_bgrx(bmp_reader); /* always expand to BGR(A) */
				l_png_color_mask = PNG_COLOR_TYPE_RGB;
			} else if (parameters->keepGrayPalette) {
				l_png_color_mask = PNG_COLOR_TYPE_PALETTE;
				l_png_bit_depth = l_bpp;
			} else {
				yabmp_set_expand_to_grayscale(bmp_reader); /* always expand to Y8 */
				l_png_color_mask = PNG_COLOR_TYPE_GRAY;
				l_png_bit_depth = 8;
			}
			break;
		default:
			printf("ERROR: Transcoding not supported.\n");
			goto FREE_INSTANCE;
	}
	switch (l_scan_direction)
	{
		case YABMP_SCAN_BOTTOM_UP:
			/* TODO check supported */
			if (yabmp_set_invert_scan_direction(bmp_reader) != YABMP_OK) {
				goto FREE_INSTANCE;
			}
			break;
		case YABMP_SCAN_TOP_DOWN:
			break;
		default:
			printf("ERROR: Unknown scan direction.\n");
			return 1;
	}
	l_output = fopen(parameters->outputFile, "wb");
	if (l_output == NULL) {
		printf("ERROR: can't open file %s for writing\n", parameters->outputFile);
		goto FREE_INSTANCE;
	}
		
	l_png_writer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, print_png_error, print_png_warning);
	if (setjmp(png_jmpbuf(l_png_writer))) {
		goto FREE_INSTANCE;
	}
	png_init_io(l_png_writer, l_output);
	l_png_info = png_create_info_struct(l_png_writer); /* errors here will generate long jump */
		
	png_set_IHDR(l_png_writer, l_png_info, l_width, l_height, l_png_bit_depth, l_png_color_mask, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	if (l_png_color_mask == PNG_COLOR_TYPE_PALETTE) {
		uint32_t i, l_num_palette;
		const uint8_t *blue_lut;
		const uint8_t *green_lut;
		const uint8_t *red_lut;
		const uint8_t *alpha_lut;
		png_color l_png_palette[256];
		
		assert(yabmp_get_palette(bmp_reader, &l_num_palette, &blue_lut, &green_lut, &red_lut, &alpha_lut) == YABMP_OK);
		if (l_num_palette > 256U) {
			printf("ERROR: palette with mor than 256 entries not supported.\n");
		}
		for (i = 0U; i < l_num_palette; ++i) {
			l_png_palette[i].blue  = blue_lut[i];
			l_png_palette[i].green = green_lut[i];
			l_png_palette[i].red   = red_lut[i];
		}
		
		png_set_PLTE(l_png_writer, l_png_info, l_png_palette, (int)l_num_palette);
	}
		
	if (l_png_has_sBIT) {
		png_color_8 l_sBIT;
		
		if ((l_png_color_mask & ~PNG_COLOR_MASK_ALPHA) == PNG_COLOR_TYPE_RGB) {
			l_sBIT.alpha = alpha_bits;
			l_sBIT.gray  = 0U;
			l_sBIT.blue  = blue_bits;
			l_sBIT.green = green_bits;
			l_sBIT.red   = red_bits;
			png_set_sBIT(l_png_writer, l_png_info, &l_sBIT);
			
			/* because of png_set_bgr */
			l_sBIT.blue  = red_bits;
			l_sBIT.red   = blue_bits;
			png_set_shift(l_png_writer, &l_sBIT);
		}
	}
	png_write_info(l_png_writer, l_png_info);
		
	switch (l_png_color_mask) {
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			png_set_bgr(l_png_writer);
			break;
		default:
			break;
	}
	if (l_png_bit_depth == 16) {
		png_set_swap(l_png_writer);
	}
	
	// Now deal with the image
	l_buffer_size = png_get_rowbytes(l_png_writer, l_png_info);
	l_buffer = malloc(l_buffer_size);
	if (l_buffer == NULL) {
		printf("ERROR: can't allocate buffer for 1 line\n");
		goto FREE_INSTANCE;
	}
	for (i = 0; i < l_height; ++i) {
		if (yabmp_read_row(bmp_reader, l_buffer, l_buffer_size) != YABMP_OK) {
			goto FREE_INSTANCE;
		}
		png_write_row(l_png_writer, l_buffer);
	}
	free(l_buffer);
	l_buffer = NULL;
		
	png_write_end(l_png_writer, NULL);
	result = 0;
FREE_INSTANCE:
	png_destroy_write_struct(&l_png_writer, &l_png_info);
	if (l_output != NULL) {
		fclose(l_output);
	}
	if (l_buffer != NULL) {
		free(l_buffer);
		l_buffer = NULL;
	}
	return result;
}

