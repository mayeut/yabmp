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
static int yabmp_vsnprintf(char * buffer, int buffer_size, const char * format, va_list args ); /* vsnprintf is C99 */

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
	yabmp_vsnprintf(message, sizeof(message), format, args);
#if !defined(NDEBUG)
	assert((len >= 0) && (len < (int)YABMP_MAX_MSG_SIZE)); /* if longer message are seen during debug, update YABMP_MAX_MSG_SIZE */
#endif
	message_fn(context, message);
}

#define PRINT_UINT(type) \
type digits = 1U; \
while (value/digits >= 10U) { \
	digits *= 10U; \
} \
while ((digits != 0U) && (current < end)) { \
	int digit = (int)(value / digits); \
	value = value % digits; \
	digits /= 10U; \
	*current++ = (char)(digit + '0'); \
}

/* limited format support, no support for size only mode */
/* error checking is minimal since it'll be used from inside the library only */
/* this is not optimized at all */
static int yabmp_vsnprintf(char * buffer, int buffer_size, const char * format, va_list args )
{
	char* end = buffer + buffer_size;
	char* current = buffer;
	int c;
	
	assert(buffer != NULL);
	assert(buffer_size > 0);
	assert(format != NULL);
	
	while (((c = *format++) != '\0') && (current < end)) {
		if (c == '%') {
			c = *format++;
			switch (c) {
				case '%':
					*current++ = (char)c;
					break;
				case 's':
					{
						const char* ptr = va_arg(args, const char*);
						if (ptr == NULL) {
							ptr = "(null)";
						}
						while (((c = *ptr++) != '\0') && (current < end)) {
							*current++ = (char)c;
						}
					}
					break;
				case 'z':
					c = *format++;
					assert(c == 'u');
					{
						size_t value = va_arg(args, size_t);
						PRINT_UINT(size_t);
					}
					break;
				case 'l':
					c = *format++;
					assert(c == 'u');
					{
						unsigned long value = va_arg(args, unsigned long);
						PRINT_UINT(unsigned long);
					}
					break;
				case 'u':
					{
						unsigned int value = va_arg(args, unsigned int);
						PRINT_UINT(unsigned int);
					}
					break;
				default:
					assert(0);
					current = end;
					break;
			}
		} else {
			*current++ = (char)c;
		}
	}
	
	if (current == end) {
		current[-1] = '\0';
	} else {
		*current = '\0';
	}
	return (int)(current - buffer);
}