// demo 01_math：向量运算验证
// 对应笔记：notes/02-math/vector-matrix.excalidraw
// 目标：亲手验证向量运算的几何性质，建立对实现的信任

#include <cg/cg.h>

#include <cmath>
#include <iostream>

namespace {

int g_failed = 0;

void check(const char* name, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << '\n';
    if (!ok) ++g_failed;
}

bool approx(double a, double b, double eps = 1e-12) {
    return std::fabs(a - b) < eps;
}

} // namespace

int main() {
    using cg::Vec3;

    std::cout << "== 01_math: 向量运算验证 ==\n\n";

    // 1. 长度与归一化：unit_vector 的结果长度应为 1
    {
        Vec3 v{3.0, 4.0, 0.0};
        check("|(3,4,0)| == 5", approx(v.length(), 5.0));

        Vec3 u = cg::unit_vector(v);
        check("unit_vector 长度为 1", approx(u.length(), 1.0));
        check("归一化不改变方向", cg::dot(u, v) > 0.0);
    }

    // 2. 点积的几何意义：u·v = |u||v|cosθ；垂直向量点积为 0
    {
        check("正交向量点积为 0",
              approx(cg::dot(Vec3::axis_x(), Vec3::axis_y()), 0.0));

        // (1,0,0) 与 (1,1,0) 夹角 45°，cos45° = sqrt(2)/2
        double cos_theta = cg::dot(cg::unit_vector(Vec3::axis_x()),
                                   cg::unit_vector({1.0, 1.0, 0.0}));
        check("45° 夹角: cos == sqrt(2)/2", approx(cos_theta, std::sqrt(2.0) / 2.0));
    }

    // 3. 叉积：结果同时垂直于两个操作数；不满足交换律（反交换律）
    {
        Vec3 c = cg::cross(Vec3::axis_x(), Vec3::axis_y());
        check("x × y == z", c.near_zero() == false &&
                            approx(c.x, 0.0) && approx(c.y, 0.0) && approx(c.z, 1.0));

        Vec3 anti = cg::cross(Vec3::axis_y(), Vec3::axis_x());
        check("反交换律: x×y + y×x ≈ 0", (c + anti).near_zero());

        check("叉积结果垂直于两个操作数",
              approx(cg::dot(c, Vec3::axis_x()), 0.0) &&
              approx(cg::dot(c, Vec3::axis_y()), 0.0));
    }

    // 4. 运算律抽查
    {
        Vec3 a{1.0, 2.0, 3.0};
        Vec3 b{4.0, -5.0, 6.0};
        double t = 2.5;
        check("分配律: t*(a+b) == t*a + t*b",
              (t * (a + b) - (t * a + t * b)).near_zero());
        check("结合律: (a+b)+(-a) ≈ b", ((a + b) + (-a) - b).near_zero());
    }

    std::cout << '\n' << (g_failed == 0 ? "全部通过" : "有检查未通过") << '\n';
    return g_failed == 0 ? 0 : 1;
}
