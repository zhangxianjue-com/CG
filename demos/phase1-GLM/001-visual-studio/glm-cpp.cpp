
#include <iostream>

#include <glm/glm.hpp>

int main()
{
    // 一个 3D 向量：x=1, y=2, z=3
    glm::vec3 v(1.0f, 2.0f, 3.0f);

    std::cout << "3D 向量 v = (" << v.x << ", " << v.y << ", " << v.z << ")\n";
    std::cout << "v 的长度 = " << glm::length(v) << '\n';
}

