#pragma once

#include "planetgen/lib/Clouds.h"

namespace planet
{
class DenseClouded : public Clouds
{
public:
    void normalizeValues(std::vector<float>& data, const FastNoise::OutputMinMax& minmax)
    {
        for (float& value : data)
        {
            value = 1.5f * ((value - minmax.min) / (minmax.max - minmax.min));
        } 
    }
    
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnSimplex2 = FastNoise::New<FastNoise::OpenSimplex2>();
        auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
        fnFractal->SetSource(fnSimplex2);
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
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnFractal3);
        fnScale->SetScale(1.0f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        auto minmax = fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        normalizeValues(output, minmax);
        
        return output;
    }
};
}
