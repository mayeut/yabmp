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
#include "../common/yabmp_printinfo.h"

static void print_yabmp_error(void* context, const char* message)
{
	(void)context;
	fprintf(stderr, "YABMP ERROR: %s\n", message);
}

static void print_yabmp_warning(void* context, const char* message)
{
	(void)context;
	fprintf(stderr, "YABMP WARNING: %s\n", message);
}

int main(int argc, char* argv[])
{
	int result = 1;
	yabmpconvert_parameters params;
	
	memset(&params, 0, sizeof(params));
	
	params.inputFile = argv[1];
	params.outputFile = argv[2];
	params.expandPalette = 0;
	params.keepGrayPalette = 0;
	
	printf("yabmpconvert v%s\n", yabmp_get_version_string());
	
	{
		yabmp* l_bmp_reader;
		
		printf("\nProcessing file %s\n", params.inputFile);
		l_bmp_reader = NULL;
		
		if (yabmp_create_reader(&l_bmp_reader, NULL, print_yabmp_error, print_yabmp_warning, NULL, NULL, NULL) != YABMP_OK) {
			goto FREE_INSTANCE;
		}
		if (yabmp_set_input_file(l_bmp_reader, params.inputFile) != YABMP_OK) {
			goto FREE_INSTANCE;
		}
		if (yabmp_read_info(l_bmp_reader) != YABMP_OK) {
			goto FREE_INSTANCE;
		}
		yabmp_printinfo(stdout, l_bmp_reader, 0);
		result = convert_topng(&params, l_bmp_reader);
FREE_INSTANCE:
		yabmp_destroy_reader(&l_bmp_reader);
	}
	return result;
}

