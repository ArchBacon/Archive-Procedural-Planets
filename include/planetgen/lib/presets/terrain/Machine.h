#pragma once

#include "planetgen/lib/Terrain.h"

namespace planet
{
class Machine : public Terrain
{
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
                {0.000f,{0.000, 0.000, 0.000}},
                {1.000f,{1.000, 1.000, 1.000}},
            };
    }
    
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnPerlin = FastNoise::New<FastNoise::OpenSimplex2>();
        auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal->SetSource(fnPerlin);
        fnFractal->SetGain(0.500f);
        fnFractal->SetWeightedStrength(0.000f);
        fnFractal->SetOctaveCount(3);
        fnFractal->SetLacunarity(2.000);
        auto fnFractal2 = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal2->SetSource(fnFractal);
        fnFractal2->SetGain(0.500f);
        fnFractal2->SetWeightedStrength(0.000f);
        fnFractal2->SetOctaveCount(3);
        fnFractal2->SetLacunarity(2.000);
        auto fnFractal3 = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal3->SetSource(fnFractal2);
        fnFractal3->SetGain(0.500f);
        fnFractal3->SetWeightedStrength(0.000f);
        fnFractal3->SetOctaveCount(3);
        fnFractal3->SetLacunarity(2.000);
        auto fnWarp = FastNoise::New<FastNoise::CellularLookup>();
        fnWarp->SetLookup(fnFractal3);
        fnWarp->SetJitterModifier(5.5f);
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnWarp);
        fnScale->SetScale(5.0f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        auto minmax = fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        // normalizeValues(output, minmax);

        return output;
    }
};
}
