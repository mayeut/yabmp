
# Defines the source code for the library
set(YABMP_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_info.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_malloc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_message.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_reader.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_rtransforms.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/yabmp_stream.c"
  
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/yabmp.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/yabmp_types.h"
  "${CMAKE_CURRENT_BINARY_DIR}/inc/private/yabmp_config.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_api.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_checks.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_info.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_internal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_malloc.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_message.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_rtransforms.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_stream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_struct.h"
)

include(TestBigEndian)
include(CheckCSourceCompiles)

test_big_endian(YABMP_BIG_ENDIAN)

check_c_source_compiles("
  int main() {
    return (int)__builtin_bswap16(0U);
  }
" YABMP_HAVE_GCC_BYTESWAP_16)

check_c_source_compiles("
  int main() {
    return (int)__builtin_bswap32(0U);
  }
" YABMP_HAVE_GCC_BYTESWAP_32)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/inc/private/yabmp_config.h.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/inc/private/yabmp_config.h")

# Build the library
add_library(${YABMP_LIBRARY_NAME} ${YABMP_SRCS})
set_target_properties(${YABMP_LIBRARY_NAME} PROPERTIES ${YABMP_LIBRARY_PROPERTIES})
target_include_directories(${YABMP_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/inc/private")
if(UNIX AND NOT APPLE)
	if(BUILD_SHARED_LIBS)
		target_link_libraries(${YABMP_LIBRARY_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/versions.ldscript")
	endif()
endif()
if(OPJ_USE_DSYMUTIL)
  if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${YABMP_LIBRARY_NAME} POST_BUILD 
    COMMAND "dsymutil" "$<TARGET_SONAME_FILE:${YABMP_LIBRARY_NAME}>"
    COMMENT "dsymutil $<TARGET_SONAME_FILE:${YABMP_LIBRARY_NAME}>"
    DEPENDS ${YABMP_LIBRARY_NAME})
  endif()
endif()
