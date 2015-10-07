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
#ifndef YABMP_API_H
#define YABMP_API_H

#if defined(yabmp_EXPORTS)
#	if defined(_MSC_VER)
#		define YABMP_VISIBILITY_PUBLIC __declspec(dllexport)
# elif defined(__GNUC__) && __GNUC__ >= 4
#		define YABMP_VISIBILITY_PUBLIC __attribute__((visibility ("default")))
#	else
#		define YABMP_VISIBILITY_PUBLIC
#	endif
#else
# if defined(__GNUC__) && __GNUC__ >= 4
#		define YABMP_VISIBILITY_PUBLIC __attribute__((visibility ("hidden")))
#	else
#		define YABMP_VISIBILITY_PUBLIC
#	endif
#endif

# if defined(__GNUC__) && __GNUC__ >= 4
#		define YABMP_VISIBILITY_INTERNAL __attribute__((visibility ("hidden")))
#	else
#		define YABMP_VISIBILITY_INTERNAL
#	endif

#define YABMP_API(return_type, symbol_name, arguments)  YABMP_VISIBILITY_PUBLIC   return_type symbol_name arguments
#define YABMP_IAPI(return_type, symbol_name, arguments) YABMP_VISIBILITY_INTERNAL return_type symbol_name arguments

#include "../yabmp.h"

/* This one will be needed by every source file */
#include <assert.h>

#endif /* YABMP_API_H */
