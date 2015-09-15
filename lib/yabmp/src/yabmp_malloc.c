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

YABMP_IAPI(void*, yabmp_malloc, (const yabmp* instance, size_t size))
{
	void* l_result = NULL;
	
	assert(instance != NULL);
	
	if (size != 0U) {
		if (instance->free_fn != NULL) {
			l_result = instance->malloc_fn(instance->alloc_context, size);
		} else {
			l_result = malloc(size);
		}
		if (l_result == NULL) {
			/* TODO replace zu (C99) */
			yabmp_send_error(instance, "Allocation of size %zu failed.", size);
		}
	}
	
	return l_result;
}

YABMP_IAPI(void,  yabmp_free,   (const yabmp* instance, void* ptr))
{
	assert(instance != NULL);
	
	if (ptr != NULL)
	{
		if (instance->free_fn != NULL) {
			instance->free_fn(instance->alloc_context, ptr);
		} else {
			free(ptr);
		}
	}
}
