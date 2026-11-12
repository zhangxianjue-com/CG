#include "cg/math/vec3.h"

namespace cg {

// —— 成员函数 ——
Vec3 Vec3::operator-() const { return {-x, -y, -z}; }

Vec3& Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vec3& Vec3::operator*=(double t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
}

Vec3& Vec3::operator/=(double t) { return *this *= 1.0 / t; }

double Vec3::length() const { return std::sqrt(length_squared()); }

double Vec3::length_squared() const { return x * x + y * y + z * z; }

bool Vec3::near_zero() const { return length_squared() < 1e-16; }

// —— 自由函数 ——
Vec3 operator+(const Vec3& u, const Vec3& v) { return {u.x + v.x, u.y + v.y, u.z + v.z}; }

Vec3 operator-(const Vec3& u, const Vec3& v) { return {u.x - v.x, u.y - v.y, u.z - v.z}; }

Vec3 operator*(const Vec3& u, const Vec3& v) { return {u.x * v.x, u.y * v.y, u.z * v.z}; }

Vec3 operator*(double t, const Vec3& v) { return {t * v.x, t * v.y, t * v.z}; }

Vec3 operator*(const Vec3& v, double t) { return t * v; }

Vec3 operator/(const Vec3& v, double t) { return (1.0 / t) * v; }

double dot(const Vec3& u, const Vec3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }

Vec3 cross(const Vec3& u, const Vec3& v) {
    return {u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x};
}

Vec3 unit_vector(const Vec3& v) { return v / v.length(); }

std::ostream& operator<<(std::ostream& out, const Vec3& v) {
    return out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}

} // namespace cg
