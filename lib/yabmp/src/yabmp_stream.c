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

#include <stdio.h>

#include "../inc/private/yabmp_internal.h"

/* FILE stream helper */
static size_t yabmp_file_read (void* context, void * ptr, size_t size)
{
	FILE* l_file = (FILE*)context;
	
	assert(l_file != NULL);
	return fread(ptr, 1U, size, l_file);
}
YABMP_UNUSED
static size_t yabmp_file_write(void* context, const void * ptr, size_t size)
{
	FILE* l_file = (FILE*)context;
	
	assert(l_file != NULL);
	return fwrite(ptr, 1U, size, l_file);
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
static void yabmp_file_close(void* context)
{
	FILE* l_file = (FILE*)context;
	
	assert(l_file != NULL);
	fclose(l_file);
}

YABMP_API(yabmp_status, yabmp_set_input_file, (yabmp* reader, const char* path))
{
	FILE* l_file = NULL;
	yabmp_status l_status = YABMP_OK;
	
	YABMP_CHECK_READER(reader);

	if (path == NULL) {
		yabmp_send_error(reader, "\"path\" is NULL.");
		l_status = YABMP_ERR_INVALID_ARGS;
		goto BADEND;
	}
	
	l_file = fopen(path, "rb");
	if (l_file == NULL) {
		yabmp_send_error(reader, "Can't open file %s for reading.", path);
		l_status = YABMP_ERR_UNKNOW;
		goto BADEND;
	}
	
	l_status = yabmp_set_input_stream(reader, (void*)l_file, yabmp_file_read, yabmp_file_seek, yabmp_file_close);
	if (l_status == YABMP_OK) {
		l_file = NULL;
	}
BADEND:
	if (l_file!= NULL) {
		fclose(l_file);
	}
	
	return l_status;
}

YABMP_API(yabmp_status, yabmp_set_output_file, (yabmp* writer, const char* path))
{
	FILE* l_file = NULL;
	yabmp_status l_status = YABMP_OK;
	
	YABMP_CHECK_WRITER(writer);
	
	if (path == NULL) {
		yabmp_send_error(writer, "\"path\" is NULL.");
		l_status = YABMP_ERR_INVALID_ARGS;
		goto BADEND;
	}
	
	l_file = fopen(path, "wb");
	if (l_file == NULL) {
		yabmp_send_error(writer, "Can't open file %s for writing.", path);
		l_status = YABMP_ERR_UNKNOW;
		goto BADEND;
	}
	/*
	l_status = yabmp_set_output_stream(writer, (void*)l_file, yabmp_file_write, yabmp_file_seek, yabmp_file_close);
	if (l_status == YABMP_OK) {
		l_file = NULL;
	}*/
BADEND:
	if (l_file!= NULL) {
		fclose(l_file);
	}
	
	return l_status;
}

YABMP_IAPI(yabmp_status, yabmp_stream_read, (yabmp* reader, yabmp_uint8* buffer, size_t buffer_len))
{
	yabmp_status l_status = YABMP_OK;
	
	assert(reader != NULL);
	assert(reader->kind == YABMP_KIND_READER);
	
	if (reader->read_fn(reader->stream_context, buffer, buffer_len) != buffer_len) {
		yabmp_send_error(reader, "Failed to read %zu bytes.", buffer_len);
		l_status = YABMP_ERR_UNKNOW;
	} else {
		reader->stream_offset += (yabmp_uint32)buffer_len;
	}
	return l_status;
}

YABMP_IAPI(yabmp_status, yabmp_stream_seek, (yabmp* reader, yabmp_uint32 offset)) /* max offset is on yabmp_uint32 for BMP */
{
	yabmp_status l_status = YABMP_OK;
	
	assert(reader != NULL);
	assert(reader->kind == YABMP_KIND_READER);
	
	l_status = reader->seek_fn(reader->stream_context, offset);
	if (l_status == YABMP_OK) {
		reader->stream_offset = offset;
	} else {
		yabmp_send_error(reader, "Failed to seek to position %" YABMP_PRIu32 ".", offset);
	}
	return l_status;
}

YABMP_IAPI(yabmp_status, yabmp_stream_skip, (yabmp* instance, yabmp_uint32 count))
{
	yabmp_status l_status = YABMP_OK;
	yabmp_uint8 l_buffer[32];
	
	assert(instance != NULL);
	assert(instance->read_fn != NULL);
	
	if (count > 0U) {
		if (instance->seek_fn != NULL) {
			/* TODO overflow check */
			l_status = yabmp_stream_seek(instance, instance->stream_offset + count);
		}
		else {
			while (count > 0U) {
				size_t l_count = 32U;
				if (count < 32U) {
					l_count = (size_t)count;
				}
				if (instance->read_fn(instance->stream_context, l_buffer, l_count) != l_count) {
					l_status = YABMP_ERR_UNKNOW;
					goto BADEND;
				} else {
					instance->stream_offset += (yabmp_uint32)l_count;
					count -= (yabmp_uint32)l_count;
				}
			}
		}
	}
BADEND:
	return l_status;
}

/* Input memory stream */
typedef struct
{
	yabmp*             reader;
	const yabmp_uint8* data;
	const yabmp_uint8* current;
	size_t             data_size;
} yabmp_input_memory_stream;

static size_t yabmp_memory_read (void* context, void * ptr, size_t size)
{
	yabmp_input_memory_stream* l_context = (yabmp_input_memory_stream*)context;
	size_t l_remaining;
	
	assert(l_context != NULL);
	
	l_remaining = l_context->data_size - (size_t)(l_context->current - l_context->data);
	if (size > l_remaining) {
		size = l_remaining;
	}
	if (size) {
		memcpy(ptr, l_context->current, size);
		l_context->current += size;
	}
	return size;
}

static yabmp_status yabmp_memory_seek(void* context, yabmp_uint32 offset)
{
	yabmp_input_memory_stream* l_context = (yabmp_input_memory_stream*)context;
	
	assert(l_context != NULL);
	
	if ((size_t)offset > l_context->data_size) {
		return YABMP_ERR_UNKNOW;
	}
	l_context->current = l_context->data + offset;
	return YABMP_OK;
}
static void yabmp_memory_close(void* context)
{
	yabmp_input_memory_stream* l_context = (yabmp_input_memory_stream*)context;
	yabmp* l_reader = NULL;
	assert(l_context != NULL);
	l_reader = l_context->reader;
	yabmp_free(l_reader, l_context);
}

YABMP_API(yabmp_status, yabmp_set_input_memory, (yabmp* reader, const void* data, size_t data_size))
{
	yabmp_status l_status = YABMP_OK;
	yabmp_input_memory_stream* l_context = NULL;
	YABMP_CHECK_READER(reader);
	
	if (data == NULL) {
		yabmp_send_error(reader, "data is NULL");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	l_context = (yabmp_input_memory_stream*)yabmp_malloc(reader, sizeof(*l_context));
	if (l_context == NULL) {
		l_status = YABMP_ERR_ALLOCATION;
		goto BADEND;
	}
	
	l_context->reader = reader;
	l_context->data = data;
	l_context->current = data;
	l_context->data_size = data_size;
	
	l_status = yabmp_set_input_stream(reader, (void*)l_context, yabmp_memory_read, yabmp_memory_seek, yabmp_memory_close);
	if (l_status == YABMP_OK) {
		l_context = NULL;
	}
BADEND:
	if (l_context != NULL) {
		yabmp_free(reader, l_context);
	}
	return l_status;
}
