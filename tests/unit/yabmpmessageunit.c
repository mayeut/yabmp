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
#include <yabmp_internal.h>
#include <yabmp_message.h>

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

int main(int argc, char* argv[])
{
	int result = EXIT_SUCCESS;
	
	yabmp l_instance = {0};
	
	l_instance.error_fn = print_error;
	l_instance.warning_fn = print_warning;
	
	/* test messages */
	{
		size_t test_size = 0U;
		unsigned int test_int = 0U;
		unsigned long test_long = 0U;
		yabmp_send_warning(&l_instance, "test yabmp_vsnprintf %%");
		yabmp_send_error(&l_instance, "test yabmp_vsnprintf %s", "string");
		yabmp_send_error(&l_instance, "test yabmp_vsnprintf %s", NULL);
		yabmp_send_error(&l_instance, "test yabmp_vsnprintf %zu", test_size);
		yabmp_send_error(&l_instance, "test yabmp_vsnprintf %u", test_int);
		yabmp_send_error(&l_instance, "test yabmp_vsnprintf %lu", test_long);
	}
	
	return result;
	
}
