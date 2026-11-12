cmake_minimum_required(VERSION 3.24)

project(CG VERSION 0.1.0 LANGUAGES CXX)

# 学习项目：警告全开，尽早暴露问题
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra)
endif()

# 统一输出目录：所有可执行文件输出到 <build>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(engine)
add_subdirectory(demos)
