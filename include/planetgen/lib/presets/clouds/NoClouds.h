#pragma once

#include "planetgen/lib/Clouds.h"

namespace planet
{
class NoClouds : public Clouds
{
public:
    NoClouds() = default;

    std::vector<float> GetNoiseData(glm::vec3 position) override { return {}; }
};
}
