add_executable(yabmpunit "${CMAKE_CURRENT_SOURCE_DIR}/yabmpunit.c" )
target_link_libraries(yabmpunit ${YABMP_LIBRARY_NAME})

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET yabmpunit POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:yabmpunit>"
  COMMENT "dsymutil $<TARGET_FILE:yabmpunit>"
  DEPENDS yabmpunit)
endif()

# batch unit tests
add_test(NAME yabmpunit COMMAND yabmpunit)
