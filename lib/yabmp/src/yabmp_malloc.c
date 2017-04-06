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

#define YABMP_MALLOC_NO_POISON
#include "../inc/private/yabmp_malloc.h"
#include "../inc/private/yabmp_message.h"
#include "../inc/private/yabmp_struct.h"

/* libc alloactors */
static void* L_yabmp_libc_malloc(void* context, size_t size)
{
	(void)context; /* unused argument */
	return malloc(size);
}

static void L_yabmp_libc_free(void* context, void* ptr)
{
	(void)context; /* unused argument */
	free(ptr);
}

YABMP_IAPI(void,  yabmp_setup_allocator, (
	yabmp* instance,
	void* alloc_context,
	yabmp_malloc_cb malloc_fn,
	yabmp_free_cb free_fn))
{
	assert(instance != NULL);

	if ((malloc_fn == NULL) || (free_fn == NULL)) {
		instance->alloc_context = NULL;
		instance->malloc_fn = L_yabmp_libc_malloc;
		instance->free_fn = L_yabmp_libc_free;
	}
	else {
		instance->alloc_context = alloc_context;
		instance->malloc_fn = malloc_fn;
		instance->free_fn = free_fn;
	}
}

YABMP_IAPI(void*, yabmp_malloc, (const yabmp* instance, size_t size))
{
	void* l_result = NULL;

	assert(instance != NULL);
	assert(instance->malloc_fn != NULL);

	if (size != 0U) {
		l_result = instance->malloc_fn(instance->alloc_context, size);
		if (l_result == NULL) {
			yabmp_send_error(instance, "Allocation of size %zu failed.", size);
		}
	}

	return l_result;
}

YABMP_IAPI(void,  yabmp_free,   (const yabmp* instance, void* ptr))
{
	assert(instance != NULL);
	assert(instance->free_fn != NULL);

	if (ptr != NULL) {
		instance->free_fn(instance->alloc_context, ptr);
	}
}
