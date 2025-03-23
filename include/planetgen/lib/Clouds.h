#pragma once

#include "Texture.h"

namespace planet
{
class Clouds : public Texture
{
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override = 0;
    virtual glm::vec3 GetColor() const { return glm::vec3(1.0f); }
};
}
