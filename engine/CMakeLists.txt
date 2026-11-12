# 渲染引擎库：所有 demo 共享代码的沉淀处
# 规则：demo 中出现两次以上且稳定的代码，下沉到这里
add_library(cg STATIC
    src/math/vec3.cpp
)

target_include_directories(cg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(cg PUBLIC cxx_std_20)
