#pragma once

#include "Texture.h"

namespace planet
{
class Terrain : public Texture
{
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override = 0;
    virtual std::vector<std::pair<float, glm::vec3>> GetColors() const { return {}; };
};
}
