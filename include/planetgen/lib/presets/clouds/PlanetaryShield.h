#pragma once

#include "planetgen/lib/Clouds.h"

namespace planet
{
class PlanetaryShield : public Clouds
{
public:
    glm::vec3 GetColor() const override
    {
        return {0.0f, 1.0f, 1.0f};
    }
    
    void normalizeValues(std::vector<float>& data, const FastNoise::OutputMinMax& minmax)
    {
        for (float& value : data)
        {
            value = 0.5f + ((value - minmax.min) / (minmax.max - minmax.min));
        }
    }

    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnPerlin = FastNoise::New<FastNoise::CellularDistance>();
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnPerlin);
        fnScale->SetScale(5.0f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        auto minmax = fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        normalizeValues(output, minmax);

        return output;
    }
};
}
