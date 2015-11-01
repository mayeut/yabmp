add_executable(yabmpunit "${CMAKE_CURRENT_SOURCE_DIR}/yabmpunit.c" )
target_link_libraries(yabmpunit ${YABMP_LIBRARY_NAME})

add_executable(yabmpmessageunit "${CMAKE_CURRENT_SOURCE_DIR}/yabmpmessageunit.c" ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/yabmp/src/yabmp_message.c)
target_include_directories(yabmpmessageunit PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../lib/yabmp/inc/private ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/yabmp/inc/private)

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET yabmpunit POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:yabmpunit>"
  COMMENT "dsymutil $<TARGET_FILE:yabmpunit>"
  DEPENDS yabmpunit)
  add_custom_command(TARGET yabmpmessageunit POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:yabmpmessageunit>"
  COMMENT "dsymutil $<TARGET_FILE:yabmpmessageunit>"
  DEPENDS yabmpmessageunit)
endif()

# batch unit tests
add_test(NAME yabmpunit COMMAND yabmpunit)
add_test(NAME yabmpmessageunit COMMAND yabmpmessageunit)
