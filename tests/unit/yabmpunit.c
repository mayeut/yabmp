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

static void print_error(void* context, const char* message)
{
	(void)context;
	fprintf(stderr, "ERROR: %s\n", message);
}

static void print_warning(void* context, const char* message)
{
	(void)context;
	fprintf(stderr, "WARNING: %s\n", message);
}

static void* custom_malloc(void* context, size_t size)
{
	(void)context;
	return malloc(size);
}
static void custom_free(void* context, void* ptr)
{
	(void)context;
	free(ptr);
}
static size_t custom_read(void* context, void * ptr, size_t size)
{
	(void)context;
	(void)ptr;
	(void)size;
	return 0U;
}
static yabmp_status custom_seek(void* context, yabmp_uint32 offset)
{
	(void)context;
	(void)offset;
	return YABMP_ERR_UNKNOW;
}
static void custom_close(void* context)
{
	(void)context;
}

int main(int argc, char* argv[])
{
	int result = EXIT_SUCCESS;
	
	/* test version */
	{
		yabmp_uint32 major  = 0U, minor  = 0U, patch  = 0U;
		yabmp_uint32 major2 = 1U, minor2 = 1U, patch2 = 1U;
		
		yabmp_get_version(NULL,    NULL,    NULL); /* check this doesn't crash */
		yabmp_get_version(&major,  &minor,  &patch);
		yabmp_get_version(&major2, NULL,    NULL);
		yabmp_get_version(NULL,    &minor2, NULL);
		yabmp_get_version(NULL,    NULL,    &patch2);
		
		result |= (((major^major2) | (minor^minor2) | (patch^patch2)) == 0U) ? EXIT_SUCCESS : EXIT_FAILURE;
	}
	
	/* test args error for yabmp_create_reader */
	{
		yabmp* l_reader = NULL;
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, custom_free) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;

		result |= (yabmp_create_reader(NULL, NULL, print_error, print_warning, NULL, NULL, custom_free) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;

		result |= (yabmp_create_reader(NULL, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;

		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, custom_free) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;

		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, custom_free) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;

		yabmp_destroy_reader(&l_reader, NULL);
	}
	
	/* test args error for yabmp_set_input_stream */
	{
		yabmp* l_reader = NULL;
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_set_input_stream(NULL, NULL, NULL, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, NULL, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, NULL, custom_seek, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, NULL, NULL, custom_close) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, custom_seek, custom_close) == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, NULL);
		
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_file(NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_file(l_reader, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_file(l_reader, "dummy/directory/that/does/not/exist/file.txt") == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, NULL);
		
	}
	
	/* test args error for yabmp_read_info */
	{
		yabmp* l_reader = NULL;
		yabmp_info* l_info = NULL;
		
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_create_info(NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_create_info(l_reader, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_create_info(NULL, &l_info) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		l_info = (yabmp_info*)l_reader;
		result |= (yabmp_create_info(l_reader, &l_info) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		l_info = NULL;
		result |= (yabmp_create_info(l_reader, &l_info) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		
		result |= (yabmp_read_info(NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_read_info(l_reader, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_read_info(NULL, l_info) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_read_info(l_reader, l_info) == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, &l_info);
	}
	
	/* test args error for yabmp_get_* (yabmp_info.c) */
	{
		yabmp* l_reader = NULL;
		yabmp_uint32 l_v0, l_v1, l_v2, l_v3;
		unsigned int l_v4, l_v5, l_v6, l_v7;
		yabmp_color const *l_lut;
		
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		/*
		TODO test before readinfo, shall fail
		result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_read_info(l_reader) == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		*/
		
		result |= (yabmp_get_dimensions(NULL, NULL, &l_v0, &l_v1) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_dimensions(l_reader, NULL, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_pixels_per_meter(NULL, NULL, &l_v0, &l_v1) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_pixels_per_meter(l_reader, NULL, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_bit_depth(NULL, NULL, &l_v4) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bit_depth(l_reader, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_color_type(NULL, NULL, &l_v4) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_color_type(l_reader, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_compression_type(NULL, NULL, &l_v0) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_compression_type(l_reader, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_scan_direction(NULL, NULL, &l_v4) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_scan_direction(l_reader, NULL, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_bitfields(NULL, NULL, &l_v0, &l_v1, &l_v2, &l_v3) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bitfields(l_reader, NULL, NULL, &l_v1, &l_v2, &l_v3) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bitfields(l_reader, NULL, &l_v0, NULL, &l_v2, &l_v3) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bitfields(l_reader, NULL, &l_v0, &l_v1, NULL, &l_v3) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bitfields(l_reader, NULL, &l_v0, &l_v1, &l_v2, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_bits(NULL, NULL, &l_v4, &l_v5, &l_v6, &l_v7) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bits(l_reader, NULL, NULL, &l_v5, &l_v6, &l_v7) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bits(l_reader, NULL, &l_v4, NULL, &l_v6, &l_v7) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bits(l_reader, NULL, &l_v4, &l_v5, NULL, &l_v7) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_bits(l_reader, NULL, &l_v4, &l_v5, &l_v6, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		result |= (yabmp_get_palette(NULL, NULL, &l_v4, &l_lut) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_palette(l_reader, NULL, NULL, &l_lut) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_get_palette(l_reader, NULL, &l_v4, NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, NULL);
	}
	
	/* test args error for yabmp_read_row */
	{
		yabmp* l_reader = NULL;
		void* l_row = (void*)1U;
		
		
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		/*
		 TODO test before readinfo, shall fail
		 result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		 result |= (yabmp_read_info(l_reader) == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		 */
		
		result |= (yabmp_read_row(NULL, l_row, 0U) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_read_row(l_reader, NULL, 0U) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, NULL);
	}
	
	/* test args error for yabmp_set_* transforms */
	{
		yabmp* l_reader = NULL;
		
		result |= (yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		/*
		 TODO test before readinfo, shall fail
		 result |= (yabmp_set_input_stream(l_reader, NULL, custom_read, NULL, NULL) == YABMP_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
		 result |= (yabmp_read_info(l_reader) == YABMP_ERR_UNKNOW) ? EXIT_SUCCESS : EXIT_FAILURE;
		 */
		
		result |= (yabmp_set_invert_scan_direction(NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_expand_to_bgrx(NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		result |= (yabmp_set_expand_to_grayscale(NULL) == YABMP_ERR_INVALID_ARGS) ? EXIT_SUCCESS : EXIT_FAILURE;
		
		yabmp_destroy_reader(&l_reader, NULL);
	}
	
	return result;
	
}
