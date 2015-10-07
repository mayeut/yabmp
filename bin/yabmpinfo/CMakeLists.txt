add_executable(yabmpinfo "${CMAKE_CURRENT_SOURCE_DIR}/yabmpinfo.c" "${CMAKE_CURRENT_SOURCE_DIR}/../common/yabmp_printinfo.c" )
target_link_libraries(yabmpinfo ${YABMP_LIBRARY_NAME} optparse)

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET yabmpinfo POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:yabmpinfo>"
  COMMENT "dsymutil $<TARGET_FILE:yabmpinfo>"
  DEPENDS yabmpinfo)
endif()

# -v exit code 0
add_test(NAME yabmpinfo-version-1 COMMAND yabmpinfo -v)
# -vo - exit code 0
add_test(NAME yabmpinfo-version-2 COMMAND yabmpinfo -vo -)
# --version prints correct version
add_test(NAME yabmpinfo-version-3 COMMAND yabmpinfo --version)
set_tests_properties(yabmpinfo-version-3 PROPERTIES PASS_REGULAR_EXPRESSION "^$<TARGET_FILE_NAME:yabmpinfo> ${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}\n$")
# --help exit code 0
add_test(NAME yabmpinfo-help-1 COMMAND yabmpinfo --help)
# -hv prints correct version & help
add_test(NAME yabmpinfo-help-2 COMMAND yabmpinfo -hv)
set_tests_properties(yabmpinfo-help-2 PROPERTIES PASS_REGULAR_EXPRESSION "^$<TARGET_FILE_NAME:yabmpinfo> ${YABMP_VERSION_MAJOR}.${YABMP_VERSION_MINOR}.${YABMP_VERSION_PATCH}\nusage")
# error
add_test(NAME yabmpinfo-error-1 COMMAND yabmpinfo)
set_tests_properties(yabmpinfo-error-1 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpinfo-error-2 COMMAND yabmpinfo -q)
set_tests_properties(yabmpinfo-error-2 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpinfo-error-3 COMMAND yabmpinfo -o)
set_tests_properties(yabmpinfo-error-3 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpinfo-error-4 COMMAND yabmpinfo -o dummy/directory/that/does/not/exist/file.txt)
set_tests_properties(yabmpinfo-error-4 PROPERTIES WILL_FAIL TRUE)
add_test(NAME yabmpinfo-error-5 COMMAND yabmpinfo dummy/directory/that/does/not/exist/file.txt)
set_tests_properties(yabmpinfo-error-5 PROPERTIES WILL_FAIL TRUE)
