// demo phase1-GLM：GLM 数学库入门
// 对应笔记：docs/notes/01-cpp/glm.md
// 目标：亲手验证 GLM 的向量/矩阵运算与坐标变换，建立对 3D 数学实现的信任

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <iostream>

namespace {

int g_failed = 0;

void check(const char* name, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (!ok) g_failed++;
}

// 浮点近似相等（避免 == 的精度陷阱）
bool near(glm::vec3 a, glm::vec3 b, float eps = 1e-5f) {
    return glm::all(glm::lessThan(glm::abs(a - b), glm::vec3(eps)));
}

}  // namespace

int main() {
    // ---------- ① 向量基础运算 ----------
    const glm::vec3 v{3.0f, 4.0f, 0.0f};
    check("length(v) == 5", std::fabs(glm::length(v) - 5.0f) < 1e-6f);
    check("normalize 后长度为 1", std::fabs(glm::length(glm::normalize(v)) - 1.0f) < 1e-6f);

    const glm::vec3 a{1.0f, 0.0f, 0.0f};
    const glm::vec3 b{0.0f, 1.0f, 0.0f};
    check("dot(a,b) == 0（垂直）", glm::dot(a, b) == 0.0f);
    check("cross(a,b) == (0,0,1)", near(glm::cross(a, b), glm::vec3(0.0f, 0.0f, 1.0f)));
    check("cross 反对称", near(glm::cross(a, b), -glm::cross(b, a)));

    // ---------- ② 矩阵变换 ----------
    // 单位矩阵
    const glm::mat4 I(1.0f);
    check("I * v == v", near(glm::vec3(I * glm::vec4(v, 1.0f)), v));

    // 平移（z 方向移动 2）
    const glm::mat4 T = glm::translate(I, glm::vec3(0.0f, 0.0f, 2.0f));
    check("平移 z+2", near(glm::vec3(T * glm::vec4(v, 1.0f)), glm::vec3(3.0f, 4.0f, 2.0f)));

    // 旋转：绕 z 轴 90°，x 轴 → y 轴
    const glm::mat4 R = glm::rotate(I, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    check("绕 z 转 90°：x → y", near(glm::vec3(R * glm::vec4(a, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f)));

    // 缩放
    const glm::mat4 S = glm::scale(I, glm::vec3(2.0f, 2.0f, 2.0f));
    check("缩放 2 倍", near(glm::vec3(S * glm::vec4(v, 1.0f)), glm::vec3(6.0f, 8.0f, 0.0f)));

    // 复合变换：先缩放、再旋转、最后平移（矩阵左乘顺序 = 应用顺序）
    // M = T * R * S，顶点 v' = M * v：先应用 S，再 R，最后 T
    const glm::mat4 M = T * R * S;
    const glm::vec3 p{1.0f, 0.0f, 0.0f};
    const glm::vec3 expected = glm::vec3(T * R * glm::vec4(glm::vec3(S * glm::vec4(p, 1.0f)), 1.0f));
    check("复合变换 M = T*R*S", near(glm::vec3(M * glm::vec4(p, 1.0f)), expected));

    // 逆矩阵：M 的逆把变换后的点变回原点
    const glm::vec3 p2 = glm::vec3(M * glm::vec4(p, 1.0f));
    check("M^-1 还原顶点", near(glm::vec3(glm::inverse(M) * glm::vec4(p2, 1.0f)), p));

    // ---------- ③ 相机变换（view + projection）----------
    // lookAt：相机在 (0,0,5) 看向原点，up 为 +y
    const glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
                                       glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));
    // 相机位置经 view 变换后应在原点（相机在自己的坐标系原点）
    check("lookAt 把相机移到原点",
          near(glm::vec3(view * glm::vec4(0.0f, 0.0f, 5.0f, 1.0f)), glm::vec3(0.0f)));

    // 透视投影：45° FOV、16:9、近 0.1 远 100
    const glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    // MVP：把世界坐标顶点变换到裁剪空间
    const glm::mat4 mvp = proj * view * M;
    // 注意：点必须在视锥内，NDC 才会落在 [-1,1]（原 p=(1,0,0) 经缩放旋转后超出视锥）
    const glm::vec4 clip = mvp * glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
    // 透视除法得到 NDC，应在 [-1,1] 内
    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    check("MVP 后 NDC 在 [-1,1]（视锥内）",
          glm::all(glm::greaterThanEqual(ndc, glm::vec3(-1.0f))) &&
          glm::all(glm::lessThanEqual(ndc, glm::vec3(1.0f))));

    std::cout << (g_failed == 0 ? "\n全部通过 ✓\n" : "\n有失败 ✗\n");
    return g_failed;
}
