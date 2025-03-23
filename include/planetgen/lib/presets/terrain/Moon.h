#pragma once

#include "planetgen/lib/Terrain.h"

namespace planet
{
class Moon : public Terrain
{
public:
    Moon()
    {
        SetEmissive(true);
    }
    
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
            {0.000f,{1.000,1.000,1.000}},
            {0.664f,{0.245,0.245,0.245}},
            {1.000f,{0.000,0.000,0.000}},
        };
    }
    
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
        auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal->SetSource(fnSimplex);
        fnFractal->SetGain(0.650f);
        fnFractal->SetWeightedStrength(0.500f);
        fnFractal->SetOctaveCount(4);
        fnFractal->SetLacunarity(2.500);
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnFractal);
        fnScale->SetScale(0.8f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        
        return output;
    }

    
};
}
