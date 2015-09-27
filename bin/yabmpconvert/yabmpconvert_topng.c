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

#if defined(PNGCBAPI)
#	define YABMP_PNGCBAPI PNGCBAPI
#else
#	define YABMP_PNGCBAPI PNGAPI
#endif

static void YABMP_PNGCBAPI print_png_error(png_structp png_struct, png_const_charp message)
{
	(void)png_struct;
	fprintf(stderr, "PNG ERROR: %s\n", message);
}
static void YABMP_PNGCBAPI print_png_warning(png_structp png_struct, png_const_charp message)
{
	(void)png_struct;
	fprintf(stderr, "PNG WARNING: %s\n", message);
}

int convert_topng(const yabmpconvert_parameters* parameters, yabmp* bmp_reader, yabmp_info* bmp_info)
{
	int result = EXIT_FAILURE; /* default is fail */
	void* l_buffer = NULL;
	size_t l_buffer_size;
	int32_t i;
	yabmp_uint32 l_width, l_height;
	yabmp_uint32 l_res_x, l_res_y;
	unsigned int l_bpp;
	unsigned int l_color_mask;
	yabmp_uint32 l_compression;
	unsigned int l_scan_direction;
	yabmp_uint32 l_png_color_mask;
	int l_png_has_sBIT = 0;
	unsigned int blue_bits, green_bits, red_bits, alpha_bits;
	int l_need_full_image = 0;
		
	png_structp l_png_writer = NULL;
	png_infop l_png_info = NULL;
	FILE* l_output = NULL;
	int l_png_bit_depth = 8; /* default to 8 bits */
	
	assert(parameters != NULL);
	assert(bmp_reader != NULL);
	assert(bmp_info != NULL);
	
	/* Those calls can't fail with proper arguments */
	(void)yabmp_get_dimensions(bmp_reader, bmp_info, &l_width, &l_height);
	(void)yabmp_get_pixels_per_meter(bmp_reader, bmp_info, &l_res_x, &l_res_y);
	(void)yabmp_get_bpp(bmp_reader, bmp_info, &l_bpp);
	(void)yabmp_get_color_mask(bmp_reader, bmp_info, &l_color_mask);
	(void)yabmp_get_compression(bmp_reader, bmp_info, &l_compression);
	(void)yabmp_get_scan_direction(bmp_reader, bmp_info, &l_scan_direction);
	(void)yabmp_get_bits(bmp_reader, bmp_info, &blue_bits, &green_bits, &red_bits, &alpha_bits);
	
	if ((blue_bits > 16U) || (green_bits > 16U) || (red_bits > 16U) || (alpha_bits > 16U)) {
		if (!parameters->quiet) {
			fprintf(stderr, "ERROR: PNG does not support more than 16 bits per channel\n");
		}
		return EXIT_FAILURE;
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
			if (parameters->expand_palette) {
				yabmp_set_expand_to_bgrx(bmp_reader); /* expand to BGR(A) */
				l_png_color_mask = PNG_COLOR_TYPE_RGB;
			} else {
				l_png_color_mask = PNG_COLOR_TYPE_PALETTE;
				l_png_bit_depth = l_bpp;
			}
			break;
		case YABMP_COLOR_MASK_PALETTE:
			if (parameters->expand_palette) {
				yabmp_set_expand_to_bgrx(bmp_reader); /* always expand to BGR(A) */
				l_png_color_mask = PNG_COLOR_TYPE_RGB;
			} else if (parameters->keep_gray_palette) {
				l_png_color_mask = PNG_COLOR_TYPE_PALETTE;
				l_png_bit_depth = l_bpp;
			} else {
				yabmp_set_expand_to_grayscale(bmp_reader); /* always expand to Y8 */
				l_png_color_mask = PNG_COLOR_TYPE_GRAY;
				l_png_bit_depth = 8;
			}
			break;
		default:
			if (!parameters->quiet) {
				fprintf(stderr, "ERROR: Transcoding not supported.\n");
			}
			return EXIT_FAILURE;
	}
	switch (l_scan_direction)
	{
		case YABMP_SCAN_BOTTOM_UP:
			switch (l_compression) {
				case YABMP_COMPRESSION_NONE:
				case YABMP_COMPRESSION_BITFIELDS:
					if (parameters->no_seek_fn) {
						/* no seek for stdin */
						l_need_full_image = 1;
					} else {
						if (yabmp_set_invert_scan_direction(bmp_reader) != YABMP_OK) {
							return EXIT_FAILURE;
						}
					}
					break;
				default:
					l_need_full_image = 1;
					break;
			}
			
			break;
		case YABMP_SCAN_TOP_DOWN:
			break;
		default:
			if (!parameters->quiet) {
				fprintf(stderr, "ERROR: Unknown scan direction.\n");
			}
			return EXIT_FAILURE;
	}
	if ((parameters->output_file[0] == '-') && (parameters->output_file[1] == '\0')) {
		l_output = stdout;
	}
	else {
		l_output = fopen(parameters->output_file, "wb");
		if (l_output == NULL) {
			if (!parameters->quiet) {
				fprintf(stderr, "ERROR: can't open file %s for writing\n", parameters->output_file);
			}
			return EXIT_FAILURE;
		}
	}
	
	l_png_writer = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, print_png_error, print_png_warning);
	if (l_png_writer == NULL) {
		if (!parameters->quiet) {
			fprintf(stderr, "ERROR: can't create PNG struct\n");
		}
		goto BADEND;
	}
	if (setjmp(png_jmpbuf(l_png_writer))) {
		goto BADEND;
	}
	png_init_io(l_png_writer, l_output);
	l_png_info = png_create_info_struct(l_png_writer); /* errors here will generate long jump */
		
	png_set_IHDR(l_png_writer, l_png_info, l_width, l_height, l_png_bit_depth, l_png_color_mask, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	if ((l_res_x != 0U) || (l_res_y != 0U)) {
		png_set_pHYs(l_png_writer, l_png_info, l_res_x, l_res_y, PNG_RESOLUTION_METER);
	}
	
	if (l_png_color_mask == PNG_COLOR_TYPE_PALETTE) {
		yabmp_uint32 i, l_num_palette;
		const yabmp_color *l_bmp_palette;
		png_color l_png_palette[256];
		
		if(yabmp_get_palette(bmp_reader, bmp_info, &l_num_palette, &l_bmp_palette) != YABMP_OK) {
			goto BADEND;
		}
		if (l_num_palette > 256U) {
			if (!parameters->quiet) {
				fprintf(stderr, "ERROR: palette with more than 256 entries not supported.\n");
			}
			goto BADEND;
		}
		for (i = 0U; i < l_num_palette; ++i) {
			l_png_palette[i].blue  = l_bmp_palette[i].blue;
			l_png_palette[i].green = l_bmp_palette[i].green;
			l_png_palette[i].red   = l_bmp_palette[i].red;
		}
		
		png_set_PLTE(l_png_writer, l_png_info, l_png_palette, (int)l_num_palette);
	}
		
	if (l_png_has_sBIT) {
		png_color_8 l_sBIT;
		
		if ((l_png_color_mask & ~PNG_COLOR_MASK_ALPHA) == PNG_COLOR_TYPE_RGB) {
			l_sBIT.alpha = alpha_bits;
			l_sBIT.gray  = 0U;
			l_sBIT.blue  = blue_bits ? blue_bits : 1; /* 0 is forbidden, transform deadlocks */
			l_sBIT.green = green_bits ? green_bits : 1;
			l_sBIT.red   = red_bits ? red_bits : 1;
			png_set_sBIT(l_png_writer, l_png_info, &l_sBIT);
			
			/* because of png_set_bgr */
			l_sBIT.blue  = red_bits ? red_bits : 1;
			l_sBIT.red   = blue_bits ? blue_bits : 1;
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
	
	/* Now deal with the image */
	l_buffer_size = png_get_rowbytes(l_png_writer, l_png_info);
	if (l_need_full_image) {
		/* TODO check overflow */
		l_buffer = malloc(l_buffer_size * (size_t)l_height);
	}
	else {
		l_buffer = malloc(l_buffer_size);
	}
	if (l_buffer == NULL) {
		if (!parameters->quiet) {
			fprintf(stderr, "ERROR: can't allocate buffer for 1 line\n");
		}
		goto BADEND;
	}
	if (l_need_full_image) {
		union
		{
			void* buffer;
			yabmp_uint8* buffer8u;
		} l_current_row;
		
		l_current_row.buffer = l_buffer;
		for (i = 0; i < l_height; ++i) {
			if (yabmp_read_row(bmp_reader, l_current_row.buffer, l_buffer_size) != YABMP_OK) {
				goto BADEND;
			}
			l_current_row.buffer8u += l_buffer_size;
		}
		for (i = 0; i < l_height; ++i) {
			l_current_row.buffer8u -= l_buffer_size;
			png_write_row(l_png_writer, l_current_row.buffer);
		}
	}
	else {
		for (i = 0; i < l_height; ++i) {
			if (yabmp_read_row(bmp_reader, l_buffer, l_buffer_size) != YABMP_OK) {
				goto BADEND;
			}
			png_write_row(l_png_writer, l_buffer);
		}
	}
	free(l_buffer);
	l_buffer = NULL;
		
	png_write_end(l_png_writer, NULL);
	result = 0;
BADEND:
	if (l_png_writer != NULL) {
		png_destroy_write_struct(&l_png_writer, &l_png_info);
	}
	if ((l_output != NULL) && (l_output != stdout)) {
		fclose(l_output);
		if (result != 0) {
			(void)remove(parameters->output_file);
		}
	}
	if (l_buffer != NULL) {
		free(l_buffer);
		l_buffer = NULL;
	}
	return result;
}

