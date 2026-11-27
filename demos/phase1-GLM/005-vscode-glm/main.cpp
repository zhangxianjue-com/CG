#include <iostream>
#include <glm/glm.hpp>

int main()
{
    glm::vec3 v1(1.0f, 2.0f, 3.0f);
    glm::vec3 v2(100.0f, 2.0f, 3.0f);
    std::cout << "v1 = (" << v1.x << ", " << v1.y << ", " << v1.z << ")\n";
    std::cout << "v2 = (" << v2.x << ", " << v2.y << ", " << v2.z << ")\n";
    std::cin.get();  // 等按一下回车再退出
    return 0;
}

