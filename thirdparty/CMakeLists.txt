# This will build third party

option(YABMP_BUILD_ZLIB "Build thirdparty zlib." OFF)
option(YABMP_BUILD_PNG "Build thirdparty libpng." OFF)

include(CheckTypeSize)
include(CheckIncludeFile)
include(CheckFunctionExists)

# Build zlib
if (YABMP_BUILD_ZLIB)

	check_include_file(sys/types.h HAVE_SYS_TYPES_H)
	check_include_file(stdint.h    HAVE_STDINT_H)
	check_include_file(stddef.h    HAVE_STDDEF_H)
	check_include_file(unistd.h    HAVE_UNISTD_H)
	check_include_file(stdarg.h    HAVE_STDARG_H)

	if(HAVE_SYS_TYPES_H)
		list(APPEND ZLIB_PUBLIC_DEFS -DHAVE_SYS_TYPES_H)
	endif()
	if(HAVE_STDINT_H)
		list(APPEND ZLIB_PUBLIC_DEFS -DHAVE_STDINT_H)
	endif()
	if(HAVE_STDDEF_H)
		list(APPEND ZLIB_PUBLIC_DEFS -DHAVE_STDDEF_H)
	endif()
	if(HAVE_UNISTD_H)
		list(APPEND ZLIB_PUBLIC_DEFS -DHAVE_UNISTD_H)
	endif()
	if(HAVE_STDARG_H)
		list(APPEND ZLIB_PUBLIC_DEFS -DHAVE_STDARG_H)
	endif()
	
	
	#
	# Check to see if we have large file support
	#
	set(CMAKE_REQUIRED_DEFINITIONS ${ZLIB_DEFS})
	list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
	check_type_size(off64_t OFF64_T)
	if(HAVE_OFF64_T)
		list(APPEND ZLIB_PUBLIC_DEFS -D_LARGEFILE64_SOURCE=1)
	endif()
	set(CMAKE_REQUIRED_DEFINITIONS) # clear variable
	
	#
	# Check for fseeko
	#
	check_function_exists(fseeko HAVE_FSEEKO)
	if(NOT HAVE_FSEEKO)
		list(APPEND ZLIB_PUBLIC_DEFS -DNO_FSEEKO)
	endif()
	if(MSVC)
		list(APPEND ZLIB_PRIVATE_DEFS -D_CRT_SECURE_NO_DEPRECATE)
		list(APPEND ZLIB_PRIVATE_DEFS -D_CRT_NONSTDC_NO_DEPRECATE)
	endif()
	
	#we use predefined file
	#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/zlib/zconf.h.cmakein ${CMAKE_CURRENT_BINARY_DIR}/zlib/zconf.h @ONLY)
	
	file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/zlib/zlib.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/zlib)
	file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/zlib/zconf.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/zlib)
	set(ZLIB_PUBLIC_HDRS
		${CMAKE_CURRENT_BINARY_DIR}/zlib/zconf.h
		${CMAKE_CURRENT_BINARY_DIR}/zlib/zlib.h
	)
	set(ZLIB_PRIVATE_HDRS
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/crc32.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/deflate.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/gzguts.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inffast.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inffixed.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inflate.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inftrees.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/trees.h
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/zutil.h
	)
	set(ZLIB_SRCS
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/adler32.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/compress.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/crc32.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/deflate.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/gzclose.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/gzlib.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/gzread.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/gzwrite.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inflate.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/infback.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inftrees.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/inffast.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/trees.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/uncompr.c
		${CMAKE_CURRENT_SOURCE_DIR}/zlib/zutil.c
	)
	add_library(zlibstatic STATIC ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
	if(ZLIB_PUBLIC_DEFS)
		target_compile_definitions(zlibstatic PUBLIC ${ZLIB_PUBLIC_DEFS})
	endif(ZLIB_PUBLIC_DEFS)
	if(ZLIB_PRIVATE_DEFS)
		target_compile_definitions(zlibstatic PRIVATE ${ZLIB_PRIVATE_DEFS})
	endif(ZLIB_PRIVATE_DEFS)
	set(ZLIB_LIBRARY zlibstatic CACHE INTERNAL "thirdparty zlib target" FORCE)
	set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/zlib CACHE INTERNAL "thirdparty zlib includes" FORCE)
elseif("${ZLIB_LIBRARY}" STREQUAL "zlibstatic")
	unset(ZLIB_LIBRARY     CACHE)
	unset(ZLIB_INCLUDE_DIR CACHE)
endif(YABMP_BUILD_ZLIB)

# Build libpng
if (YABMP_BUILD_PNG)
	unset(PNG_LIBRARY_DEBUG   CACHE)
	unset(PNG_LIBRARY_RELEASE CACHE)
	find_package(ZLIB REQUIRED)
	if(NOT WIN32)
  	find_library(M_LIBRARY
    	NAMES m
    	PATHS /usr/lib /usr/local/lib
		)
		if(NOT M_LIBRARY)
			message(ERROR "math lib 'libm' not found; PNG build failed")
		endif()
	else()
		# not needed on windows
		set(M_LIBRARY)
	endif()
	
	if(ZLIB_PUBLIC_DEFS)
		list(APPEND PNG_PRIVATE_DEFS ${ZLIB_PUBLIC_DEFS})
	endif(ZLIB_PUBLIC_DEFS)
	if(MSVC)
		list(APPEND PNG_PRIVATE_DEFS -D_CRT_SECURE_NO_DEPRECATE)
	endif(MSVC)
	
	file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.h ${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngconf.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libpng)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt ${CMAKE_CURRENT_BINARY_DIR}/libpng/pnglibconf.h)
	set(PNG_PUBLIC_HDRS
		${CMAKE_CURRENT_BINARY_DIR}/libpng/png.h
		${CMAKE_CURRENT_BINARY_DIR}/libpng/pngconf.h
		${CMAKE_CURRENT_BINARY_DIR}/libpng/pnglibconf.h
	)
	set(PNG_PRIVATE_HDRS
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngdebug.h
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pnginfo.h
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngpriv.h
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngstruct.h
	)
	set(PNG_SRCS
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/png.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngerror.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngget.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngmem.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngpread.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngread.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngrio.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngrtran.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngrutil.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngset.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngtrans.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngwio.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngwrite.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngwtran.c
		${CMAKE_CURRENT_SOURCE_DIR}/libpng/pngwutil.c
	)
	add_library(pngstatic STATIC ${PNG_SRCS} ${PNG_PUBLIC_HDRS} ${PNG_PRIVATE_HDRS})
	target_include_directories(pngstatic PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpng)
	if(PNG_PRIVATE_DEFS)
		target_compile_definitions(pngstatic PRIVATE ${PNG_PRIVATE_DEFS} ${ZLIB_PUBLIC_DEFS})
	endif(PNG_PRIVATE_DEFS)
	if(M_LIBRARY)
		target_link_libraries(pngstatic ${M_LIBRARY})
	endif(M_LIBRARY)
	set(PNG_LIBRARY pngstatic CACHE INTERNAL "thirdparty libpng target" FORCE)
	set(PNG_PNG_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libpng CACHE INTERNAL "thirdparty libpng includes" FORCE)
elseif("${PNG_LIBRARY}" STREQUAL "pngstatic")
	unset(PNG_LIBRARY         CACHE)
	unset(PNG_PNG_INCLUDE_DIR CACHE)
 
endif(YABMP_BUILD_PNG)

# Build optparse
check_include_file(stdio.h HAVE_STDIO_H)
if(NOT HAVE_STDIO_H)
	message(FATAL_ERROR "The file stdio.h is mandatory but not found on your system")
endif(NOT HAVE_STDIO_H)
add_library(optparse STATIC ${CMAKE_CURRENT_SOURCE_DIR}/optparse/optparse.c ${CMAKE_CURRENT_SOURCE_DIR}/optparse/optparse.h)
target_include_directories(optparse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/optparse)
