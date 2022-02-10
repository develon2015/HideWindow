file(GLOB_RECURSE files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.c)
message(STATUS "构建Win32Helper: ${files}")
add_library(Win32Helper STATIC ${files})
