
FIND_PACKAGE(ZLIB)
IF(NOT ZLIB_FOUND)
  message(ERROR "Z lib not found !")
ENDIF(NOT ZLIB_FOUND)

FIND_PACKAGE(PNG)
IF(NOT PNG_FOUND)
  message(ERROR "PNG lib not found !")
ENDIF(NOT PNG_FOUND)

# Headers file are located here:
include_directories(
  ${PNG_INCLUDE_DIRS}
  )

add_executable(yabmpconvert "${CMAKE_CURRENT_SOURCE_DIR}/yabmpconvert.c" "${CMAKE_CURRENT_SOURCE_DIR}/yabmpconvert_topng.c" "${CMAKE_CURRENT_SOURCE_DIR}/../common/yabmp_printinfo.c")
target_link_libraries(yabmpconvert ${YABMP_LIBRARY_NAME} ${PNG_LIBRARIES} optparse)

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET yabmpconvert POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:yabmpconvert>"
  COMMENT "dsymutil $<TARGET_FILE:yabmpconvert>"
  DEPENDS yabmpconvert)
endif()

# -v exit code 0
add_test(NAME yabmpconvert-version-1 COMMAND yabmpconvert -v)
# --version prints correct version
add_test(NAME yabmpconvert-version-2 COMMAND yabmpconvert --version)
set_tests_properties(yabmpconvert-version-2 PROPERTIES PASS_REGULAR_EXPRESSION "^$<TARGET_FILE_NAME:yabmpconvert> ${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}\n$")
# --help exit code 0
add_test(NAME yabmpconvert-help-1 COMMAND yabmpconvert --help)
# -hv prints correct version & help
add_test(NAME yabmpconvert-help-2 COMMAND yabmpconvert -hv)
set_tests_properties(yabmpconvert-help-2 PROPERTIES PASS_REGULAR_EXPRESSION "^$<TARGET_FILE_NAME:yabmpconvert> ${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}\nusage")
# error
add_test(NAME yabmpconvert-error-1 COMMAND yabmpconvert)
set_tests_properties(yabmpconvert-error-1 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-2 COMMAND yabmpconvert -q)
set_tests_properties(yabmpconvert-error-2 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-3 COMMAND yabmpconvert -o)
set_tests_properties(yabmpconvert-error-3 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-4 COMMAND yabmpconvert -o dummy/directory/that/does/not/exist/file.txt)
set_tests_properties(yabmpconvert-error-4 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-5 COMMAND yabmpconvert -i)
set_tests_properties(yabmpconvert-error-5 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-6 COMMAND yabmpconvert -i dummy/directory/that/does/not/exist/file.txt)
set_tests_properties(yabmpconvert-error-6 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpconvert-error-7 COMMAND yabmpconvert -i - -o dummy/directory/that/does/not/exist/file.txt)
set_tests_properties(yabmpconvert-error-7 PROPERTIES WILL_FAIL TRUE ENVIRONMENT YABMP_USE_MEMORY_STREAM=1)
add_test(NAME yabmpconvert-error-8 COMMAND yabmpconvert -i dummy/directory/that/does/not/exist/file.txt -o -)
set_tests_properties(yabmpconvert-error-8 PROPERTIES WILL_FAIL TRUE ENVIRONMENT YABMP_USE_MEMORY_STREAM=1)

