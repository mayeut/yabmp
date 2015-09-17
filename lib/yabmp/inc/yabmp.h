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
#ifndef YABMP_H
#define YABMP_H

#include "yabmp_types.h"

/* C library */
#ifdef __cplusplus
	extern "C" {
#endif

#ifndef YABMP_API
#	define YABMP_API(return_type, symbol_name, arguments) return_type symbol_name arguments
#endif

#define YABMP_SCAN_BOTTOM_UP 0U
#define YABMP_SCAN_TOP_DOWN  1U
		
#define YABMP_COLOR_MASK_PALETTE 1U
#define YABMP_COLOR_MASK_COLOR   2U
#define YABMP_COLOR_MASK_ALPHA   4U
		
#define YABMP_COMPRESSION_NONE      0U
#define YABMP_COMPRESSION_RLE8      1U
#define YABMP_COMPRESSION_RLE4      2U
#define YABMP_COMPRESSION_BITFIELDS 3U
		
typedef enum yabmp_status_enum
{
	YABMP_OK = 0,
	YABMP_ERR_UNKNOW,
	YABMP_ERR_ALLOCATION,
	YABMP_ERR_INVALID_ARGS
} yabmp_status;
		
typedef struct yabmp_struct      yabmp;
typedef struct yabmp_info_struct yabmp_info;
		
typedef void  (*yabmp_message_cb)(void* context, const char* message);
typedef void* (*yabmp_malloc_cb)(void* context, size_t size);
typedef void  (*yabmp_free_cb)(void* context, void* ptr);
		
typedef size_t (*yabmp_stream_read_cb) (void* context, void * ptr, size_t size);
typedef size_t (*yabmp_stream_write_cb)(void* context, const void * ptr, size_t size);
typedef yabmp_status (*yabmp_stream_seek_cb) (void* context, yabmp_uint32 offset);
typedef void (*yabmp_stream_close_cb) (void* context);
		
YABMP_API(void, yabmp_get_version, (yabmp_uint32 * major, yabmp_uint32 * minor, yabmp_uint32 * patch));
YABMP_API(const char*, yabmp_get_version_string, ());
		
YABMP_API(yabmp_status, yabmp_create_reader, (
	yabmp** reader,
	void* message_context,
	yabmp_message_cb error_fn,
	yabmp_message_cb warning_fn,
	void* alloc_context,
	yabmp_malloc_cb malloc_fn,
	yabmp_free_cb free_fn
));
YABMP_API(void, yabmp_destroy_reader, (yabmp** reader));
		
YABMP_API(yabmp_status, yabmp_set_input_stream, (
	yabmp* reader,
	void* stream_context,
	yabmp_stream_read_cb  read_fn,
	yabmp_stream_seek_cb  seek_fn,
	yabmp_stream_close_cb close_fn
));
YABMP_API(yabmp_status, yabmp_set_input_file, (
	yabmp* reader,
	const char* path
));
		
YABMP_API(yabmp_status, yabmp_read_info, (yabmp* reader));
YABMP_API(yabmp_status, yabmp_read_row, (yabmp* reader, void* row, size_t row_size));
YABMP_API(yabmp_status, yabmp_get_dimensions, (const yabmp* instance, yabmp_uint32* width, yabmp_uint32* height));
YABMP_API(yabmp_status, yabmp_get_pixels_per_meter, (const yabmp* instance, yabmp_uint32* x, yabmp_uint32* y));
YABMP_API(yabmp_status, yabmp_get_bpp, (const yabmp* instance, unsigned int* bpp));
YABMP_API(yabmp_status, yabmp_get_color_mask, (const yabmp* instance, unsigned int* color_mask));
YABMP_API(yabmp_status, yabmp_get_compression, (const yabmp* instance, yabmp_uint32* compression));
YABMP_API(yabmp_status, yabmp_get_scan_direction, (const yabmp* instance, unsigned int* scan_direction));
YABMP_API(yabmp_status, yabmp_get_bitfields, (const yabmp* instance, yabmp_uint32* blue_mask, yabmp_uint32* green_mask, yabmp_uint32 * red_mask, yabmp_uint32 * alpha_mask));
YABMP_API(yabmp_status, yabmp_get_bits, (const yabmp* instance, unsigned int* blue_bits, unsigned int* green_bits, unsigned int * red_bits, unsigned int * alpha_bits));
YABMP_API(yabmp_status, yabmp_get_palette, (const yabmp* instance, yabmp_uint32 * count, yabmp_uint8 const** blue_lut, yabmp_uint8 const** green_lut, yabmp_uint8 const** red_lut, yabmp_uint8 const ** alpha_lut));
		
YABMP_API(yabmp_status, yabmp_set_invert_scan_direction, (yabmp* instance));
YABMP_API(yabmp_status, yabmp_set_expand_to_bgrx, (yabmp* instance));
YABMP_API(yabmp_status, yabmp_set_expand_to_grayscale, (yabmp* instance));
		
#ifdef __cplusplus
	}
#endif

#endif /* YABMP_H */
