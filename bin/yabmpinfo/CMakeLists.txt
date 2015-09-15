set(YABMPINFO "yabmpinfo")

add_executable(${YABMPINFO} yabmpinfo.c ${CMAKE_CURRENT_SOURCE_DIR}/../common/yabmp_printinfo.c )
target_link_libraries(${YABMPINFO} ${YABMP_LIBRARY_NAME} optparse)

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET ${YABMPINFO} POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:${YABMPINFO}>"
  COMMENT "dsymutil $<TARGET_FILE:${YABMPINFO}>"
  DEPENDS ${YABMPINFO})
endif()
