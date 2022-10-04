# CMakeList.txt: FloatWindow 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
project ("FloatWindow")

# 将源代码添加到此项目的可执行文件。
# add_executable (FloatWindow "FloatWindow.cpp" "FloatWindow.h")
add_library (fw STATIC  "Main.cpp")

# TODO: 如有需要，请添加测试并安装目标。

target_include_directories(fw PUBLIC "${CMAKE_HOME_DIRECTORY}/src")
