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
#ifndef YABMP_H
#define YABMP_H

#include "yabmp_types.h"

/* C library */
#ifdef __cplusplus
	extern "C" {
#endif

#ifndef YABMP_API
/** 
 * Macro to easily change API definition.
 */
#	define YABMP_API(return_type, symbol_name, arguments) return_type symbol_name arguments
#endif

#define YABMP_SCAN_BOTTOM_UP 0U /**< Image scan direction is Bottom-Up. */
#define YABMP_SCAN_TOP_DOWN  1U /**< Image scan direction is Top-Down. */
		
#define YABMP_COLOR_MASK_PALETTE   1U /**< Image uses a palette.       */
#define YABMP_COLOR_MASK_COLOR     2U /**< Image is a color image.     */
#define YABMP_COLOR_MASK_ALPHA     4U /**< Image has an alpha channel. */
#define YABMP_COLOR_MASK_BITFIELDS 8U /**< Image uses bitfields.       */
		
#define YABMP_COLOR_TYPE_BGR             (YABMP_COLOR_MASK_COLOR)
#define YABMP_COLOR_TYPE_PALETTE         (YABMP_COLOR_MASK_COLOR | YABMP_COLOR_MASK_PALETTE)
#define YABMP_COLOR_TYPE_GRAY_PALETTE    (YABMP_COLOR_MASK_PALETTE)
#define YABMP_COLOR_TYPE_BITFIELDS       (YABMP_COLOR_MASK_COLOR | YABMP_COLOR_MASK_BITFIELDS)
#define YABMP_COLOR_TYPE_BITFIELDS_ALPHA (YABMP_COLOR_MASK_COLOR | YABMP_COLOR_MASK_BITFIELDS | YABMP_COLOR_MASK_ALPHA)
		
#define YABMP_COMPRESSION_NONE      0U /**< Image data is not compressed. */
#define YABMP_COMPRESSION_RLE8      1U /**< Image data is compressed using RLE8 algorithm. */
#define YABMP_COMPRESSION_RLE4      2U /**< Image data is compressed using RLE4 algorithm. */

/**
 * Return codes
 */
typedef enum yabmp_status_enum
{
	YABMP_OK = 0,          /**< No error */
	YABMP_ERR_UNKNOW,      /**< Unknown error */
	YABMP_ERR_ALLOCATION,  /**< Allocation failed */
	YABMP_ERR_INVALID_ARGS /**< Invalid arguments */
} yabmp_status;

/**
 * YABMP reader/writer opaque structure.
 */
typedef struct yabmp_struct      yabmp;
/**
 * YABMP info opaque structure.
 */
typedef struct yabmp_info_struct yabmp_info;
		
/**
 * Palette color.
 */
typedef struct yabmp_color_struct
{
   yabmp_uint8 red;   /**< Red component of the color */
   yabmp_uint8 green; /**< Green component of the color */
   yabmp_uint8 blue;  /**< Blue component of the color */
} yabmp_color;
		
/**
 * Callback function prototype for messages
 * @param[in] context User message context provided in #yabmp_create_reader.
 * @param[in] message Message sent by the library.
 *
 * @see
 *   yabmp_create_reader
 */
typedef void  (*yabmp_message_cb)(void* context, const char* message);
/**
 * Callback function prototype for allocations
 * @param[in] context User allocation context provided in #yabmp_create_reader.
 * @param[in] size    Size of the memory block to be allocated.
 * @return Pointer to an allocated memory block of size \a size on success. NULL on failure.
 *
 * @see
 *   yabmp_create_reader
 */
typedef void* (*yabmp_malloc_cb)(void* context, size_t size);
/**
 * Callback function prototype for desallocations
 * @param[in] context User allocation context provided in #yabmp_create_reader.
 * @param[in] ptr     Pointer to a memory block allocated using the #yabmp_malloc_cb provided in #yabmp_create_reader.
 *
 * @see
 *   yabmp_create_reader
 */
typedef void  (*yabmp_free_cb)(void* context, void* ptr);
		
/**
 * Callback function prototype for stream reading
 * @param[in] context User stream context provided in #yabmp_set_input_stream.
 * @param[in] ptr     Pointer to a memory block in which data from input stream shall be copied.
 * @param[in] size    Number of bytes to be copied.
 * @return Number of bytes copied into \a ptr.
 *
 * @see
 *   yabmp_set_input_stream
 */
typedef size_t (*yabmp_stream_read_cb) (void* context, void * ptr, size_t size);
		
/**
 * Callback function prototype for stream reading
 * @param[in] context User stream context provided in #yabmp_set_output_stream.
 * @param[in] ptr     Pointer to a memory block from which data shall be copied.
 * @param[in] size    Number of bytes to be copied.
 * @return Number of bytes written into the output stream.
 *
 * @see
 *   yabmp_set_output_stream
 */
typedef size_t (*yabmp_stream_write_cb)(void* context, const void * ptr, size_t size);
/**
 * Callback function prototype for stream seeking
 * @param[in] context User stream context provided in #yabmp_set_input_stream or #yabmp_set_output_stream.
 * @param[in] offset  Offset from beginning of stream in bytes (SEEK_SET).
 * @return #YABMP_OK if successful.
 *
 * @see
 *   yabmp_set_input_stream\n
 *   yabmp_set_output_stream
 */
typedef yabmp_status (*yabmp_stream_seek_cb) (void* context, yabmp_uint32 offset);
		
/**
 * Callback function prototype to close a stream
 * @param[in] context User stream context provided in #yabmp_set_input_stream or #yabmp_set_output_stream.
 *
 * @see
 *   yabmp_set_input_stream\n
 *   yabmp_set_output_stream
 */
typedef void (*yabmp_stream_close_cb) (void* context);
		
/**
 * Gets library version components as integers.
 *
 * @param[out] major Major version (optional, can be NULL)
 * @param[out] minor Minor version (optional, can be NULL)
 * @param[out] patch Patch version (optional, can be NULL)
 *
 */
YABMP_API(void, yabmp_get_version, (yabmp_uint32 * major, yabmp_uint32 * minor, yabmp_uint32 * patch));
		
/**
 * Gets library version.
 *
 * @return Version as C string. (e.g. "0.1.0").
 *
 * @remarks
 *   The string returned shall not be freed.
 *
 */
YABMP_API(const char*, yabmp_get_version_string, ());
		
/**
 * Creates a BMP reader.
 *
 * @param[out] reader          Pointer to the reader object.
 * @param[in]  message_context User context that will be provided to \a error_fn & \a warning_fn when called.
 * @param[in]  error_fn        Message callback that will be called on error. Optional, can be NULL.
 * @param[in]  warning_fn      Message callback that will be called on warning. Optional, can be NULL.
 * @param[in]  alloc_context   User context that will be provided to \a malloc_fn & \a free_fn when called.
 * @param[in]  malloc_fn       Callback that will be called for allocation. Optional, can be NULL only if \a free_fn is also NULL.
 * @param[in]  free_fn         Callback that will be called for desallocation. Optional, can be NULL only if \a malloc_fn is also NULL.
 *
 * @return 
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_ALLOCATION on allocation failure.
 *
 */
YABMP_API(yabmp_status, yabmp_create_reader, (
	yabmp** reader,
	void* message_context,
	yabmp_message_cb error_fn,
	yabmp_message_cb warning_fn,
	void* alloc_context,
	yabmp_malloc_cb malloc_fn,
	yabmp_free_cb free_fn
));
		
/**
 * Destroys a BMP reader.
 *
 * @param[in, out] reader Pointer to the reader object.
 * @param[in, out] info   Pointer to the information object.
 *
 */
YABMP_API(void, yabmp_destroy_reader, (yabmp** reader, yabmp_info** info));

/**
 * Sets input stream.
 *
 * @param[in]  reader         Pointer to the reader object.
 * @param[in]  stream_context User context that will be provided to \a read_fn, \a seek_fn & \a close_fn when called.
 * @param[in]  read_fn        Callback that will be called to read data from input stream.
 * @param[in]  seek_fn        Callback that will be called to seek in the input stream. Optional, can be NULL (see remarks).
 * @param[in]  close_fn       Callback that will be called to close the input stream. Optional, can be NULL.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.
 *
 * @remarks
 *   If \a seek_fn is NULL, it won't be possible to use #yabmp_set_invert_scan_direction.
 *
 */
YABMP_API(yabmp_status, yabmp_set_input_stream, (
	yabmp* reader,
	void* stream_context,
	yabmp_stream_read_cb  read_fn,
	yabmp_stream_seek_cb  seek_fn,
	yabmp_stream_close_cb close_fn
));
		
/**
 * Sets input file.
 *
 * @param[in]  reader Pointer to the reader object.
 * @param[in]  path   Path to the file to be opened.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_UNKNOW if the file can't be opened.
 *
 */
YABMP_API(yabmp_status, yabmp_set_input_file, (
	yabmp* reader,
	const char* path
));

/**
 * Creates an information object.
 *
 * @param[in]  instance Pointer to the reader/writer object.
 * @param[out] info     Pointer to the information object.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_ALLOCATION on allocation failure.
 *
 */
YABMP_API(yabmp_status, yabmp_create_info, (yabmp* instance, yabmp_info ** info));

/**
 * Reads image information from the input stream.
 *
 * An input stream must have been set for this \a reader object using #yabmp_set_input_stream or #yabmp_set_input_file.
 *
 * @param[in]  reader  Pointer to the reader object.
 * @param[out] info    Pointer to the information object.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_UNKNOW in other failure cases.
 *
 * @see
 *   yabmp_set_input_stream\n
 *   yabmp_set_input_file
 *
 */
YABMP_API(yabmp_status, yabmp_read_info, (yabmp* reader, yabmp_info * info));

/**
 * Reads a row of image data from the input stream.
 *
 * Image information must have been read for this \a reader object using #yabmp_read_info.
 *
 * @param[in]  reader   Pointer to the reader object.
 * @param[in]  row      Pointer to the buffer that will receive image data.
 * @param[in]  row_size Size of the \a row buffer in bytes.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_ALLOCATION on allocation failure.
 * #YABMP_ERR_UNKNOW in other failure cases.
 *
 * @see
 *   yabmp_read_info\n
 *   yabmp_set_invert_scan_direction\n
 *   yabmp_set_expand_to_bgrx\n
 *   yabmp_set_expand_to_grayscale\n
 *
 */
YABMP_API(yabmp_status, yabmp_read_row, (yabmp* reader, void* row, size_t row_size));

YABMP_API(yabmp_status, yabmp_get_dimensions, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* width, yabmp_uint32* height));
YABMP_API(yabmp_status, yabmp_get_pixels_per_meter, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* ppm_x, yabmp_uint32* ppm_y));
YABMP_API(yabmp_status, yabmp_get_bit_depth, (const yabmp* instance, const yabmp_info* info, unsigned int* bit_depth));
YABMP_API(yabmp_status, yabmp_get_color_type, (const yabmp* instance, const yabmp_info* info, unsigned int* color_type));
YABMP_API(yabmp_status, yabmp_get_compression_type, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* compression));
YABMP_API(yabmp_status, yabmp_get_scan_direction, (const yabmp* instance, const yabmp_info* info, unsigned int* scan_direction));
YABMP_API(yabmp_status, yabmp_get_bitfields, (const yabmp* instance, const yabmp_info* info, yabmp_uint32* blue_mask, yabmp_uint32* green_mask, yabmp_uint32 * red_mask, yabmp_uint32 * alpha_mask));
YABMP_API(yabmp_status, yabmp_get_bits, (const yabmp* instance, const yabmp_info* info, unsigned int* blue_bits, unsigned int* green_bits, unsigned int * red_bits, unsigned int * alpha_bits));
YABMP_API(yabmp_status, yabmp_get_palette, (const yabmp* instance, const yabmp_info* info, unsigned int * color_count, yabmp_color const** palette));

/**
 * Invert scan direction.
 *
 * Image rows will be read in the opposite direction as what's reported by #yabmp_get_scan_direction.
 * This requires a seek function to be set when calling #yabmp_set_input_stream.
 * Inverting scan direction is only supported for #YABMP_COMPRESSION_NONE compression type.
 *
 * @param[in]  instance Pointer to the reader object.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 * #YABMP_ERR_UNKNOW when no seek function is set or compression type is not #YABMP_COMPRESSION_NONE.
 *
 * @see
 *   yabmp_get_scan_direction\n
 *   yabmp_get_compression_type\n
 *   yabmp_set_input_stream
 *
 */
YABMP_API(yabmp_status, yabmp_set_invert_scan_direction, (yabmp* instance));
/**
 * Expand to BGR(A).
 *
 * Image rows will be read in BGR(A) format.
 *
 * @param[in]  instance Pointer to the reader object.
 *
 * @return
 * #YABMP_OK on success.\n
 * #YABMP_ERR_INVALID_ARGS when invalid arguments are provided.\n
 *
 */
YABMP_API(yabmp_status, yabmp_set_expand_to_bgrx, (yabmp* instance));
YABMP_API(yabmp_status, yabmp_set_expand_to_grayscale, (yabmp* instance));
		
#ifdef __cplusplus
	}
#endif

#endif /* YABMP_H */
