
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

set(YABMPCONVERT "yabmpconvert")

add_executable(${YABMPCONVERT} yabmpconvert.c yabmpconvert_topng.c ../common/yabmp_printinfo.c)
target_link_libraries(${YABMPCONVERT} ${YABMP_LIBRARY_NAME} ${PNG_LIBRARIES} optparse)

if(YABMP_USE_DSYMUTIL)
  add_custom_command(TARGET ${YABMPCONVERT} POST_BUILD 
  COMMAND "dsymutil" "$<TARGET_FILE:${YABMPCONVERT}>"
  COMMENT "dsymutil $<TARGET_FILE:${YABMPCONVERT}>"
  DEPENDS ${YABMPCONVERT})
endif()
