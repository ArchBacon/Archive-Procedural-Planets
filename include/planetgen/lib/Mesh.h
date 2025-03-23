#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace planet
{
struct MeshConfig
{
    float radius = 1.0f;    // Size of the sphere
    int stacks = 32;        // Minimum of 2, default of 32
    int sectors = 64;       // Minimum of 3, default of 64
    bool inverted = false;
    glm::vec3 offset = glm::vec3(0.0f); // Location in world space
};

struct Mesh
{
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec2> uvs{};
    std::vector<uint16_t> indices{};
};
}
