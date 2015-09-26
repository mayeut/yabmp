find_package(Doxygen)

if(DOXYGEN_FOUND)
  # Configure the doxygen config file with variable from CMake and move it
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  
  set(YABMP_HEADERS
    ${CMAKE_SOURCE_DIR}/lib/yabmp/inc/yabmp.h
    ${CMAKE_SOURCE_DIR}/lib/yabmp/inc/yabmp_types.h
  )
  # Generate new target to build the html documentation
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            ${CMAKE_CURRENT_BINARY_DIR}/mainpage.h
            ${YABMP_HEADERS}
  )
  add_custom_target(doc ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMENT "Building doxygen documentation"
  )
else()
  message(STATUS "Doxygen not found, we cannot generate the documentation")
endif()
