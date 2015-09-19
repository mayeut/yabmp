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

#include <stdarg.h>
#include <stdio.h>
#include "../inc/private/yabmp_message.h"
#include "../inc/private/yabmp_struct.h"

#define YABMP_MAX_MSG_SIZE 256U

static void yabmp_send_message(yabmp_message_cb message_fn, void* context, const char* format, va_list args);

YABMP_IAPI(void, yabmp_send_error,   (const yabmp* instance, const char* format, ...))
{
	assert(instance != NULL);
	assert(format != NULL);
	
	if (instance->error_fn != NULL) {
		va_list args;
		
		va_start(args, format);
		yabmp_send_message(instance->error_fn, instance->message_context, format, args);
		va_end(args);
	}
}
YABMP_IAPI(void, yabmp_send_warning, (const yabmp* instance, const char* format, ...))
{
	assert(instance != NULL);
	assert(format != NULL);
	
	if (instance->warning_fn != NULL) {
		va_list args;
		
		va_start(args, format);
		yabmp_send_message(instance->warning_fn, instance->message_context, format, args);
		va_end(args);
	}
}

static void yabmp_send_message(yabmp_message_cb message_fn, void* context, const char* format, va_list args)
{
	char message[YABMP_MAX_MSG_SIZE];
#if !defined(NDEBUG)
	int len =
#endif
	vsnprintf(message, sizeof(message), format, args); /* TODO this is not C90 */
#if !defined(NDEBUG)
	assert((len >= 0) && (len < (int)YABMP_MAX_MSG_SIZE)); /* if longer message are seen during debug, update YABMP_MAX_MSG_SIZE */
#endif
	message_fn(context, message);
}
