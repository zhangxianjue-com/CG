cmake_minimum_required(VERSION 3.24)

project(glm-demo LANGUAGES CXX)

# GLM 由 vcpkg 提供（见 vcpkg.json，configure 时自动安装）
# 使用 find_package 而非 FetchContent：依赖在构建时由 vcpkg 拉取、缓存
find_package(glm CONFIG REQUIRED)

add_executable(glm-cpp glm-cpp.cpp)

target_link_libraries(glm-cpp PRIVATE glm::glm)

# 源文件是 UTF-8（中文注释/字符串）：MSVC 默认按系统代码页 GBK 解释会乱码
if(MSVC)
    target_compile_options(glm-cpp PRIVATE /utf-8)
endif()
