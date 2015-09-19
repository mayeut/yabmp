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



#include "../inc/private/yabmp_internal.h"

static yabmp_status local_read_file_header(yabmp* reader, yabmp_info* info);
static yabmp_status local_read_core_info(yabmp* reader, yabmp_info* info);
static yabmp_status local_read_info(yabmp* reader, yabmp_info* info);
static yabmp_status local_rle4_decode_row(yabmp* reader, yabmp_uint8* row, int repack);
static yabmp_status local_rle8_decode_row(yabmp* reader, yabmp_uint8* row);

YABMP_API(yabmp_status, yabmp_create_reader, (
	yabmp** reader,
	void* message_context,
	yabmp_message_cb error_fn,
	yabmp_message_cb warning_fn,
	void* alloc_context,
	yabmp_malloc_cb malloc_fn,
	yabmp_free_cb free_fn))
{
	yabmp_status l_status = YABMP_OK;
	yabmp  l_interimInstance;
	yabmp* l_reader = NULL;
	
	memset(&l_interimInstance, 0, sizeof(l_interimInstance));
	
	l_interimInstance.kind = YABMP_KIND_READER;
	
	l_interimInstance.message_context = message_context;
	l_interimInstance.error_fn = error_fn;
	l_interimInstance.warning_fn = warning_fn;
	
	
	/* check reader valid */
	if ((reader == NULL) || (*reader != NULL)) {
		yabmp_send_error(&l_interimInstance, "Invalid arguments for yabmp_create_reader. \"reader\" is NULL or its content not NULL.");
		l_status = YABMP_ERR_INVALID_ARGS;
	}
	if (((malloc_fn == NULL) || (free_fn == NULL)) && ((malloc_fn != NULL) || (free_fn != NULL))) {
		yabmp_send_error(&l_interimInstance, "Invalid arguments for yabmp_create_reader. Either \"malloc_fn\" is NULL or \"free_fn\" is NULL while the other one is not NULL.");
		l_status = YABMP_ERR_INVALID_ARGS;
	}
	if (l_status != YABMP_OK) {
		return l_status;
	}

	l_interimInstance.alloc_context = alloc_context;
	l_interimInstance.malloc_fn = malloc_fn;
	l_interimInstance.free_fn = free_fn;
	
	l_reader = yabmp_malloc(&l_interimInstance, sizeof(*l_reader));
	if (l_reader == NULL) {
		yabmp_send_error(&l_interimInstance, "Can't allocate yabmp reader.");
		return YABMP_ERR_ALLOCATION;
	}
	memcpy(l_reader, &l_interimInstance, sizeof(l_interimInstance));
	
	yabmp_init_info(&(l_reader->info));
	*reader = l_reader;
	return l_status;
}

YABMP_API(void, yabmp_destroy_reader, (yabmp** reader))
{
	/* simple error checking */
	if ((reader == NULL) || (*reader == NULL)) {
		return;
	}
	
	/* now deal with reader */
	{
		yabmp  l_interimInstance;
		yabmp* l_reader = *reader;
		
		/* check reader */
		
		memset(&l_interimInstance, 0, sizeof(l_interimInstance));
		
		l_interimInstance.message_context = l_reader->message_context;
		l_interimInstance.error_fn = l_reader->error_fn;
		l_interimInstance.warning_fn = l_reader->warning_fn;
		
		l_interimInstance.alloc_context = l_reader->alloc_context;
		l_interimInstance.malloc_fn = l_reader->malloc_fn;
		l_interimInstance.free_fn = l_reader->free_fn;
		
		/* free content */
		if (l_reader->rle_row != NULL) {
			yabmp_free(l_reader, l_reader->rle_row);
		}
		
		if (l_reader->close_fn != NULL) {
			l_reader->close_fn(l_reader->stream_context);
		}
		
		/* free */
		l_reader->kind = YABMP_KIND_READER ^ YABMP_KIND_WRITER;
		yabmp_free(&l_interimInstance, l_reader);
		*reader = NULL;
	}
}

YABMP_API(yabmp_status, yabmp_set_input_stream, (
	yabmp* reader,
	void* stream_context,
	yabmp_stream_read_cb read_fn,
	yabmp_stream_seek_cb seek_fn,
	yabmp_stream_close_cb close_fn
))
{
	yabmp_status l_status = YABMP_OK;
	
	YABMP_CHECK_READER(reader);
	
	if (read_fn == NULL) {
		yabmp_send_error(reader, "NULL read function.");
		return YABMP_ERR_INVALID_ARGS;
	}
	if (seek_fn == NULL) {
		yabmp_send_warning(reader, "NULL seek function.");
	}
	
	if ((reader->status & YABMP_STATUS_HAS_STREAM) != 0U) {
		yabmp_send_error(reader, "Stream already set.");
		return YABMP_ERR_UNKNOW;
	}
	
	/* Shall we mark set ? */
	reader->stream_context = stream_context;
	reader->read_fn  = read_fn;
	reader->seek_fn  = seek_fn;
	reader->close_fn = close_fn;
	
	reader->status |= YABMP_STATUS_HAS_STREAM;
	
	return l_status;
}

YABMP_API(yabmp_status, yabmp_read_info, (yabmp* reader))
{
	yabmp_status l_status = YABMP_OK;
	yabmp_info*  l_info = NULL;
	
	YABMP_CHECK_READER(reader);
	
	l_info = &(reader->info);
	
	if ((reader->status & YABMP_STATUS_HAS_STREAM) == 0U) {
		yabmp_send_error(reader, "Stream not set.");
		l_status = YABMP_ERR_UNKNOW;
		goto BADEND;
	}
	
	/* read file header */
	l_status = local_read_file_header(reader, l_info);
	if (l_status != YABMP_OK) {
		goto BADEND;
	}
	
	switch (l_info->file.type) {
		case YABMP_FILE_TYPE('B', 'M'): /* BM */
			break;
		case YABMP_FILE_TYPE('B', 'A'): /* BA */
			yabmp_send_error(reader, "OS/2 struct bitmap array not supported.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
		case YABMP_FILE_TYPE('C', 'I'): /* CI */
			yabmp_send_error(reader, "OS/2 struct color icon not supported.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
		case YABMP_FILE_TYPE('C', 'P'): /* CP */
			yabmp_send_error(reader, "OS/2 const color pointer not supported.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
		case YABMP_FILE_TYPE('I', 'C'): /* IC */
			yabmp_send_error(reader, "OS/2 struct icon not supported.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
		case YABMP_FILE_TYPE('P', 'T'): /* PT */
			yabmp_send_error(reader, "OS/2 pointer.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
		default:
			yabmp_send_error(reader, "Unknown file type.");
			l_status = YABMP_ERR_UNKNOW;
			goto BADEND;
	}
	
	/* read core info */
	l_status = local_read_core_info(reader, l_info);
	if (l_status != YABMP_OK) {
		goto BADEND;
	}
	
	/* read info */
	l_status = local_read_info(reader, l_info);
	if (l_status != YABMP_OK) {
		goto BADEND;
	}
	
	{
		unsigned int l_dummy_shift;
		unsigned int l_blue_bits, l_green_bits, l_red_bits, l_alpha_bits;
		
		yabmp_bitfield_get_shift_and_bits(l_info->v2.blueMask,  &l_dummy_shift, &l_blue_bits);
		yabmp_bitfield_get_shift_and_bits(l_info->v2.greenMask, &l_dummy_shift, &l_green_bits);
		yabmp_bitfield_get_shift_and_bits(l_info->v2.redMask,   &l_dummy_shift, &l_red_bits);
		yabmp_bitfield_get_shift_and_bits(l_info->v3.alphaMask, &l_dummy_shift, &l_alpha_bits);
		
		l_info->expanded_bpp = 8U;
		
		if ((l_blue_bits > 16) || (l_green_bits > 16) || (l_red_bits > 16) || (l_alpha_bits > 16)) {
			l_info->expanded_bpp = 32U;
		}
		else if ((l_blue_bits > 8) || (l_green_bits > 8) || (l_red_bits > 8) || (l_alpha_bits > 8)) {
			l_info->expanded_bpp = 16U;
		}
		
	}
	reader->status |= YABMP_STATUS_HAS_INFO;
BADEND:
	return l_status;
}

static yabmp_status local_read_file_header(yabmp* reader, yabmp_info* info)
{
	yabmp_status l_status = YABMP_OK;
	
	assert(reader != NULL);
	assert(info != NULL);
	
	/* read type */
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &(info->file.type)));
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->file.fileSize)));
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &(info->file.reserved1)));
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &(info->file.reserved2)));
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->file.dataOffset)));
	
	return l_status;
}

static yabmp_status local_read_core_info(yabmp* reader, yabmp_info* info)
{
	yabmp_status l_status = YABMP_OK;
	yabmp_uint32 l_smallCore = 0U;
	
	assert(reader != NULL);
	assert(info != NULL);
	
	/* read core info */
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->core.size)));
	switch (info->core.size) {
		case 12U: /* only core */
			l_smallCore = 1U;
			break;
		default:
			break;
	}
	if (l_smallCore != 0U) {
		yabmp_uint16 l_value;
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_value));
		info->core.width = l_value;
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_value));
		info->core.height = l_value;
	} else {
		yabmp_uint32 l_height;
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->core.width))); /* we'll need to validate this */
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_height));
		
		/* TODO Top-Down */
		//if (l_height & 0x80000000U) { /* height is negative 2's complement */
		//	l_height = (l_height ^ 0xFFFFFFFFU) + 1U;
		//	/* TODO set top-down flag */
		//}
		
		info->core.height = l_height;
	}
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &(info->core.colorPlaneCount)));
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &(info->core.bpp)));
	
	return l_status;
}

static yabmp_status local_read_info(yabmp* reader, yabmp_info* info)
{
	yabmp_status l_status = YABMP_OK;
	int l_is_os2 = 0;
	
	assert(reader != NULL);
	assert(info != NULL);
	
	switch (info->core.size) {
		case 12U:  /* core only */
		case 40U:  /* info v1 */
		case 52U:  /* info v2 */
		case 56U:  /* info v3 */
		case 108U: /* info v4 */
		case 124U: /* info v5 */
			break;
		case 16U:
		case 64U:  /* OS22XBITMAPHEADER */
			l_is_os2 = 1;
			break;
		default:
			yabmp_send_error(reader, "Invalid Bitmap file (unknown header size : %" YABMP_PRIu32 ").", info->core.size);
			return YABMP_ERR_UNKNOW;
	}

	/* read v1 */
	if (info->core.size >= 40U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.compression)));
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.rawDataSize)));
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.resXppm)));
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.resYppm)));
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.pltColorCount)));
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v1.importantColorCount)));
	}
	
	if (l_is_os2) {
		if (info->core.size >= 64U)
		{
			/* let's just ignore for now... */
			YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, 24U));
		}
	}
	else {
		/* some invalid files out there... */
		if ((info->v1.compression == YABMP_COMPRESSION_BITFIELDS) && (info->core.size < 52U)) {
			if ((info->file.dataOffset > reader->stream_offset) && ((info->file.dataOffset - reader->stream_offset) >= 12U)) {
				yabmp_send_warning(reader, "Compression BMP bitfields found with v1 only header. Assuming V2 header.");
				info->core.size = 52U;
			}
		}
		
		/* read v2 */
		if (info->core.size >= 52U)
		{
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v2.redMask)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v2.greenMask)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v2.blueMask)));
		}
		/* read v2 */
		if (info->core.size >= 56U)
		{
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v3.alphaMask)));
		}
		
		if (info->core.size >= 108U)
		{
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v4.colorSpaceType)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, info->v4.colorSpaceEP, sizeof(info->v4.colorSpaceEP)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v4.redGamma)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v4.greenGamma)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v4.blueGamma)));
		}
		
		if (info->core.size >= 124U)
		{
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v5.intent)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v5.iccProfileData)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v5.iccProfileSize)));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(info->v5.reserved)));
		}
	}
	
	/* TODO Shall we read a LUT when bpp > 8 ? */
	if (info->core.bpp <= 8U) {
		const yabmp_uint32 l_maxColorCount = 1U << info->core.bpp;
		yabmp_uint32 l_colorCount = info->v1.pltColorCount;
		unsigned int l_isColorPalette = 0U;
		yabmp_uint32 i;
		
		if (l_colorCount == 0U) {
			/* Probably not valid, but such files have been seen in the wild. */
			if ((info->core.size == 12U) && (info->file.dataOffset > (reader->stream_offset + 2U)) && (info->file.dataOffset < (reader->stream_offset + 3U*l_maxColorCount))) {
				yabmp_send_warning(reader, "Data offset suggests wrong sized palette. Correcting palette size.");
				l_colorCount = (info->file.dataOffset - reader->stream_offset) / 3U;
			} else {
				l_colorCount = l_maxColorCount;
			}
		}
		else if (l_colorCount > l_maxColorCount) {
			yabmp_send_warning(reader, "Invalid palette found (%" YABMP_PRIu32 " entries). Ignoring some values.", l_colorCount);
			l_colorCount = l_maxColorCount;
		}
		
		if (info->core.size >= 40U) { /* palette entry is 4 bytes */
			for (i = 0U; i < l_colorCount; ++i) {
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutB[i])));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutG[i])));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutR[i])));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutA[i])));
				
				l_isColorPalette |= (info->lutB[i] ^ info->lutG[i]) | (info->lutG[i] ^ info->lutR[i]);
			}
		} else { /* palette entry is 3 bytes */
			for (i = 0U; i < l_colorCount; ++i) {
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutB[i])));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutG[i])));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(info->lutR[i])));
				
				l_isColorPalette |= (info->lutB[i] ^ info->lutG[i]) | (info->lutG[i] ^ info->lutR[i]);
			}
		}
		/* ignore values over 256U */
		for (; i < info->v1.pltColorCount; ++i) {
			yabmp_uint32 l_value;
			/* We can't be in BITMAPCOREHEADER case here */
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_value));
		}
		if (l_isColorPalette == 0U) {
			info->colorMask &= ~(yabmp_uint32)YABMP_COLOR_MASK_COLOR;
		}
		info->colorMask |= YABMP_COLOR_MASK_PALETTE;
	} else if (info->v1.pltColorCount > 0) {
		yabmp_send_warning(reader, "Ignoring palette in true color image.");
		/* We can't be in BITMAPCOREHEADER case here */
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, info->v1.pltColorCount * 4U));
	}
	if (info->v3.alphaMask != 0U) {
		info->colorMask |= YABMP_COLOR_MASK_ALPHA;
	}
	
	return l_status;
}

static yabmp_status local_rle4_decode_row(yabmp* instance, yabmp_uint8* row, int repack)
{
	yabmp_uint32 l_remaining = instance->info.core.width;
	yabmp_uint8* l_dst = row;
	yabmp_uint8  l_buffer[128];
	
	if (repack) {
		l_dst = instance->rle_row;
	}
	
	if (instance->rle_skip_y > 0) {
		memset(l_dst, 0, l_remaining);
		instance->rle_skip_y--;
		goto REPACK;
	}
	
	if (instance->rle_skip_x) {
		memset(l_dst, 0, instance->rle_skip_x);
		l_remaining -= instance->rle_skip_x;
		instance->rle_skip_x = 0U;
	}
	
	for (;;) {
		yabmp_uint8 l_values[2];
		unsigned int l_len;
		
		YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_values, sizeof(l_values)));
		l_len = l_values[0];
		if (l_len) { /* non escaped Encoded mode */
			yabmp_uint8  l_index = l_values[1];
			if (l_len > l_remaining) {
				/* we have a problem here ... */
				yabmp_send_error(instance, "The RLE4 stream seems corrupted");
				return YABMP_ERR_UNKNOW;
			}
			l_remaining -= l_len;
			/* write value */
			while (l_len > 1U) {
				*l_dst++ = l_index >> 4;
				*l_dst++ = l_index & 0xFU;
				l_len -= 2;
			}
			if (l_len) {
				*l_dst++ = l_index >> 4;
			}
		}
		else { /* escaped mode */
			unsigned int l_abs = l_values[1];
			
			if (l_abs == 0U) { /* end of line */
				memset(l_dst, 0, l_remaining);
				break;
			}
			else if (l_abs == 1U) { /* end of bitmap */
				memset(l_dst, 0, l_remaining);
				break;
			}
			else if (l_abs == 2U) { /* delta dx,dy */
				yabmp_uint8 l_delta[2];
				unsigned int l_count;
				
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_delta, sizeof(l_delta)));
				
				l_count = l_delta[0];
				if (l_count > l_remaining) {
					/* we have a problem here ... */
					yabmp_send_error(instance, "The RLE4 stream seems corrupted");
					return YABMP_ERR_UNKNOW;
				}
				if (l_delta[1] == 0U) {
					/* only dx */
					l_remaining -= l_count;
					while (l_count-- > 0U) {
						*l_dst++ = 0U;
					}
				}
				else {
					memset(l_dst, 0, l_remaining);
					instance->rle_skip_x = (instance->info.core.width - l_remaining) + l_count;
					instance->rle_skip_y = l_delta[1] - 1U;
					break;
				}
			}
			else /* absolute mode */
			{
				unsigned int i;
				if (l_abs > l_remaining) {
					/* we have a problem here ... */
					yabmp_send_error(instance, "The RLE4 stream seems corrupted");
					return YABMP_ERR_UNKNOW;
				}
				l_remaining -= l_abs;
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_buffer, (l_abs + 1U) / 2U ));
				if (((l_abs + 1U) / 2U) & 1U) { /* skip padding byte */
					yabmp_uint8 l_padding;
					YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(instance,  &l_padding));
				}
				
				for (i = 0U; i < l_abs / 2U; ++i) {
					*l_dst++ = l_buffer[i] >> 4;
					*l_dst++ = l_buffer[i] & 0xFU;
				}
				if (l_abs & 1U) {
					*l_dst++ = l_buffer[i] >> 4;
				}
				
			}
		}
	}
REPACK:
	if (repack) {
		yabmp_uint32 i;
		yabmp_uint32 l_len = instance->info.core.width;
		yabmp_uint8* l_src = instance->rle_row;
		
		for (i = 0U; i < l_len / 2U; ++i) {
			row[i] = (l_src[2*i] << 4) | (l_src[2*i+1]);
		}
		if (l_len & 1U) {
			row[i] = l_src[2*i] << 4;
		}
	}
	return YABMP_OK;
}

static yabmp_status local_rle8_decode_row(yabmp* instance, yabmp_uint8* row)
{
	yabmp_uint32 l_remaining = instance->info.core.width;
	
	if (instance->rle_skip_y > 0) {
		memset(row, 0, l_remaining);
		instance->rle_skip_y--;
		return YABMP_OK;
	}
	
	if (instance->rle_skip_x) {
		memset(row, 0, instance->rle_skip_x);
		l_remaining -= instance->rle_skip_x;
		instance->rle_skip_x = 0U;
	}
	
	for (;;) {
		yabmp_uint8 l_values[2];
		unsigned int l_len;
		
		YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_values, sizeof(l_values)));
		l_len = l_values[0];
		if (l_len) { /* non escaped Encoded mode */
			yabmp_uint8  l_index = l_values[1];
			if (l_len > l_remaining) {
				/* we have a problem here ... */
				yabmp_send_error(instance, "The RLE8 stream seems corrupted");
				return YABMP_ERR_UNKNOW;
			}
			l_remaining -= l_len;
			/* write value */
			while (l_len-- > 0U) {
				*row++ = l_index;
			}
		}
		else { /* escaped mode */
			unsigned int l_abs = l_values[1];
			
			if (l_abs == 0U) { /* end of line */
				memset(row, 0, l_remaining);
				break;
			}
			else if (l_abs == 1U) { /* end of bitmap */
				memset(row, 0, l_remaining);
				break;
			}
			else if (l_abs == 2U) { /* delta dx,dy */
				yabmp_uint8 l_delta[2];
				unsigned int l_count;
				
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_delta, sizeof(l_delta)));
				
				l_count = l_delta[0];
				if (l_count > l_remaining) {
					/* we have a problem here ... */
					yabmp_send_error(instance, "The RLE8 stream seems corrupted");
					return YABMP_ERR_UNKNOW;
				}
				if (l_delta[1] == 0U) {
					/* only dx */
					l_remaining -= l_count;
					while (l_count-- > 0U) {
						*row++ = 0U;
					}
				}
				else {
					memset(row, 0, l_remaining);
					instance->rle_skip_x = (instance->info.core.width - l_remaining) + l_count;
					instance->rle_skip_y = l_delta[1] - 1U;
					break;
				}
			}
			else /* absolute mode */
			{
				if (l_abs > l_remaining) {
					/* we have a problem here ... */
					yabmp_send_error(instance, "The RLE8 stream seems corrupted");
					return YABMP_ERR_UNKNOW;
				}
				l_remaining -= l_abs;
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, row, l_abs));
				row += l_abs;
				if (l_abs & 1U) { /* skip padding byte */
					yabmp_uint8 l_padding;
					YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(instance,  &l_padding));
				}
			}
		}
	}
	return YABMP_OK;
}

static yabmp_status local_get_row_bytes(yabmp* instance)
{
	yabmp_uint32 l_row_bytes = 0U;
	
	assert(instance != NULL);
	
	switch (instance->info.v1.compression) {
		case YABMP_COMPRESSION_NONE:
		case YABMP_COMPRESSION_RLE4:
		case YABMP_COMPRESSION_RLE8:
		case YABMP_COMPRESSION_BITFIELDS:
			break;
  default:
			instance->input_row_bytes  = 0U;
			instance->input_step_bytes = 0U;
			return YABMP_OK;
	}
	
	l_row_bytes = instance->info.core.width;
	
	/* TODO overflow check */
	l_row_bytes = ((l_row_bytes * (yabmp_uint32)instance->info.core.bpp) + 7U) & ~(yabmp_uint32)7U;
	l_row_bytes /= 8U;
	
	instance->input_row_bytes  = l_row_bytes;
	instance->transformed_row_bytes = l_row_bytes; /* no transforms */
	instance->input_step_bytes = (l_row_bytes + 3U) & ~(yabmp_uint32)3U;
	return YABMP_OK;
}

static yabmp_status local_setup_read(yabmp* reader)
{
	assert(reader != NULL);
	
	if (reader->info.file.dataOffset < reader->stream_offset) {
		yabmp_send_error(reader, "Invalid data offset.");
		return YABMP_ERR_UNKNOW;
	}
	if (reader->info.file.dataOffset > reader->stream_offset) {
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, reader->info.file.dataOffset - reader->stream_offset));
	}
	
	YABMP_SIMPLE_CHECK(local_get_row_bytes(reader));
	
	if (reader->info.v1.compression == YABMP_COMPRESSION_RLE4) {
		/* we might need an intermediate buffer */
		/* TODO check expand */
		/* TODO check overflow */
		reader->rle_row = yabmp_malloc(reader, reader->input_row_bytes * 2U);
		if (reader->rle_row == NULL) {
			return YABMP_ERR_ALLOCATION;
		}
	}
	
	if (reader->transforms & YABMP_TRANSFORM_SCAN_ORDER) {
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, reader->input_step_bytes * (yabmp_uint32)(reader->info.core.height - 1)));
	}
	
	if ((reader->transforms & (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) == (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) {
		yabmp_send_error(reader, "Can't apply YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE transforms.");
		return YABMP_ERR_UNKNOW;
	}
	
	/* First see if we must clear that (when format is already expanded) */
	if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
		if (reader->info.core.bpp == 24) {
			reader->transforms &= ~YABMP_TRANSFORM_EXPAND;
		}
	}
	
	/* TODO no LUT for 8bpp ?? */
	if (reader->transforms & (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) {
		reader->row8u = yabmp_malloc(reader, reader->input_step_bytes);
		if (reader->row8u == NULL) {
			return YABMP_ERR_ALLOCATION;
		}
		
		if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
			/* BGR or BGR(A) */
			/* TODO Check overflow */
			/* TODO 32bpp */
			yabmp_uint32 l_Bpc = reader->info.expanded_bpp / 8U;
			if (reader->info.colorMask & YABMP_COLOR_MASK_ALPHA) {
				reader->transformed_row_bytes = 4 * reader->info.core.width * l_Bpc;
			} else {
				reader->transformed_row_bytes = 3 * reader->info.core.width * l_Bpc;
			}
		}
		else if (reader->transforms & YABMP_TRANSFORM_GRAYSCALE) {
			reader->transformed_row_bytes = reader->info.core.width;
		}
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_read_row, (yabmp* reader, void* row, size_t row_size))
{	
	YABMP_CHECK_READER(reader);
	
	if (row == NULL) {
		yabmp_send_error(reader, "NULL info or NULL row.");
		return YABMP_ERR_INVALID_ARGS;
	}
	
	if ((reader->status & YABMP_STATUS_HAS_INFO) == 0U) {
		yabmp_send_error(reader, "Info not read yet.");
		return YABMP_ERR_UNKNOW;
	}
	
	if ((reader->status & YABMP_STATUS_HAS_LINES) == 0U) {
		/* setup reading */
		YABMP_SIMPLE_CHECK(local_setup_read(reader));
	}
	
	if (row_size < (size_t)reader->transformed_row_bytes) {
		yabmp_send_error(reader, "Invalid row size.");
		return YABMP_ERR_UNKNOW;
	}
	
	switch (reader->info.v1.compression) {
		case YABMP_COMPRESSION_RLE8:
			YABMP_SIMPLE_CHECK(local_rle8_decode_row(reader, row));
			break;
		case YABMP_COMPRESSION_RLE4:
			YABMP_SIMPLE_CHECK(local_rle4_decode_row(reader, row, 1));
			break;
  default:
			break;
	}
	
	if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
		YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, reader->row8u, reader->input_step_bytes));
		if (reader->info.core.bpp == 1U) {
			yabmp_pal1_to_bgr24(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 2U) {
			yabmp_pal2_to_bgr24(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 4U) {
			yabmp_pal4_to_bgr24(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 8U) {
			yabmp_pal8_to_bgr24(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 16U) {
			if ((reader->info.colorMask & YABMP_COLOR_MASK_ALPHA) != 0U) {
				if (reader->info.expanded_bpp == 8U) {
					yabmp_bf16u_to_bgra32(reader, reader->row16u, row);
				} else {
					yabmp_bf16u_to_bgra64(reader, reader->row16u, row);
				}
			} else {
				if (reader->info.expanded_bpp == 8U) {
					yabmp_bf16u_to_bgr24(reader, reader->row16u, row);
				} else {
					yabmp_bf16u_to_bgr48(reader, reader->row16u, row);
				}
			}
		} else if (reader->info.core.bpp == 32U) {
			if ((reader->info.colorMask & YABMP_COLOR_MASK_ALPHA) != 0U) {
				if (reader->info.expanded_bpp == 8U) {
					yabmp_bf32u_to_bgra32(reader, reader->row32u, row);
				} else {
					yabmp_bf32u_to_bgra64(reader, reader->row32u, row);
				}
				/* TODO yabmp_bf32u_to_bgra128 ??? */
			} else  {
				if (reader->info.expanded_bpp == 8U) {
					yabmp_bf32u_to_bgr24(reader, reader->row32u, row);
				} else {
					yabmp_bf32u_to_bgr48(reader, reader->row32u, row);
				}
				/* TODO yabmp_bf32u_to_bgra128 ??? */
			}
		}
	}
	else if (reader->transforms & YABMP_TRANSFORM_GRAYSCALE) {
		YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, reader->row8u, reader->input_step_bytes));
		if (reader->info.core.bpp == 1U) {
			yabmp_pal1_to_y8(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 2U) {
			yabmp_pal2_to_y8(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 4U) {
			yabmp_pal4_to_y8(reader, reader->row8u, row);
		}
		else if (reader->info.core.bpp == 8U) {
			yabmp_pal8_to_y8(reader, reader->row8u, row);
		}
	}
	else {
		switch (reader->info.v1.compression) {
			case YABMP_COMPRESSION_RLE4:
			case YABMP_COMPRESSION_RLE8:
				break;
			default:
				YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, row, reader->input_row_bytes));
				YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, (yabmp_uint32)(reader->input_step_bytes - reader->input_row_bytes)));
				break;
		}
	}
	
	if (reader->transforms & YABMP_TRANSFORM_SCAN_ORDER) {
		YABMP_SIMPLE_CHECK(yabmp_stream_seek(reader, reader->stream_offset - 2U * reader->input_step_bytes));
	}
	reader->status |= YABMP_STATUS_HAS_LINES;
	
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_set_invert_scan_direction, (yabmp* instance))
{
	yabmp_status l_status = YABMP_OK;
	
	YABMP_CHECK_INSTANCE(instance);
	
	switch (instance->info.v1.compression)
	{
		case YABMP_COMPRESSION_NONE:
		case YABMP_COMPRESSION_BITFIELDS: /* internal value */
			instance->transforms |= YABMP_TRANSFORM_SCAN_ORDER;
			break;
  default:
			yabmp_send_error(instance, "Scan direction change is only supported for YABMP_COMPRESSION_NONE & YABMP_COMPRESSION_BITFIELDS.");
			l_status = YABMP_ERR_UNKNOW;
			break;
	}
	
	return l_status;
}

YABMP_API(yabmp_status, yabmp_set_expand_to_bgrx, (yabmp* instance))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if (instance->info.expanded_bpp > 16U) {
		yabmp_send_error(instance, "yabmp_set_expand_to_bgrx is not valid for channel depth > 16.");
		return YABMP_ERR_UNKNOW;
	}
	instance->transforms |= YABMP_TRANSFORM_EXPAND;
	
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_set_expand_to_grayscale, (yabmp* instance))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if ((instance->info.colorMask & YABMP_COLOR_MASK_COLOR) != 0U) {
		yabmp_send_error(instance, "yabmp_set_grayscale is only valid for non-color palette image.");
		return YABMP_ERR_UNKNOW;
	}
	
	instance->transforms |= YABMP_TRANSFORM_GRAYSCALE;

	return YABMP_OK;
}