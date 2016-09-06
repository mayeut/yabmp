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

#define YABMP_LCS_CALIBRATED_RGB      0x00000000U
#define YABMP_LCS_sRGB                0x73524742U
#define YABMP_LCS_WINDOWS_COLOR_SPACE 0x57696E20U
#define YABMP_LCS_PROFILE_LINKED      0x4C494E4BU
#define YABMP_LCS_PROFILE_EMBEDDED    0x4D424544U

#define YABMP_LCS_GM_ABS_COLORIMETRIC 0x00000008U
#define YABMP_LCS_GM_BUSINESS         0x00000001U
#define YABMP_LCS_GM_GRAPHICS         0x00000002U
#define YABMP_LCS_GM_IMAGES           0x00000004U

static yabmp_status local_read_info_no_validation(yabmp* reader);
static yabmp_status local_valid_info(yabmp* reader);

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
	
	yabmp_init_info(&(l_reader->info2));
	yabmp_init_version(l_reader);
	*reader = l_reader;
	return l_status;
}

YABMP_API(void, yabmp_destroy_reader, (yabmp** reader, yabmp_info** info))
{
	/* simple error checking */
	if ((reader == NULL) || (*reader == NULL)) {
		return;
	}
	
	yabmp_destroy_info(*reader, info);
	
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
		yabmp_free(l_reader, l_reader->rle_row);
		yabmp_free(l_reader, l_reader->input_row);
		yabmp_free(l_reader, l_reader->info2.icc_profile);
		
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

YABMP_API(yabmp_status, yabmp_read_info, (yabmp* reader, yabmp_info* info))
{
	YABMP_CHECK_READER(reader);
	
	if (info == NULL) {
		yabmp_send_error(reader, "NULL info.");
		return YABMP_ERR_INVALID_ARGS;
	}
	YABMP_SIMPLE_CHECK(local_read_info_no_validation(reader));
	YABMP_SIMPLE_CHECK(local_valid_info(reader));
	
	/* Update BPC */
	if (((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_BITFIELDS) != 0U)
	{
		unsigned int l_dummy_shift, l_bits;
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_blue,  &l_dummy_shift, &l_bits);
		reader->info2.bpc_blue = l_bits;
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_green, &l_dummy_shift, &l_bits);
		reader->info2.bpc_green = l_bits;
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_red,   &l_dummy_shift, &l_bits);
		reader->info2.bpc_red = l_bits;
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_alpha, &l_dummy_shift, &l_bits);
		reader->info2.bpc_alpha = l_bits;
	}
	else {
		reader->info2.bpc_blue = 8;
		reader->info2.bpc_green = 8;
		reader->info2.bpc_red = 8;
		reader->info2.bpc_alpha = 0;
	}
	
	/* Update row bytes */
	{
		size_t l_row_bytes = reader->info2.width;
		l_row_bytes = ((l_row_bytes * (size_t)reader->info2.bpp) + 7U) & ~(size_t)7U;
		l_row_bytes /= 8U;
		reader->info2.rowbytes = l_row_bytes;
	}
	
	memcpy(info, &(reader->info2), sizeof(struct yabmp_info_struct));
	/* let's recreate icc profile */
	info->icc_profile = NULL;
	if (reader->info2.icc_profile != NULL) {
		info->icc_profile = yabmp_malloc(reader, reader->info2.icc_profile_size);
		if (info->icc_profile == NULL) {
			return YABMP_ERR_ALLOCATION;
		}
		memcpy(info->icc_profile, reader->info2.icc_profile, reader->info2.icc_profile_size);
	}
	
	return YABMP_OK;
}

static yabmp_status local_read_info_no_validation(yabmp* reader)
{
	yabmp_status   l_status = YABMP_OK;
	yabmp_uint16   l_data16u;
	yabmp_uint32   l_header_size;
	yabmp_uint32   l_bmpheader_offset;
	yabmp_uint32   l_palette_color_count = 0U;
	int l_is_os2 = 0;
	
	YABMP_CHECK_READER(reader);
	
	if ((reader->status & YABMP_STATUS_HAS_STREAM) == 0U) {
		yabmp_send_error(reader, "Stream not set.");
		return YABMP_ERR_UNKNOW;
	}
	if ((reader->status & YABMP_STATUS_HAS_INFO) == YABMP_STATUS_HAS_INFO) {
		yabmp_send_warning(reader, "Info already read.");
		return YABMP_OK;
	}
	
	reader->info2.flags = YABMP_COLOR_MASK_COLOR << YABMP_COLOR_SHIFT;
	
	/* read file header */
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u));
	switch (l_data16u) {
		case YABMP_FILE_TYPE('B', 'M'): /* BM */
			break;
		default:
			yabmp_send_error(reader, "Unknown file type.");
			return YABMP_ERR_UNKNOW;
	}
	YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, 4U)); /* skip file size */
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u)); /* reserved1 */
	if (l_data16u != 0U) {
		yabmp_send_warning(reader, "Found invalid reserved value in stream.");
	}
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u)); /* reserved2 */
	if (l_data16u != 0U) {
		yabmp_send_warning(reader, "Found invalid reserved value in stream.");
	}
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->data_offset)));
	l_bmpheader_offset = reader->stream_offset;
	
	/* read core info */
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_header_size)); /* header size */
	switch (l_header_size)
	{
		case 12U: /* only core */
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u)); /* width */
			reader->info2.width = l_data16u;
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u)); /* height */
			reader->info2.height = l_data16u;
			break;
		case 16U: /* OS/2 v2 minimum header */
		case 20U: /* OS/2 v2 Compression */
		case 24U: /* OS/2 v2 ImageDataSize */
		case 28U: /* OS/2 v2 XResolution */
		case 32U: /* OS/2 v2 YResolution */
		case 36U: /* OS/2 v2 ColorsUsed */
		case 42U: /* OS/2 v2 Units */
		case 44U: /* OS/2 v2 Reserved (Padding) */
		case 46U: /* OS/2 v2 Recording */
		case 48U: /* OS/2 v2 Rendering */
		case 60U: /* OS/2 v2 ColorEncoding */
		case 64U: /* OS/2 v2 full 64 bytes header */
			l_is_os2 = 1;
			/* fall through intended */
		case 40U: /* MS v3 + OS/2 v2 ColorsImportant */
		case 52U: /* Adobe RGB  masks + OS/2 Size1 */
		case 56U: /* Adobe RGBA masks + OS/2 Size2 */
		case 108U: /* MS v4 */
		case 124U: /* MS v5 */
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.width)));  /* width */
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.height))); /* height */
			break;
		default:
			yabmp_send_error(reader, "Invalid header size %u bytes.", (unsigned int)l_header_size);
			return YABMP_ERR_UNKNOW;
	}
	if (reader->info2.height & 0x80000000U) { /* height is negative 2's complement */
		reader->info2.height = (reader->info2.height ^ 0xFFFFFFFFU) + 1U;
		reader->info2.flags |= YABMP_SCAN_TOP_DOWN << YABMP_SCAN_SHIFT;
	}
	
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u)); /* color plane count */
	if (l_data16u != 1U) {
		yabmp_send_error(reader, "%u color plane(s) not supported.", (unsigned int)l_data16u);
		return YABMP_ERR_UNKNOW;
	}
	
	YABMP_SIMPLE_CHECK(yabmp_stream_read_le_16u(reader, &l_data16u));
	switch (l_data16u) {
		case 16U:
		case 32U:
			break;
		case 1U:
		case 2U:
		case 4U:
		case 8U:
		case 24U:
			if ((l_header_size == 52U) || (l_header_size == 56U)) {
				/* we're dealing with an OS/2 v2 bitmap, Adobe RGB(A) masks is 16/32 bpp which OS/2 doesn't support */
				l_is_os2 = 1;
			}
			break;
		default:
			yabmp_send_error(reader, "%ubpp not supported.", (unsigned int)l_data16u);
			return YABMP_ERR_UNKNOW;
	}
	reader->info2.bpp = (yabmp_uint8)l_data16u;
	
	/* read info */
	if (l_header_size >= 20U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.compression)));
	}
	if (l_header_size >= 24U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, 4U)); /* rawDataSize */
	}
	if (l_header_size >= 28U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.res_ppm_x)));
	}
	if (l_header_size >= 32U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.res_ppm_y)));
	}
	if (l_header_size >= 36U)
	{
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_palette_color_count));
	}
	if (l_header_size >= 40U)
	{
		yabmp_uint32 l_value;
		/* Ignore importantColorCount */
		YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_value));
	}
	
	if (l_is_os2) {
		if (l_header_size > 40U)
		{
			/* let's just ignore for now... */
			YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, l_header_size - 40U));
		}
	} else {
		/* Not OS/2 */
		if ((reader->info2.compression == 3U /* BI_BITFIELDS */) && (l_header_size < 52U)) {
			if ((reader->data_offset > reader->stream_offset) && ((reader->data_offset - reader->stream_offset) >= 12U)) {
				l_header_size = 52U;
			} else {
				yabmp_send_error(reader, "Compression BMP bitfields found but masks aren't present.");
				return YABMP_ERR_UNKNOW;
			}
		}
		if ((reader->info2.compression == 6U /* BI_ALPHABITFIELDS */) && (l_header_size == 40U)) {
			if ((reader->data_offset > reader->stream_offset) && ((reader->data_offset - reader->stream_offset) >= 16U)) {
				l_header_size = 56U;
				reader->info2.compression = 3U; /* BI_BITFIELDS */
			} else {
				yabmp_send_error(reader, "Compression BMP bitfields found but masks aren't present.");
				return YABMP_ERR_UNKNOW;
			}
		}
		
		/* read v2 */
		if (l_header_size >= 52U)
		{
			if (reader->info2.compression != 3U /* BI_BITFIELDS */) {
				YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, 3U * (yabmp_uint32)sizeof(yabmp_uint32)));
			}
			else {
				reader->info2.flags |= YABMP_COLOR_MASK_BITFIELDS << YABMP_COLOR_SHIFT;
				YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.mask_red)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.mask_green)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.mask_blue)));
			}
		}
		/* read v2 */
		if (l_header_size >= 56U)
		{
			if (reader->info2.compression != 3U /* BI_BITFIELDS */) {
				YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, 1U * (yabmp_uint32)sizeof(yabmp_uint32)));
			}
			else {
				YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &(reader->info2.mask_alpha)));
			}
		}
		
		if (l_header_size >= 108U)
		{
			yabmp_uint32 l_colorspace_type;
			
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_colorspace_type));
			
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_r.x));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_r.y));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_r.z));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_g.x));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_g.y));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_g.z));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_b.x));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_b.y));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.cie_b.z));
			
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.gamma_r));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.gamma_g));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &reader->info2.gamma_b));
			
			switch (l_colorspace_type) {
				case YABMP_LCS_CALIBRATED_RGB:
					reader->info2.cp_type = YABMP_COLOR_PROFILE_CALIBRATED_RGB;
					break;
				case YABMP_LCS_sRGB:
					reader->info2.cp_type = YABMP_COLOR_PROFILE_sRGB;
					break;
				case YABMP_LCS_WINDOWS_COLOR_SPACE: /* not sure what to use here... */
					reader->info2.cp_type = YABMP_COLOR_PROFILE_sRGB;
					break;
				case YABMP_LCS_PROFILE_EMBEDDED:
					reader->info2.cp_type = YABMP_COLOR_PROFILE_ICC_EMBEDDED;
					break;
				case YABMP_LCS_PROFILE_LINKED:
					reader->info2.cp_type = YABMP_COLOR_PROFILE_ICC_LINKED;
					break;
				default:
					/* ignore for now */
					break;
			}
			
		}
		
		if (l_header_size >= 124U)
		{
			yabmp_uint32 l_intent, l_data_offset, l_data_size, l_reserved;
			
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_intent));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_data_offset));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_data_size));
			YABMP_SIMPLE_CHECK(yabmp_stream_read_le_32u(reader, &l_reserved));
			
			switch (l_intent)
			{
				case YABMP_LCS_GM_ABS_COLORIMETRIC:
					reader->info2.cp_intent = YABMP_COLOR_PROFILE_INTENT_ABSCOL;
					break;
				case YABMP_LCS_GM_GRAPHICS:
					reader->info2.cp_intent = YABMP_COLOR_PROFILE_INTENT_RELCOL;
					break;
				case YABMP_LCS_GM_IMAGES:
					reader->info2.cp_intent = YABMP_COLOR_PROFILE_INTENT_PERCEPTUAL;
					break;
				case YABMP_LCS_GM_BUSINESS:
					reader->info2.cp_intent = YABMP_COLOR_PROFILE_INTENT_SATURATION;
					break;
				default:
					reader->info2.cp_intent = 255U; /* valid info shall fail */
					break;
			}
			
			switch (reader->info2.cp_type) {
				case YABMP_COLOR_PROFILE_ICC_EMBEDDED:
				case YABMP_COLOR_PROFILE_ICC_LINKED:
					if (reader->seek_fn != NULL) {
						yabmp_uint32 l_offset = reader->stream_offset;
						reader->info2.icc_profile_size = l_data_size;
						reader->info2.icc_profile = yabmp_malloc(reader, reader->info2.icc_profile_size);
						if (reader->info2.icc_profile == NULL) {
							return YABMP_ERR_ALLOCATION;
						}
						if ((YABMP_UINT32_MAX - l_bmpheader_offset) < l_data_offset) {
							yabmp_send_error(reader, "Would overflow.");
							return YABMP_ERR_UNKNOW;
						}
						YABMP_SIMPLE_CHECK(yabmp_stream_seek(reader, l_bmpheader_offset + l_data_offset));
						YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, reader->info2.icc_profile, reader->info2.icc_profile_size));
						YABMP_SIMPLE_CHECK(yabmp_stream_seek(reader, l_offset));
					}
					else {
						yabmp_send_warning(reader, "No seek function provided. ICC profile will be ignored.");
						reader->info2.cp_type = YABMP_COLOR_PROFILE_NONE;
					}
					break;
				default:
					break;
			}
		}
	}
	
	if (reader->info2.bpp <= 8U) {
		const yabmp_uint32 l_maxColorCount = 1U << reader->info2.bpp;
		yabmp_uint32 l_colorCount = l_palette_color_count;
		yabmp_uint32 l_total_color_count = l_colorCount;
		unsigned int l_isColorPalette = 0U;
		yabmp_uint32 i;
		
		if (l_colorCount == 0U) {
			/* Probably not valid, but such files have been seen in the wild. */
			if ((l_header_size == 12U) && (reader->data_offset > (reader->stream_offset + 2U)) && (reader->data_offset < (reader->stream_offset + 3U*l_maxColorCount))) {
				yabmp_send_warning(reader, "Data offset suggests wrong sized palette. Correcting palette size.");
				l_colorCount = (reader->data_offset - reader->stream_offset) / 3U;
			} else {
				l_colorCount = l_maxColorCount;
			}
			l_total_color_count = l_colorCount;
		}
		else if (l_colorCount > l_maxColorCount) {
			yabmp_send_warning(reader, "Invalid palette found (%" YABMP_PRIu32 " entries). Ignoring some values.", l_colorCount);
			l_colorCount = l_maxColorCount;
		}
		
		reader->info2.num_palette = (unsigned int)l_colorCount;
		if (l_header_size > 12U) { /* palette entry is 4 bytes */
			yabmp_color * l_palette = reader->info2.palette;
			for (i = 0U; i < l_colorCount; ++i) {
				yabmp_uint8 l_value;
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].blue)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].green)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].red)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &l_value));
				
				l_isColorPalette |= (l_palette[i].blue ^ l_palette[i].green) | (l_palette[i].green ^ l_palette[i].red);
			}
		} else { /* palette entry is 3 bytes */
			yabmp_color * l_palette = reader->info2.palette;
			for (i = 0U; i < l_colorCount; ++i) {
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].blue)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].green)));
				YABMP_SIMPLE_CHECK(yabmp_stream_read_8u(reader, &(l_palette[i].red)));
				
				l_isColorPalette |= (l_palette[i].blue ^ l_palette[i].green) | (l_palette[i].green ^ l_palette[i].red);
			}
		}
		/* Ignore values over l_colorCount */
		/* We can't be in BITMAPCOREHEADER case here, palette entry is 4 bytes */
		l_total_color_count -= l_colorCount;
		if (l_total_color_count > 0U) {
			if (l_total_color_count > (YABMP_UINT32_MAX / 4U)) {
				yabmp_send_error(reader, "Overflow detected.");
				return YABMP_ERR_UNKNOW;
			}
			YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, l_total_color_count * 4U));
		}
		if (l_isColorPalette == 0U) {
			reader->info2.flags &= ~(YABMP_COLOR_MASK_COLOR << YABMP_COLOR_SHIFT);
		}
		reader->info2.flags |= YABMP_COLOR_MASK_PALETTE << YABMP_COLOR_SHIFT;
	}
	else if (l_palette_color_count > 0) {
		yabmp_send_warning(reader, "Ignoring palette in true color image.");
		/* We can't be in BITMAPCOREHEADER case here, palette entry is 4 bytes */
		if (l_palette_color_count > (YABMP_UINT32_MAX / 4U)) {
			yabmp_send_error(reader, "Overflow detected.");
			return YABMP_ERR_UNKNOW;
		}
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, l_palette_color_count * 4U));
	}
	
	/* Update alpha mask */
	if (reader->info2.mask_alpha != 0U) {
		reader->info2.flags |= YABMP_COLOR_MASK_ALPHA << YABMP_COLOR_SHIFT;
	}
	
	/* Update 16/32bpp to BitFields */
	if (reader->info2.bpp == 32U) {
		if (reader->info2.compression == YABMP_COMPRESSION_NONE) {
			/* default color mask - no compression */
			reader->info2.flags |= YABMP_COLOR_MASK_BITFIELDS << YABMP_COLOR_SHIFT;
			reader->info2.mask_blue  = 0x000000FFU;
			reader->info2.mask_green = 0x0000FF00U;
			reader->info2.mask_red   = 0x00FF0000U;
			reader->info2.mask_alpha = 0x00000000U;
		}
		else if (reader->info2.compression == 3U /* BI_BITFIELDS */) {
			reader->info2.compression = YABMP_COMPRESSION_NONE;
		}
		
	} else if (reader->info2.bpp == 16U) {
		if (reader->info2.compression == YABMP_COMPRESSION_NONE) {
			/* default color mask - no compression */
			reader->info2.flags |= YABMP_COLOR_MASK_BITFIELDS << YABMP_COLOR_SHIFT;
			reader->info2.mask_blue  = 0x001FU;
			reader->info2.mask_green = 0x03E0U;
			reader->info2.mask_red   = 0x7C00U;
			reader->info2.mask_alpha = 0x0000U;
		}
		else if (reader->info2.compression == 3U /* BI_BITFIELDS */) {
			reader->info2.compression = YABMP_COMPRESSION_NONE;
		}
	}
	
	/* update internal values */
	{
		unsigned int l_dummy_shift;
		unsigned int l_blue_bits, l_green_bits, l_red_bits, l_alpha_bits;
		
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_blue,  &l_dummy_shift, &l_blue_bits);
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_green, &l_dummy_shift, &l_green_bits);
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_red,   &l_dummy_shift, &l_red_bits);
		yabmp_bitfield_get_shift_and_bits(reader->info2.mask_alpha, &l_dummy_shift, &l_alpha_bits);
		
		reader->info2.expanded_bps = 8U;
		if ((l_blue_bits > 16) || (l_green_bits > 16) || (l_red_bits > 16) || (l_alpha_bits > 16)) {
			reader->info2.expanded_bps = 32U;
		}
		else if ((l_blue_bits > 8) || (l_green_bits > 8) || (l_red_bits > 8) || (l_alpha_bits > 8)) {
			reader->info2.expanded_bps = 16U;
		}
	}
	
	reader->status |= YABMP_STATUS_HAS_INFO;
	return l_status;
}

static yabmp_status local_valid_info(yabmp* reader)
{
	YABMP_CHECK_READER(reader);
	
	if ((reader->status & YABMP_STATUS_HAS_INFO) == 0U) {
		yabmp_send_error(reader, "Info not read yet.");
		return YABMP_ERR_UNKNOW;
	}
	
	switch (reader->info2.compression) {
		case YABMP_COMPRESSION_NONE:
			if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_BITFIELDS) {
				switch (reader->info2.bpp) {
					case 16U:
					case 32U:
					{
						/* Check bitfield validity */
						unsigned int l_all_shift, l_blue_shift, l_green_shift, l_red_shift, l_alpha_shift;
						unsigned int l_all_bits,  l_blue_bits,  l_green_bits,  l_red_bits,  l_alpha_bits;
						
						yabmp_bitfield_get_shift_and_bits(reader->info2.mask_blue | reader->info2.mask_green | reader->info2.mask_red | reader->info2.mask_alpha, &l_all_shift, &l_all_bits);
						yabmp_bitfield_get_shift_and_bits(reader->info2.mask_blue,  &l_blue_shift, &l_blue_bits);
						yabmp_bitfield_get_shift_and_bits(reader->info2.mask_green, &l_green_shift, &l_green_bits);
						yabmp_bitfield_get_shift_and_bits(reader->info2.mask_red,   &l_red_shift, &l_red_bits);
						yabmp_bitfield_get_shift_and_bits(reader->info2.mask_alpha, &l_alpha_shift, &l_alpha_bits);
						
						/* sum of bits must be == to bit count of ORed mask (no intersection) */
						if ((l_blue_bits + l_green_bits + l_red_bits + l_alpha_bits) != l_all_bits) {
							yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
							return YABMP_ERR_UNKNOW;
						}
						/* sum of bits must be <= to bpp */
						if (l_all_bits > reader->info2.bpp) {
							yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
							return YABMP_ERR_UNKNOW;
						}
						/* each mask set bits must be contiguous */
						/* check (mask >> bits) + 1 is a power of 2 */
						if (reader->info2.mask_blue != 0U) {
							yabmp_uint32 l_mask = reader->info2.mask_blue >> l_blue_shift;
							if ((l_mask & (l_mask+1U)) != 0U) {
								yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
								return YABMP_ERR_UNKNOW;
							}
						}
						if (reader->info2.mask_green != 0U) {
							yabmp_uint32 l_mask = reader->info2.mask_green >> l_green_shift;
							if ((l_mask & (l_mask+1U)) != 0U) {
								yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
								return YABMP_ERR_UNKNOW;
							}
						}
						if (reader->info2.mask_red != 0U) {
							yabmp_uint32 l_mask = reader->info2.mask_red >> l_red_shift;
							if ((l_mask & (l_mask+1U)) != 0U) {
								yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
								return YABMP_ERR_UNKNOW;
							}
						}
						if (reader->info2.mask_alpha != 0U) {
							yabmp_uint32 l_mask = reader->info2.mask_alpha >> l_alpha_shift;
							if ((l_mask & (l_mask+1U)) != 0U) {
								yabmp_send_error(reader, "Invalid masks found for BitFields compression.");
								return YABMP_ERR_UNKNOW;
							}
						}
					}
						break;
					default:
						yabmp_send_error(reader, "%ubpp not supported for BitFields.", (unsigned int)reader->info2.bpp);
						return YABMP_ERR_UNKNOW;
				}
			}
			else {
				switch (reader->info2.bpp) {
					case 1U:
					case 2U:
					case 4U:
					case 8U:
					case 24U:
						break;
					case 16:
					case 32:
						yabmp_send_error(reader, "%ubpp not supported for Non BitFields.", (unsigned int)reader->info2.bpp);
						return YABMP_ERR_UNKNOW;
					default:
						yabmp_send_error(reader, "%ubpp not supported.", (unsigned int)reader->info2.bpp);
						return YABMP_ERR_UNKNOW;
				}
			}
			break;
		case YABMP_COMPRESSION_RLE4:
			if (reader->info2.bpp != 4U) {
				yabmp_send_error(reader, "%ubpp not supported for RLE4 compression.", (unsigned int)reader->info2.bpp);
				return YABMP_ERR_UNKNOW;
			}
			break;
		case YABMP_COMPRESSION_RLE8:
			if (reader->info2.bpp != 8U) {
				yabmp_send_error(reader, "%ubpp not supported for RLE8 compression.", (unsigned int)reader->info2.bpp);
				return YABMP_ERR_UNKNOW;
			}
			break;
		default:
			yabmp_send_error(reader, "compression %" YABMP_PRIu32 " not supported.", reader->info2.compression);
			return YABMP_ERR_UNKNOW;
	}
	
	if (reader->info2.cp_intent == 255U) {
		yabmp_send_error(reader, "Unknown color profile intent.");
		return YABMP_ERR_UNKNOW;

	}
	switch (reader->info2.cp_type) {
		case YABMP_COLOR_PROFILE_ICC_EMBEDDED:
		case YABMP_COLOR_PROFILE_ICC_LINKED:
			if (reader->info2.icc_profile == NULL) {
				yabmp_send_error(reader, "No ICC profile read (probably ICC specified with v4 BMP header).");
				return YABMP_ERR_UNKNOW;
			}
			break;
		default:
			break;
	}
	
	reader->status |= YABMP_STATUS_HAS_VALID_INFO;
	return YABMP_OK;
}

static yabmp_status local_rle4_decode_row(yabmp* instance, yabmp_uint8* row, int repack)
{
	yabmp_uint32 l_remaining = instance->info2.width;
	yabmp_uint8* l_dst = row;
	
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
		l_dst += instance->rle_skip_x;
		instance->rle_skip_x = 0U;
	}
	
	for (;;) /* process one line at a time */
	{
		yabmp_uint8  l_values[2];
		unsigned int l_len;
		YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_values, sizeof(l_values)));
		l_len = l_values[0];
		if (l_len) { /* non escaped Encoded mode */
			yabmp_uint8  l_index = l_values[1];
			if (l_len > l_remaining) {
				/* limit to remaining */
				l_len = (unsigned int)l_remaining;
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
				instance->rle_skip_y = UINT_MAX;
				break;
			}
			else if (l_abs == 2U) { /* delta dx,dy */
				yabmp_uint8 l_delta[2];
				unsigned int l_count;
				
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_delta, sizeof(l_delta)));
				
				l_count = l_delta[0];
				if (l_count > l_remaining) {
					/* limit to remaining */
					l_count = (unsigned int)l_remaining;
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
					instance->rle_skip_x = (instance->info2.width - l_remaining) + l_count;
					instance->rle_skip_y = l_delta[1] - 1U;
					break;
				}
			}
			else /* absolute mode */
			{
				yabmp_uint8  l_buffer[128];
				unsigned int i;
				if (l_abs > l_remaining) {
					/* limit to remaining */
					l_abs = (unsigned int)l_remaining;
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
		yabmp_uint32 l_len = instance->info2.width;
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
	yabmp_uint32 l_remaining = instance->info2.width;
	
	if (instance->rle_skip_y > 0) {
		memset(row, 0, l_remaining);
		instance->rle_skip_y--;
		return YABMP_OK;
	}
	
	if (instance->rle_skip_x) {
		memset(row, 0, instance->rle_skip_x);
		l_remaining -= instance->rle_skip_x;
		row += instance->rle_skip_x;
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
				/* limit to remaining */
				l_len = (unsigned int)l_remaining;
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
				instance->rle_skip_y = UINT_MAX;
				break;
			}
			else if (l_abs == 2U) { /* delta dx,dy */
				yabmp_uint8 l_delta[2];
				unsigned int l_count;
				
				YABMP_SIMPLE_CHECK(yabmp_stream_read(instance, l_delta, sizeof(l_delta)));
				
				l_count = l_delta[0];
				if (l_count > l_remaining) {
					/* limit to remaining */
					l_count = (unsigned int)l_remaining;
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
					instance->rle_skip_x = (instance->info2.width - l_remaining) + l_count;
					instance->rle_skip_y = l_delta[1] - 1U;
					break;
				}
			}
			else /* absolute mode */
			{
				if (l_abs > l_remaining) {
					/* limit to remaining */
					l_abs = (unsigned int)l_remaining;
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

static yabmp_status local_setup_read(yabmp* reader)
{
	yabmp_uint32 l_rle4_factor = 1U;
	assert(reader != NULL);
	
	if (reader->rle_row != NULL) {
		yabmp_free(reader, reader->rle_row);
		reader->rle_row = NULL;
	}
	if (reader->input_row != NULL) {
		yabmp_free(reader, reader->input_row);
		reader->input_row = NULL;
	}
	
	if (reader->data_offset < reader->stream_offset) {
		yabmp_send_error(reader, "Invalid data offset.");
		return YABMP_ERR_UNKNOW;
	}
	if (reader->data_offset > reader->stream_offset) {
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, reader->data_offset - reader->stream_offset));
	}
	
	switch (reader->info2.compression) {
		case YABMP_COMPRESSION_RLE4:
			l_rle4_factor = 2U;
			break;
		case YABMP_COMPRESSION_NONE:
		case YABMP_COMPRESSION_RLE8:
		default:
			break;
	}
	
	if ((sizeof(size_t) > sizeof(yabmp_uint32)) && (reader->info2.rowbytes > 0xFFFFFFFFU)) {
		yabmp_send_error(reader, "Would overflow.");
		return YABMP_ERR_UNKNOW;
	}
	reader->input_row_bytes  = (yabmp_uint32)reader->info2.rowbytes;
	reader->transformed_row_bytes = reader->input_row_bytes; /* no transforms */
	
	if (reader->input_row_bytes & ((l_rle4_factor - 1U) << 31U)) { /* l_rle4_factor is 1 or 2 */
		yabmp_send_error(reader, "Would overflow.");
		return YABMP_ERR_UNKNOW;
	}
	reader->input_step_bytes = reader->input_row_bytes * l_rle4_factor;
	if (reader->input_step_bytes > (0xFFFFFFFFU - 3U)) {
		yabmp_send_error(reader, "Would overflow.");
		return YABMP_ERR_UNKNOW;
	}
	reader->input_step_bytes = (reader->input_step_bytes + 3U) & ~(yabmp_uint32)3U;
	
	if (reader->transforms & YABMP_TRANSFORM_SCAN_ORDER) {
		YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, reader->input_step_bytes * (yabmp_uint32)(reader->info2.height - 1U)));
	}
	
	if ((reader->transforms & (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) == (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) {
		yabmp_send_error(reader, "Can't apply YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE transforms.");
		return YABMP_ERR_UNKNOW;
	}
	
	/* First see if we must clear that (when format is already expanded) */
	if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
		if (reader->info2.bpp == 24U) {
			reader->transforms &= ~YABMP_TRANSFORM_EXPAND;
		}
	}
	
	/* TODO no LUT for 8bpp ?? */
	if (reader->transforms & (YABMP_TRANSFORM_EXPAND | YABMP_TRANSFORM_GRAYSCALE)) {
		reader->input_row = yabmp_malloc(reader, reader->input_step_bytes);
		if (reader->input_row == NULL) {
			return YABMP_ERR_ALLOCATION;
		}
		
		if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
			/* BGR or BGR(A) */
			/* TODO 32bpp */
			yabmp_uint32 l_Bpc = reader->info2.expanded_bps / 8U;
			if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_ALPHA) {
				l_Bpc *= 4U;
			} else {
				l_Bpc *= 3U;
			}
			if (reader->info2.width > (0xFFFFFFFFU / l_Bpc)) {
				yabmp_send_error(reader, "Would overflow.");
				return YABMP_ERR_UNKNOW;
			}
			reader->transformed_row_bytes = reader->info2.width * l_Bpc;
		}
		else if (reader->transforms & YABMP_TRANSFORM_GRAYSCALE) {
			reader->transformed_row_bytes = reader->info2.width;
		}
	}
	else if (reader->info2.compression == YABMP_COMPRESSION_RLE4) {
		/* we need an intermediate buffer */
		/* Overflow check already done for reader->input_step_bytes */
		reader->rle_row = yabmp_malloc(reader, reader->input_row_bytes * 2U);
		if (reader->rle_row == NULL) {
			return YABMP_ERR_ALLOCATION;
		}
	}
	
	if (reader->transforms & YABMP_TRANSFORM_EXPAND) {
		if (reader->info2.bpp == 1U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal1_to_bgr24;
		}
		else if (reader->info2.bpp == 2U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal2_to_bgr24;
		}
		else if ((reader->info2.bpp == 4U) && (reader->info2.compression != YABMP_COMPRESSION_RLE4)) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal4_to_bgr24;
		}
		else if ((reader->info2.bpp == 8U) || (reader->info2.compression == YABMP_COMPRESSION_RLE4)) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal8_to_bgr24;
		}
		else if (reader->info2.bpp == 16U) {
			if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_ALPHA) {
				if (reader->info2.expanded_bps == 8U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf16u_to_bgra32;
				} else {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf16u_to_bgra64;
				}
			} else {
				if (reader->info2.expanded_bps == 8U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf16u_to_bgr24;
				} else {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf16u_to_bgr48;
				}
			}
		} else if (reader->info2.bpp == 32U) {
			if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_ALPHA) {
				if (reader->info2.expanded_bps == 8U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf32u_to_bgra32;
				} else if (reader->info2.expanded_bps == 16U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf32u_to_bgra64;
				} else {
					/* TODO yabmp_bf32u_to_bgra128 ??? */
					yabmp_send_error(reader, "Can't expand to %ubpp sample.", 4U * (unsigned int)reader->info2.expanded_bps);
					return YABMP_ERR_UNKNOW;
				}
			} else  {
				if (reader->info2.expanded_bps == 8U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf32u_to_bgr24;
				} else if (reader->info2.expanded_bps == 16U) {
					reader->transform_fn = (yabmp_transform_fn)yabmp_bf32u_to_bgr48;
				} else {
					/* TODO yabmp_bf32u_to_bgr96 ??? */
					yabmp_send_error(reader, "Can't expand to %ubpp sample.", 3U * (unsigned int)reader->info2.expanded_bps);
					return YABMP_ERR_UNKNOW;
				}
			}
		}
	}
	else if (reader->transforms & YABMP_TRANSFORM_GRAYSCALE) {
		if (reader->info2.bpp == 1U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal1_to_y8;
		}
		else if (reader->info2.bpp == 2U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal2_to_y8;
		}
		else if (reader->info2.bpp == 4U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal4_to_y8;
		}
		else if (reader->info2.bpp == 8U) {
			reader->transform_fn = (yabmp_transform_fn)yabmp_pal8_to_y8;
		} else {
			/* TODO ??? */
			yabmp_send_error(reader, "Can't expand palette from %ubpp sample.", (unsigned int)reader->info2.bpp);
			return YABMP_ERR_UNKNOW;
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
		yabmp_send_error(reader, "yabmp_read_info not called.");
		return YABMP_ERR_UNKNOW;
	}
	
	if ((reader->status & YABMP_STATUS_HAS_VALID_INFO) == 0U) {
		yabmp_send_error(reader, "Invalid info were found.");
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
	
	if (reader->transform_fn != NULL) {
		switch (reader->info2.compression) {
			case YABMP_COMPRESSION_RLE8:
				YABMP_SIMPLE_CHECK(local_rle8_decode_row(reader, reader->input_row));
				break;
			case YABMP_COMPRESSION_RLE4:
				YABMP_SIMPLE_CHECK(local_rle4_decode_row(reader, reader->input_row, 0));
				break;
		default:
				YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, reader->input_row, reader->input_step_bytes));
#if defined(YABMP_BIG_ENDIAN)
				if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_BITFIELDS) {
					if (reader->info2.bpp == 16U) {
						yabmp_swap16u(reader, reader->input_row);
					} else {
						yabmp_swap32u(reader, reader->input_row);
					}
				}
#endif
				break;
		}
		reader->transform_fn(reader, reader->input_row, row);
	}
	else {
		switch (reader->info2.compression) {
			case YABMP_COMPRESSION_RLE8:
				YABMP_SIMPLE_CHECK(local_rle8_decode_row(reader, row));
				break;
			case YABMP_COMPRESSION_RLE4:
				YABMP_SIMPLE_CHECK(local_rle4_decode_row(reader, row, 1));
				break;
			default:
				YABMP_SIMPLE_CHECK(yabmp_stream_read(reader, row, reader->input_row_bytes));
#if defined(YABMP_BIG_ENDIAN)
				if ((reader->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_BITFIELDS) {
					if (reader->info2.bpp == 16U) {
						yabmp_swap16u(reader, row);
					} else {
						yabmp_swap32u(reader, row);
					}
				}
#endif
				YABMP_SIMPLE_CHECK(yabmp_stream_skip(reader, (yabmp_uint32)(reader->input_step_bytes - reader->input_row_bytes)));
				break;
		}
	}
	
	if (reader->transforms & YABMP_TRANSFORM_SCAN_ORDER) {
		if (reader->stream_offset >= 2U * reader->input_step_bytes) {
			/* not last line to read */
			YABMP_SIMPLE_CHECK(yabmp_stream_seek(reader, reader->stream_offset - 2U * reader->input_step_bytes));
		}
	}
	reader->status |= YABMP_STATUS_HAS_LINES;
	
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_set_invert_scan_direction, (yabmp* instance))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if (instance->seek_fn == NULL) {
		yabmp_send_error(instance, "Scan direction change is only supported with a non NULL seek function.");
		return YABMP_ERR_UNKNOW;
	}
	
	switch (instance->info2.compression)
	{
		case YABMP_COMPRESSION_NONE:
			instance->transforms |= YABMP_TRANSFORM_SCAN_ORDER;
			break;
  default:
			yabmp_send_error(instance, "Scan direction change is only supported for YABMP_COMPRESSION_NONE & YABMP_COMPRESSION_BITFIELDS.");
			return YABMP_ERR_UNKNOW;
	}
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_set_expand_to_bgrx, (yabmp* instance))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if (instance->info2.expanded_bps > 16U) {
		yabmp_send_error(instance, "yabmp_set_expand_to_bgrx is not valid for channel depth > 16.");
		return YABMP_ERR_UNKNOW;
	}
	instance->transforms |= YABMP_TRANSFORM_EXPAND;
	
	return YABMP_OK;
}

YABMP_API(yabmp_status, yabmp_set_expand_to_grayscale, (yabmp* instance))
{
	YABMP_CHECK_INSTANCE(instance);
	
	if (((instance->info2.flags >> YABMP_COLOR_SHIFT) & YABMP_COLOR_MASK_COLOR) != 0U) {
		yabmp_send_error(instance, "yabmp_set_grayscale is only valid for non-color palette image.");
		return YABMP_ERR_UNKNOW;
	}
	
	instance->transforms |= YABMP_TRANSFORM_GRAYSCALE;

	return YABMP_OK;
}
