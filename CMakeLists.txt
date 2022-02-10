#指定最小版本号，高版本cmake会自动降级到该版本，从而实现向后兼容
cmake_minimum_required(VERSION 3.2)

#定义解决方案
project(MyApps
    LANGUAGES C CXX #定义编程语言
)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory("src/Win32Helper")
# add_subdirectory("src/系统托盘")
# add_subdirectory("src/DLL测试")
add_subdirectory("src/窗口隐藏器")
