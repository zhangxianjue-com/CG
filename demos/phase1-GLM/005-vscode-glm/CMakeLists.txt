cmake_minimum_required(VERSION 3.24)

project(vscode-glm LANGUAGES CXX)

include(FetchContent)                           # 引入模块
FetchContent_Declare(glm                        # 声明：仓库 + 锁定版本
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE)                            # 浅克隆：只取 tag 那一次提交，不拉全历史
FetchContent_MakeAvailable(glm)                 # 下载并纳入构建
   
add_executable(vscode-glm main.cpp)

target_link_libraries(vscode-glm PRIVATE glm::glm) # 链接：目标名是第 11 行的 vscode-glm

# 源文件是 UTF-8（中文注释/字符串）：MSVC 默认按系统代码页 GBK 解释会报 C4819
if(MSVC)
    target_compile_options(vscode-glm PRIVATE /utf-8)
endif()


