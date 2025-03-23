#pragma once

#include "planetgen/lib/Terrain.h"

namespace planet
{
class Barren : public Terrain
{
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
            {0.000f,{0.855, 0.745, 0.005}},
            {0.194f,{0.492, 0.079, 0.079}},
            {0.489f,{0.365, 0.207, 0.000}},
            {0.991f,{0.695, 0.378, 0.000}},
        };
    }
    
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnSimplex = FastNoise::New<FastNoise::CellularDistance>();
        auto fnFractal = FastNoise::New<FastNoise::FractalRidged>();
        fnFractal->SetSource(fnSimplex);
        fnFractal->SetGain(2.000f);
        fnFractal->SetWeightedStrength(0.000f);
        fnFractal->SetOctaveCount(2);
        fnFractal->SetLacunarity(2.500);
        auto fnScale = FastNoise::New<FastNoise::FractalPingPong>();
        fnScale->SetSource(fnFractal);
        fnScale->SetGain(0.500f);
        fnScale->SetWeightedStrength(0.000f);
        fnScale->SetPingPongStrength(2.000f);
        fnScale->SetOctaveCount(3);
        fnScale->SetLacunarity(2.000);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        
        return output;
    }
};
}
