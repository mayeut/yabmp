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
#ifndef YABMP_INTERNAL_H
#define YABMP_INTERNAL_H

#include <string.h>

#define YABMP_STRINGIFY(s) YABMP_STRINGIFY_IMPL(s)
#define YABMP_STRINGIFY_IMPL(s) #s

#if defined(__GNUC__)
#	define YABMP_UNUSED __attribute__((unused))
#	else
# define YABMP_UNUSED
#endif

#include <yabmp_config.h>
#include "yabmp_api.h"
#include "yabmp_struct.h"
#include "yabmp_info.h"
#include "yabmp_malloc.h"
#include "yabmp_message.h"
#include "yabmp_stream.h"
#include "yabmp_checks.h"
#include "yabmp_rtransforms.h"

#endif /* YABMP_INTERNAL_H */
