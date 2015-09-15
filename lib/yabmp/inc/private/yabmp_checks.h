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
#ifndef YABMP_CHECKS_H
#define YABMP_CHECKS_H

#include "yabmp_api.h"
#include "yabmp_struct.h"

#define YABMP_SIMPLE_CHECK(call) do { \
		yabmp_status l_check_status = call; \
		if (l_check_status != YABMP_OK) { \
			return l_check_status; \
		} \
	} while (0)

#define YABMP_CHECK_INSTANCE_BASE(instance, checkA, checkB) do { \
		if (instance == NULL) { \
			return YABMP_ERR_INVALID_ARGS; \
		} \
		if ((instance->kind != checkA) && (instance->kind != checkB)) { \
			return YABMP_ERR_INVALID_ARGS; \
		} \
	} while (0)

#define YABMP_CHECK_INSTANCE(instance) YABMP_CHECK_INSTANCE_BASE(instance, YABMP_KIND_READER, YABMP_KIND_WRITER)
#define YABMP_CHECK_READER(instance)   YABMP_CHECK_INSTANCE_BASE(instance, YABMP_KIND_READER, YABMP_KIND_READER)
#define YABMP_CHECK_WRITER(instance)   YABMP_CHECK_INSTANCE_BASE(instance, YABMP_KIND_WRITER, YABMP_KIND_WRITER)


#endif /* YABMP_CHECKS_H */
