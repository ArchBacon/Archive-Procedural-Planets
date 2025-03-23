#pragma once

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "tools/tools.hpp"

namespace planet
{
struct SphericalCoordinates
{
    std::vector<float> x, y, z;

    SphericalCoordinates() = default;

    SphericalCoordinates(const int size)
    {
        x.resize(size);
        y.resize(size);
        z.resize(size);
    }
};

class Sphere
{
    // Resolution as the key (int)
    static std::unordered_map<int, SphericalCoordinates> coords;

public:
    static Mesh UV(float radius = 1.0f, int stacks = 16, int sectors = 32, bool inverted = false);
    static SphericalCoordinates GetSphericalCoordinates(float radius = 1.0f, int resolution = 256, glm::vec3 offset = glm::vec3(0.0f));

private:
    static void CalculateSphericalCoordinates(int resolution);
};
}
