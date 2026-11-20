cmake_minimum_required(VERSION 3.24)
project(phase1_glm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLM（OpenGL Mathematics，纯头文件库）——configure 时自动从 GitHub 下载
include(FetchContent)
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glm)

add_executable(glm_demo main.cpp)
target_include_directories(glm_demo PRIVATE ${glm_SOURCE_DIR})

# 源码含中文字面量，MSVC 需按 UTF-8 解析
if(MSVC)
  target_compile_options(glm_demo PRIVATE /utf-8)
endif()
