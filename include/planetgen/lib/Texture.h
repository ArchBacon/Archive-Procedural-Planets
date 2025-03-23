#pragma once

#include <FastNoise/FastNoise.h>
#include <vector>
#include <glm/glm.hpp>
#include "planetgen/lib/Sphere.h"

namespace planet
{
struct SphericalCoordinates;

class Texture
{
    friend class Planet;
    
public:
    Texture() = default;
    virtual ~Texture() = default;

    virtual std::vector<float> GetNoiseData(glm::vec3 offset) = 0;

    int GetSeed() const { return seed; }
    void SetSeed(const int newSeed) { seed = newSeed; }

    int GetTextureResolution() const { return resolution; }
    void SetTextureResolution(const int newResolution) { resolution = newResolution; }

    bool IsEmissive() const { return emissive; }
    void SetEmissive(bool isEmissive) { emissive = isEmissive; }

protected:
    int seed = 1337;
    int resolution = 1024;
    bool emissive = false;

private:
    float radius = 1.0f;
    glm::vec3 offset{0.0f};

protected:
    [[nodiscard]] SphericalCoordinates GetSphericalCoordinates() const
    {
        return Sphere::GetSphericalCoordinates(radius, resolution, offset);
    }    
};
}
