message(STATUS "当前目录: ${CMAKE_HOME_DIRECTORY}")
file(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
message(STATUS "源码: ${files}")

set(App "hide")
add_executable("${App}" ${files})

target_include_directories("${App}" PUBLIC "${CMAKE_HOME_DIRECTORY}/src")
target_link_libraries("${App}" comctl32)
target_link_libraries("${App}" Win32Helper)

#指定链接器语言
set_target_properties("${App}" PROPERTIES LINKER_LANGUAGE CXX)

#定义入口点和子系统
set_target_properties("${App}" PROPERTIES LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\"")
set_target_properties("${App}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
