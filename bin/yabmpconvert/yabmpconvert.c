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
#include <optparse.h>

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

static const char* get_appname(const char* app)
{
	const char* l_firstResult = NULL;
	const char* l_secondResult = NULL;
	
	l_firstResult = strrchr(app, '/');
	l_secondResult = strrchr(app, '\\');
	
	if(l_firstResult == NULL) {
		l_firstResult = l_secondResult;
	}
	if ((ptrdiff_t)(l_secondResult - l_firstResult) > 0) {
		l_firstResult = l_secondResult;
	}
	if (l_firstResult == NULL) {
		return app;
	}
	return l_firstResult + 1;
}
static void print_usage(FILE* stream, const char* app)
{
	fprintf(
		stream,
		"usage:\n"
		"%s -h|--help : this help message\n"
		"%s -v|--version : print version\n"
		"%s [-vq] -i input -o output\n"
		"  -v, --version: print version before info\n"
		"  -q, --quiet:   no error/warning printed\n"
		"  -i, --input:   input filename\n"
		"  -o, --output:  output filename\n", app, app, app);
}

int main(int argc, char* argv[])
{
	static const struct optparse_long options[] = {
		{ "input",   'i', OPTPARSE_REQUIRED },
		{ "output",  'o', OPTPARSE_REQUIRED },
		{ "version", 'v', OPTPARSE_NONE },
		{ "help",    'h', OPTPARSE_NONE },
		{ "quiet",   'q', OPTPARSE_NONE },
		{ 0 }
	};
	
	int result = EXIT_SUCCESS;
	yabmpconvert_parameters params;
	struct optparse optparse;
	int option;
	
	memset(&params, 0, sizeof(params));
	optparse_init(&optparse, argv);
	
	while ((option = optparse_long(&optparse, options, NULL)) != -1) {
		switch (option) {
			case 'v':
				params.version = 1;
				break;
			case 'h':
				params.help = 1;
				break;
			case 'q':
				params.quiet = 1;
				break;
			case 'o':
				params.output_file = optparse.optarg;
				break;
			case 'i':
				params.input_file = optparse.optarg;
				break;
			case '?':
				fprintf(stderr, "%s: %s\n", get_appname(argv[0]), optparse.errmsg);
				print_usage(stderr, get_appname(argv[0]));
				result = 1;
				goto BADEND;
		}
	}
	
	if (params.version) {
		FILE* stream = stdout;
		
		if (!params.help && (params.output_file != NULL) && (strcmp(params.output_file, "-") == 0)) {
			stream = stderr;
		}
		fprintf(stream, "%s %s\n", get_appname(argv[0]), yabmp_get_version_string());
	}
	if (params.help) {
		print_usage(stdout, get_appname(argv[0]));
		goto BADEND;
	}
	
	if ((params.output_file == NULL) && (params.input_file == NULL) ) {
		if (params.version) {
			goto BADEND;
		}
	}
	
	{
		yabmp* l_bmp_reader = NULL;
		
		if (yabmp_create_reader(&l_bmp_reader, NULL, params.quiet ? NULL : print_yabmp_error, params.quiet ? NULL : print_yabmp_warning, NULL, NULL, NULL) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		if (yabmp_set_input_file(l_bmp_reader, params.input_file) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		if (yabmp_read_info(l_bmp_reader) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		/* yabmp_printinfo(stdout, l_bmp_reader, 0); */
		result = convert_topng(&params, l_bmp_reader);
FREE_INSTANCE:
		yabmp_destroy_reader(&l_bmp_reader);
	}
	
BADEND:
	return result;
}
