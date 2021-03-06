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

#ifndef YABMPCONVERT_H
#define YABMPCONVERT_H

#include <yabmp.h>
#include <yabmpconvert_config.h>

typedef struct
{
	const char* input_file;
	const char* output_file;
	yabmp_malloc_cb malloc;
	yabmp_free_cb free;
	unsigned int version:1;
	unsigned int help:1;
	unsigned int quiet:1;
	unsigned int expand_palette:1;
	unsigned int keep_gray_palette:1;
	unsigned int no_seek_fn:1;
	
} yabmpconvert_parameters;

int convert_topng(const yabmpconvert_parameters* parameters, yabmp* bmp_reader, yabmp_info* bmp_info);

#endif /* YABMPCONVERT_H */
