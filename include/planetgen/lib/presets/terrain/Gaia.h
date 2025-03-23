#pragma once

#include "planetgen/lib/Terrain.h"

namespace planet
{
class Gaia : public Terrain
{
public:
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
            {0.495f,{0.110,0.318,0.792}},    // Dark water
            {0.540f,{0.714,0.890,0.859}},    // Shallow water
            {0.570f,{0.898,0.851,0.761}},    // Sand
            {0.610f,{0.447,0.329,0.157}},    // Grass
            {0.650f,{0.710,0.729,0.380}},    // Forest
            {1.000f,{0.486,0.553,0.298}},    // Darker forest
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
