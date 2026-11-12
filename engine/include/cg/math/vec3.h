#pragma once

#include <cmath>
#include <ostream>

namespace cg {

// 三维向量：图形学数学的最小单元
// 阶段 0 目标：吃透向量的几何意义（方向 + 长度）
// 设计参考 Ray Tracing in One Weekend 的 vec3，保持最小可用形态，
// 随机数、反射/折射等函数留到对应阶段用到时再加入。
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    // —— 运算与赋值 ——
    Vec3 operator-() const;        // 反向向量
    Vec3& operator+=(const Vec3& v);
    Vec3& operator*=(double t);
    Vec3& operator/=(double t);

    // —— 度量 ——
    double length() const;
    double length_squared() const; // 避免开方，比较距离时优先使用
    bool near_zero() const;        // 浮点误差下的"零向量"判断

    // —— 常用静态向量 ——
    static Vec3 zero() { return {}; }
    static Vec3 axis_x() { return {1.0, 0.0, 0.0}; }
    static Vec3 axis_y() { return {0.0, 1.0, 0.0}; }
    static Vec3 axis_z() { return {0.0, 0.0, 1.0}; }
};

// —— 自由函数：向量运算 ——
Vec3 operator+(const Vec3& u, const Vec3& v);
Vec3 operator-(const Vec3& u, const Vec3& v);
Vec3 operator*(const Vec3& u, const Vec3& v); // 逐分量相乘（颜色调制等用）
Vec3 operator*(double t, const Vec3& v);
Vec3 operator*(const Vec3& v, double t);
Vec3 operator/(const Vec3& v, double t);

double dot(const Vec3& u, const Vec3& v);
Vec3 cross(const Vec3& u, const Vec3& v);
Vec3 unit_vector(const Vec3& v);              // 归一化（零向量会除零，见笔记踩坑记录）

std::ostream& operator<<(std::ostream& out, const Vec3& v);

} // namespace cg
