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

/* Multiple inclusion protection */
#ifndef YABMP_STREAM_H
#define YABMP_STREAM_H

#include "yabmp_internal.h"

YABMP_IAPI(yabmp_status, yabmp_stream_read, (yabmp* instance, yabmp_uint8* buffer, size_t buffer_len));
YABMP_IAPI(yabmp_status, yabmp_stream_seek, (yabmp* reader, yabmp_uint32 offset)); /* max offset is on yabmp_uint32 for BMP */
YABMP_IAPI(yabmp_status, yabmp_stream_skip, (yabmp* instance, yabmp_uint32 count));

YABMP_UNUSED
static yabmp_uint16 yabmp_bswap16(yabmp_uint16 x)
{
#if defined(YABMP_HAVE_GCC_BYTESWAP_16)
	return __builtin_bswap16(x);
#else
	return (x >> 8) | (x << 8);
#endif
}

YABMP_UNUSED
static yabmp_uint32 yabmp_bswap32(yabmp_uint32 x)
{
#if defined(YABMP_HAVE_GCC_BYTESWAP_32)
	return __builtin_bswap32(x);
#else
	return
		((x & 0xFF000000U) >> 24) |
		((x & 0x00FF0000U) >>  8) |
		((x & 0x0000FF00U) <<  8) |
		((x & 0x000000FFU) << 24);
#endif
}

YABMP_UNUSED
static yabmp_status yabmp_stream_read_8u(yabmp* instance, yabmp_uint8* value)
{
	yabmp_status l_status = YABMP_OK;
	
	/* TODO memory interface ? */
	assert(instance != NULL);
	assert(value != NULL);
	assert(instance->read_fn != NULL);

	if (instance->read_fn(instance->stream_context, value, 1U) != 1U) {
		l_status = YABMP_ERR_UNKNOW;
	} else {
		instance->stream_offset += 1U;
	}
	return l_status;
}

YABMP_UNUSED
static yabmp_status yabmp_stream_read_le_16u(yabmp* instance, yabmp_uint16* value)
{
	yabmp_status l_status = YABMP_OK;
	
	/* TODO memory interface ? */
	assert(instance != NULL);
	assert(value != NULL);
	assert(instance->read_fn != NULL);
	/* if we're on a little endian machine, just read directly */
	if (instance->read_fn(instance->stream_context, value, 2U) != 2U) {
		l_status = YABMP_ERR_UNKNOW;
	} else {
		instance->stream_offset += 2U;
#if defined(YABMP_BIG_ENDIAN)
		*value = yabmp_bswap16(*value);
#endif
	}
	return l_status;
}

YABMP_UNUSED
static yabmp_status yabmp_stream_read_le_32u(yabmp* instance, yabmp_uint32* value)
{
	yabmp_status l_status = YABMP_OK;
	
	/* TODO memory interface ? */
	assert(instance != NULL);
	assert(value != NULL);
	assert(instance->read_fn != NULL);
	/* if we're on a little endian machine, just read directly */
	if (instance->read_fn(instance->stream_context, value, 4U) != 4U) {
		l_status = YABMP_ERR_UNKNOW;
	} else {
		instance->stream_offset += 4U;
#if defined(YABMP_BIG_ENDIAN)
		*value = yabmp_bswap32(*value);
#endif
	}
	return l_status;
}

#endif /* YABMP_STREAM_H */
