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
#ifndef YABMP_TYPES_H
#define YABMP_TYPES_H

#include <limits.h>
#include <stddef.h>

/* Let's use constants from limits.h to define yabmp integer types */
#if CHAR_BIT == 8 && UCHAR_MAX == 255
typedef unsigned char yabmp_uint8;  /**< 8-bit unsigned integer. Unsigned integer type with width of exactly 8 bits. */
#else
#	error "yabmp requires an 8-bit unsigned integer type"
#endif

#if UINT_MAX == 65535
typedef unsigned int yabmp_uint16;
#elif USHRT_MAX == 65535
typedef unsigned short yabmp_uint16; /**< 16-bit unsigned integer. Unsigned integer type with width of exactly 16 bits. */
#else
#	error "yabmp requires a 16-bit unsigned integer type"
#endif

#if UINT_MAX == 4294967295
typedef unsigned int yabmp_uint32; /**< 32-bit unsigned integer. Unsigned integer type with width of exactly 32 bits. */
#	define YABMP_PRIu32 "u" /**< \brief Format conversion specifier to output an unsigned decimal integer value of type #yabmp_uint32. */
# define YABMP_PRIX32 "X" /**< \brief Format conversion specifier to output an unsigned uppercase hexadecimal integer value of type #yabmp_uint32. */
#	define YABMP_UINT32_MAX UINT_MAX /**< Maximum value of an object of type #yabmp_uint32. */
#elif ULONG_MAX == 4294967295
typedef unsigned long int yabmp_uint32;
#	define YABMP_PRIu32 "lu"
# define YABMP_PRIX32 "lX"
#	define YABMP_UINT32_MAX ULONG_MAX
#else
#	error "yabmp requires a 32-bit unsigned integer type"
#endif

#endif /* YABMP_TYPES_H */
