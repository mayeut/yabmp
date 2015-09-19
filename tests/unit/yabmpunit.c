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

		yabmp_destroy_reader(&l_reader);
	}
	
	return result;
	
}
