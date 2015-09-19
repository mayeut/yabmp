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
#ifndef YABMP_STRUCT_H
#define YABMP_STRUCT_H

#include "yabmp_api.h"
#include "yabmp_info.h"

#define YABMP_KIND_READER 0xF573BD17
#define YABMP_KIND_WRITER 0x9EAC6248

#define YABMP_STATUS_HAS_STREAM     1U
#define YABMP_STATUS_HAS_INFO       2U
#define YABMP_STATUS_HAS_VALID_INFO 4U
#define YABMP_STATUS_HAS_LINES      8U

#define YABMP_TRANSFORM_SCAN_ORDER 1U
#define YABMP_TRANSFORM_EXPAND     2U
#define YABMP_TRANSFORM_GRAYSCALE  4U

typedef void  (*yabmp_transform_fn)(const yabmp* instance, const void* pSrc, void* pDst );

struct yabmp_struct
{
	yabmp_uint32        kind; /* YABMP_KIND_READER or YABMP_KIND_WRITER */
	
	/* allocation management */
	void* alloc_context; /* context passed to allocation functions */
	yabmp_malloc_cb malloc_fn; /* user provided malloc function */
	yabmp_free_cb   free_fn;   /* user provided free function */
	
	/* error management */
	void* message_context; /* context passed to message functions */
	yabmp_message_cb error_fn;   /* user provided error message function */
	yabmp_message_cb warning_fn; /* user provided warning message function */
	
	/* stream management */
	void* stream_context; /* context passed to stream functions */
	yabmp_stream_read_cb  read_fn;  /* user provided stream read function */
	yabmp_stream_write_cb write_fn; /* user provided stream write function */
	yabmp_stream_seek_cb  seek_fn;  /* user provided stream seek function */
	yabmp_stream_close_cb close_fn; /* user provided stream close function */
	yabmp_uint32 stream_offset; /* current offset */
	
	yabmp_uint32 status; /* what have we done ? */
	yabmp_uint32 transforms; /* transformations that need to be done */
	
	yabmp_info info; /* info that get's read/written from/to bmp. User get's different info depending on transformation */ 
	
	yabmp_uint32 input_row_bytes;  /* input row size in bytes */
	yabmp_uint32 input_step_bytes; /* intput step size in bytes */
	
	yabmp_uint32 transformed_row_bytes; /* transformed row size in bytes */
	
	yabmp_transform_fn transform_fn;
	void*              input_row;
	
	yabmp_uint8* rle_row;
	yabmp_uint32 rle_skip_x;
	yabmp_uint32 rle_skip_y;
};

#endif /* YABMP_STRUCT_H */
