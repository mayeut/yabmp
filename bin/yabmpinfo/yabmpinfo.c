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
#include <optparse.h>
#include "../common/yabmp_printinfo.h"

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
		"%s [-vqo output] file1 [file2 ...]\n"
		"  -v, --version: print version before info\n"
		"  -q, --quiet:   no error/warning printed\n"
		"  -o, --output:  output filename\n", app, app, app);
}

int main(int argc, char* argv[])
{
	static const struct optparse_long options[] = {
		{ "version", 'v', OPTPARSE_NONE },
		{ "help",    'h', OPTPARSE_NONE },
		{ "quiet",   'q', OPTPARSE_NONE },
		{ "output",  'o', OPTPARSE_REQUIRED },
		{ 0 }
	};
	int result = EXIT_SUCCESS;
	int has_multiple_files = 0;
	struct optparse optparse;
	int option;
	struct {
		unsigned int version:1;
		unsigned int help:1;
		unsigned int quiet:1;
	} flags = {0};
	const char* output = NULL;
	const char* input = NULL;
	const char* last_input = NULL;
	FILE* outStream = stdout;
	
	optparse_init(&optparse, argv);
	
	while ((option = optparse_long(&optparse, options, NULL)) != -1) {
		switch (option) {
			case 'v':
				flags.version = 1;
				break;
			case 'h':
				flags.help = 1;
				break;
			case 'q':
				flags.quiet = 1;
				break;
			case 'o':
				output = optparse.optarg;
				break;
			case '?':
				fprintf(stderr, "%s: %s\n", get_appname(argv[0]), optparse.errmsg);
				print_usage(stderr, get_appname(argv[0]));
				result = 1;
				goto BADEND;
		}
	}
	
	if (flags.help) {
		output = NULL;
	}
	
	if ((output != NULL) && (strcmp(output, "-") != 0)) {
		outStream = fopen(output, "wt");
		if (outStream == NULL) {
			if (!flags.quiet) {
				fprintf(stderr, "Can't open file %s for writing\n", output);
			}
			result = 1;
			goto BADEND;
		}
	}
	
	if (flags.version) {
		fprintf(outStream, "%s %s\n", get_appname(argv[0]), yabmp_get_version_string());
	}
	if (flags.help) {
		print_usage(stdout, get_appname(argv[0]));
		goto BADEND;
	}
	
	input = optparse_arg(&optparse);
	if (input == NULL) {
		if (!flags.version) {
			if (!flags.quiet) {
				fprintf(stderr, "%s: missing file1 argument\n", get_appname(argv[0]));
				print_usage(stderr, get_appname(argv[0]));
			}
			result = 1;
		}
		goto BADEND;
	}
	
	/* Check for multiple files */
	{
		struct optparse optparsecpy;
		memcpy(&optparsecpy, &optparse, sizeof(optparse));
		if (optparse_arg(&optparsecpy) != NULL) {
			has_multiple_files = 1;
		}
	}
	
	do
	{
		yabmp* l_reader;
		if (has_multiple_files) {
			if (last_input != NULL) {
				fputc('\n', outStream);
			}
			last_input = input;
			fprintf(outStream, "%s:\n", input);
		}
		l_reader = NULL;
		if (yabmp_create_reader(&l_reader, NULL, flags.quiet ? NULL : print_error, flags.quiet ? NULL : print_warning, NULL, NULL, NULL) != YABMP_OK) {
			result = 1;
			goto FREE_INSTANCE;
		}
		if (yabmp_set_input_file(l_reader, input) != YABMP_OK) {
			result = 1;
			goto FREE_INSTANCE;
		}
		if (yabmp_read_info(l_reader) != YABMP_OK) {
			result = 1;
			goto FREE_INSTANCE;
		}
		if (yabmp_printinfo(outStream, l_reader, flags.quiet) != 0) {
			result = 1;
		}
FREE_INSTANCE:
		yabmp_destroy_reader(&l_reader);
		if (result) {
			goto BADEND;
		}
	} while ((input = optparse_arg(&optparse)) != NULL);
BADEND:
	if ((outStream != stdout) && (outStream != NULL)) {
		fclose(outStream);
	}
	
	return result;
#if 0
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
	/* test args error for yabmp_create_reader */
	{
		yabmp* l_reader = NULL;
		yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, NULL);
		yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, NULL, custom_free);
		yabmp_create_reader(NULL, NULL, print_error, print_warning, NULL, NULL, custom_free);
		yabmp_create_reader(NULL, NULL, print_error, print_warning, NULL, NULL, NULL);
		yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, custom_free);
		yabmp_create_reader(&l_reader, NULL, print_error, print_warning, NULL, custom_malloc, custom_free);
		yabmp_destroy_reader(&l_reader, NULL);
	}
	
	return result;
#endif
}

