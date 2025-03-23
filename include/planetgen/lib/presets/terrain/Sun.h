#pragma once

#include "planetgen/lib/Terrain.h"
#include "FastNoise/FastNoise.h"

namespace planet
{
class Sun : public Terrain
{
public:
    Sun()
    {
        emissive = true;
    }
    
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
            {0.334f, {1.000, 1.000, 1.000}},
            {0.445f, {0.988, 0.804, 0.016}},
            {0.642f, {0.988, 0.271, 0.016}},
            {0.667f, {0.988, 0.345, 0.016}},
            {0.778f, {0.988, 0.176, 0.016}},
            {0.889f, {0.988, 0.549, 0.016}},
        };
    }

    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnPerlin = FastNoise::New<FastNoise::Perlin>();
        auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal->SetSource(fnPerlin);
        fnFractal->SetGain(0.500f);
        fnFractal->SetWeightedStrength(0.500f);
        fnFractal->SetOctaveCount(3);
        fnFractal->SetLacunarity(2.000);
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnFractal);
        fnScale->SetScale(12.0f);
        auto fnSeed = FastNoise::New<FastNoise::SeedOffset>();
        fnSeed->SetSource(fnScale);
        fnSeed->SetOffset(1);
        auto fnFade = FastNoise::New<FastNoise::MaxSmooth>();
        fnFade->SetLHS(fnSeed);
        fnFade->SetRHS(fnScale);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        fnFade->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);

        return output;
    }
};
}
