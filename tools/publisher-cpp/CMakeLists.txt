cmake_minimum_required(VERSION 3.24)
project(cgpublish LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FTXUI（终端 UI 库）
include(FetchContent)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v6.1.9
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(ftxui)

# 目标名用 ASCII（CMake 目标名不允许空格），输出文件名另设
add_executable(yushu_project_management
  src/main.cpp
  src/engine.cpp
  src/cli.cpp
  src/tui.cpp)
set_target_properties(yushu_project_management PROPERTIES OUTPUT_NAME "yushu-project-management")

target_include_directories(yushu_project_management PRIVATE src)

# 源码含中文字面量，MSVC 需按 UTF-8 解析
if(MSVC)
  target_compile_options(yushu_project_management PRIVATE /utf-8)
endif()

target_link_libraries(yushu_project_management PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  winhttp   # VerifyAsync 的 HTTP 请求（Windows SDK 自带）
  shell32   # ShellExecuteW 打开站点
  ole32     # IFileOpenDialog 文件夹选择框
)
