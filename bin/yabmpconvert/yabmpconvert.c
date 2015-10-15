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

#if defined(_MSC_VER)
#	include <io.h>
#	include <fcntl.h>
#endif

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

/* this is a mode to detect potential leak with correct tools, unchecked allocations, allocator misuse (unmatching malloc/free) */
static yabmp_uint32 allocation_max = 0U;
static yabmp_uint32 allocation_current = 0U;

static void* custom_malloc(void* context, size_t size)
{
	yabmp_uint8* buffer;
	(void)context;
	
	allocation_current++;
	if (allocation_current > allocation_max) {
		return NULL;
	}
	buffer = malloc(size+16);
	if (buffer != NULL) {
		buffer += 16;
	}
	return buffer;
}
static void custom_free(void* context, void* ptr)
{
	yabmp_uint8* buffer = ptr;
	(void)context;
	
	if (buffer != NULL) {
		buffer -= 16;
	}
	
	free(buffer);
}

static size_t yabmp_file_read (void* context, void * ptr, size_t size)
{
	FILE* l_file = (FILE*)context;
	
	assert(l_file != NULL);
	return fread(ptr, 1U, size, l_file);
}

static yabmp_status yabmp_file_seek(void* context, yabmp_uint32 offset)
{
	FILE* l_file = (FILE*)context;
	yabmp_status l_status = YABMP_OK;
	
	assert(l_file != NULL);
	
#if LONG_MAX < 0xFFFFFFFFU
	if (offset > LONG_MAX) {
		return YABMP_ERR_UNKNOW;
	}
#endif
	
	if (fseek(l_file, (long)offset, SEEK_SET) != 0) {
		l_status = YABMP_ERR_UNKNOW;
	}
	return l_status;
}

static const char* get_appname(const char* app)
{
	const char* l_firstResult = NULL;
	int l_offset = 1;
	
	l_firstResult = strrchr(app, '/');
	if (l_firstResult == NULL) {
		l_firstResult = app;
		l_offset = 0;
	}
#if defined(WIN32)
	{
		const char* l_secondResult = strrchr(l_firstResult, '\\');
		if(l_secondResult != NULL) {
			l_firstResult = l_secondResult;
			l_offset = 1;
		}
	}
#endif
	return l_firstResult + l_offset;
}

static void print_usage(FILE* stream, const char* app)
{
	fprintf(
		stream,
		"usage:\n"
		"%s -h|--help : this help message\n"
		"%s -v|--version : print version\n"
		"%s [-ekvq] [--no-seek] -i input -o output\n"
		"  -i, --input:           input filename\n"
		"  -o, --output:          output filename\n"
		"  -e, --expand-palette:  expand palette to RGB\n"
		"  -k, --keep-palette:    keep grayscale palette\n"
		"  -n, --no-seek:         no seek function when reading from stdin\n"
		"  -v, --version:         print version before info\n"
		"  -q, --quiet:           no error/warning printed\n"
		, app, app, app);
}

static void stream_setmode_binary(FILE* stream, unsigned int quiet)
{
#if defined(_MSC_VER)
	if (_setmode (fileno(stream), _O_BINARY) == -1) {
		if (!quiet) {
			fprintf(stderr, "Can't set stdin mode to binary\n");
		}
		exit(EXIT_FAILURE);
	}
#else
	(void)stream;
	(void)quiet;
#endif
}

int main(int argc, char* argv[])
{
	static const struct optparse_long options[] = {
		{ "input",          'i', OPTPARSE_REQUIRED },
		{ "output",         'o', OPTPARSE_REQUIRED },
		{ "expand-palette", 'e', OPTPARSE_NONE },
		{ "keep-palette",   'k', OPTPARSE_NONE },
		{ "no-seek",        'n', OPTPARSE_NONE },
		{ "version",        'v', OPTPARSE_NONE },
		{ "help",           'h', OPTPARSE_NONE },
		{ "quiet",          'q', OPTPARSE_NONE },
		{ 0 }
	};
	const char* use_custom_malloc = NULL;
	const char* use_memory_stream = NULL;
	int result = EXIT_SUCCESS;
	yabmpconvert_parameters params;
	struct optparse optparse;
	int option;
	void* input_data = NULL;
	size_t input_data_size = 0U;
	
	argv[0] = (char*)get_appname(argv[0]);
	
	/* env options */
	use_custom_malloc = getenv("YABMP_USE_CUSTOM_MALLOC");
	if (use_custom_malloc != NULL) {
		if ((use_custom_malloc[0] == '0') && (use_custom_malloc[1] == '\0')) {
			use_custom_malloc = NULL;
		}
	}
	use_memory_stream = getenv("YABMP_USE_MEMORY_STREAM");
	if (use_memory_stream != NULL) {
		if ((use_memory_stream[0] == '0') && (use_memory_stream[1] == '\0')) {
			use_memory_stream = NULL;
		}
	}
	
	memset(&params, 0, sizeof(params));
	if (use_custom_malloc != NULL) {
		params.malloc = custom_malloc;
		params.free = custom_free;
	}
	
	optparse_init(&optparse, argv);
	while ((option = optparse_long(&optparse, options, NULL)) != -1) {
		switch (option) {
			case 'i':
				params.input_file = optparse.optarg;
				break;
			case 'o':
				params.output_file = optparse.optarg;
				break;
			case 'e':
				params.expand_palette = 1;
				break;
			case 'k':
				params.keep_gray_palette = 1;
				break;
			case 'v':
				params.version = 1;
				break;
			case 'h':
				params.help = 1;
				break;
			case 'q':
				params.quiet = 1;
				break;
			case 'n':
				params.no_seek_fn = 1;
				break;
			case '?':
				fprintf(stderr, "%s: %s\n", argv[0], optparse.errmsg);
				print_usage(stderr, argv[0]);
				result = 1;
				goto BADEND;
		}
	}
	
	if (params.version) {
		FILE* stream = stdout;
		
		if (!params.help && (params.output_file != NULL) && (strcmp(params.output_file, "-") == 0)) {
			stream = stderr;
		}
		fprintf(stream, "%s %s\n", argv[0], yabmp_get_version_string());
	}
	if (params.help) {
		print_usage(stdout, argv[0]);
		goto BADEND;
	}
	
	if ((params.output_file == NULL) || (params.input_file == NULL) ) {
		if (params.version) {
			goto BADEND;
		}
		else {
			if (!params.quiet) {
				fprintf(stderr, "%s: missing input or output file\n", argv[0]);
				print_usage(stderr, argv[0]);
			}
			result = EXIT_FAILURE;
			goto BADEND;
		}
	}
	
	if ((use_custom_malloc != NULL) && !params.quiet) {
		fprintf(stderr, "Using custom allocation\n");
	}
	if ((use_memory_stream != NULL) && !params.quiet) {
		fprintf(stderr, "Using memory stream\n");
	}
	
	for (;;)
	{
		yabmp* l_bmp_reader = NULL;
		yabmp_info* l_bmp_info = NULL;
		
		if (use_memory_stream != NULL) {
			FILE* l_file = NULL;
			if ((params.input_file[0] == '-') && (params.input_file[1] == '\0')) {
				if (!params.quiet) {
					fprintf(stderr, "Can't use memory stream with stdin\n");
				}
				result = EXIT_FAILURE;
				goto BADEND;
			}
			l_file = fopen(params.input_file, "rb");
			if (l_file == NULL) {
				if (!params.quiet) {
					fprintf(stderr, "Can't open file '%s'\n", params.input_file);
				}
				result = EXIT_FAILURE;
				goto BADEND;
			}
			if (fseek(l_file, 0, SEEK_END) == 0)
			{
				long l_len = ftell(l_file);
				if (l_len > 0) {
					input_data_size = (size_t)l_len;
					rewind(l_file);
					if (use_custom_malloc != NULL) {
						input_data = custom_malloc(NULL, input_data_size);
					} else {
						input_data = malloc(input_data_size);
					}
					if (input_data != NULL) {
						if (fread(input_data, 1U, input_data_size, l_file) != input_data_size) {
							free(input_data);
							input_data = NULL;
						}
					}
				}
			}
			fclose(l_file);
			if (input_data == NULL) {
				if (!params.quiet) {
					fprintf(stderr, "Couldn't read file '%s'\n", params.input_file);
				}
				result = EXIT_FAILURE;
				goto FREE_INSTANCE;
			}
		}
		
		if (yabmp_create_reader(&l_bmp_reader, NULL, params.quiet ? NULL : print_yabmp_error, params.quiet ? NULL : print_yabmp_warning, NULL, (use_custom_malloc != NULL) ? custom_malloc : NULL, (use_custom_malloc != NULL) ? custom_free : NULL) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		if (yabmp_create_info(l_bmp_reader, &l_bmp_info) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		if (input_data != NULL) {
			if (yabmp_set_input_memory(l_bmp_reader, input_data, input_data_size) != YABMP_OK) {
				result = EXIT_FAILURE;
				goto FREE_INSTANCE;
			}
		}
		else if ((params.input_file[0] == '-') && (params.input_file[1] == '\0')) {
			stream_setmode_binary(stdin, params.quiet);
			/* This can't fail with proper arguments */
			(void)yabmp_set_input_stream(l_bmp_reader, stdin, yabmp_file_read, params.no_seek_fn ? NULL : yabmp_file_seek, NULL);
		} else {
			if (yabmp_set_input_file(l_bmp_reader, params.input_file) != YABMP_OK) {
				result = EXIT_FAILURE;
				goto FREE_INSTANCE;
			}
		}
		if (yabmp_read_info(l_bmp_reader, l_bmp_info) != YABMP_OK) {
			result = EXIT_FAILURE;
			goto FREE_INSTANCE;
		}
		/* yabmp_printinfo(stdout, l_bmp_reader, 0); */
		result = convert_topng(&params, l_bmp_reader, l_bmp_info);
FREE_INSTANCE:
		yabmp_destroy_reader(&l_bmp_reader, &l_bmp_info);
		if (input_data != NULL) {
			if (use_custom_malloc != NULL) {
				custom_free(NULL, input_data);
			} else {
				free(input_data);
			}
		}
		if ((use_custom_malloc != NULL) && (result != 0)) {
			if (allocation_current < allocation_max) {
				break;
			}
			allocation_max++;
			if (allocation_max == 0xFFFFFFFFU) {
				if (!params.quiet) {
					fprintf(stderr, "Reached maximum number of allocations\n");
					break;
				}
			}
			allocation_current = 0U;
			continue;
		}
		break;
	}
	
BADEND:
	return result;
}

