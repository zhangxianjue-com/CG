cmake_minimum_required(VERSION 3.24)   # 最低 CMake 版本
   project(hello LANGUAGES CXX)           # 工程名 + C++ 语言
   add_executable(hello main.cpp)         # 生成 hello.exe