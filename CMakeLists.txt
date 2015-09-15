# Main CMakeLists.txt to build yabmp using CMake (www.cmake.org)

cmake_minimum_required(VERSION 3.3.1)

# --------------------------------------------------------------------------
# Path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
    
set(YABMP_LIBRARY_NAME yabmp)

project(YABMP VERSION 1.0.0 LANGUAGES C)

set(PACKAGE_VERSION "${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}")

set(YABMP_LIBRARY_PROPERTIES
  VERSION   "${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}"
  SOVERSION "${YABMP_VERSION_MAJOR}"
)

set(YABMP_INSTALL_SUBDIR "${YABMP_LIBRARY_NAME}-${YABMP_VERSION_MAJOR}")

if(NOT YABMP_INSTALL_BIN_DIR)
  set(YABMP_INSTALL_BIN_DIR "bin")
endif()

if(NOT YABMP_INSTALL_LIB_DIR)
  set(YABMP_INSTALL_LIB_DIR "lib")
endif()

if(NOT YABMP_INSTALL_INCLUDE_DIR)
  set(YABMP_INSTALL_INCLUDE_DIR "include/${YABMP_INSTALL_SUBDIR}")
endif()

if (APPLE)
	list(APPEND YABMP_LIBRARY_PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${YABMP_INSTALL_LIB_DIR}")
	option(YABMP_USE_DSYMUTIL "Call dsymutil on binaries after build." OFF)
endif()

#-----------------------------------------------------------------------------
# Big endian test:
include (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(YABMP_BIG_ENDIAN)

#-----------------------------------------------------------------------------
# yabmp build configuration options.
option(BUILD_SHARED_LIBS "Build shared library and link executables against it." ON)
set (EXECUTABLE_OUTPUT_PATH ${YABMP_BINARY_DIR}/build CACHE PATH "Single output directory for building all executables.")
set (LIBRARY_OUTPUT_PATH ${YABMP_BINARY_DIR}/build CACHE PATH "Single output directory for building all libraries.")
mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

#-----------------------------------------------------------------------------
# Compiler specific flags:
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "-Wall -Werror ${CMAKE_C_FLAGS}") # "All" warnings (c.f. gcc documentation). Warnings are error.
endif()

# Check if some include files are provided by the system
include (EnsureFileInclude)
# These files are mandatory
ensure_file_include("stdlib.h"   HAVE_STDLIB_H   YES)
ensure_file_include("string.h"   HAVE_STRING_H   YES)
ensure_file_include("stdio.h"    HAVE_STDIO_H    YES)
ensure_file_include("stdarg.h"   HAVE_STDARG_H   YES)
ensure_file_include("assert.h"   HAVE_ASSERT_H   YES)
ensure_file_include("stdint.h"   HAVE_STDINT_H   YES)
ensure_file_include("inttypes.h" HAVE_INTTYPES_H YES)

#-----------------------------------------------------------------------------
# Build Library
add_subdirectory(lib)

#-----------------------------------------------------------------------------
# Build Thirdparty
add_subdirectory(thirdparty)

#-----------------------------------------------------------------------------
# Build Applications
add_subdirectory(bin)

