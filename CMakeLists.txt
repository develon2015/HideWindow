#指定最小版本号，高版本cmake会自动降级到该版本，从而实现向后兼容
cmake_minimum_required(VERSION 3.2)

#定义解决方案
project(MyApps
    LANGUAGES C CXX #定义编程语言
)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
# add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:gbk>") #使用系统默认代码页GBK来兼容xxA函数
# add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>") #init_locale("zh_CN.UTF-8")后可输出%ls格式和%s格式,但是xxA函数将乱码
#由于调试信息只在Debug时关注,因此我们在Release时将执行编码恢复为gbk
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:$<$<CONFIG:Debug>:utf-8>$<$<CONFIG:Release>:gbk>>")

#定义如何链接VC运行时
add_compile_options($<$<CONFIG:Debug>:/MDd>) #MSVCRTD.lib
add_compile_options($<$<CONFIG:Release>:/MT>) #LIBCMT.lib

if(WIN32)
    add_compile_options(/W4) #定义警告级别
    add_compile_options(/wd4819) #禁用代码页警告
endif()

add_subdirectory("src/Win32Helper")
# add_subdirectory("src/系统托盘")
# add_subdirectory("src/DLL测试")
add_subdirectory("src/FloatWindow")
add_subdirectory("src/窗口隐藏器")
