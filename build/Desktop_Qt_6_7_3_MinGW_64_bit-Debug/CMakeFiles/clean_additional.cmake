# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\NDIQtBackend_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\NDIQtBackend_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\StreamNDI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\StreamNDI_autogen.dir\\ParseCache.txt"
  "NDIQtBackend_autogen"
  "StreamNDI_autogen"
  )
endif()
