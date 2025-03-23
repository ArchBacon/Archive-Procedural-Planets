#pragma once

#include "planetgen/lib/Clouds.h"

namespace planet
{
class CirrusClouds : public Clouds
{
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        const auto fnSimplex2 = FastNoise::New<FastNoise::OpenSimplex2>();
        const auto fnFractal = FastNoise::New<FastNoise::FractalRidged>();
        fnFractal->SetSource(fnSimplex2);
        fnFractal->SetGain(0.500f);
        fnFractal->SetWeightedStrength(0.000f);
        fnFractal->SetOctaveCount(5);
        fnFractal->SetLacunarity(2.000);
        const auto fnOffset = FastNoise::New<FastNoise::DomainOffset>();
        fnOffset->SetSource(fnFractal);
        fnOffset->SetOffset<FastNoise::Dim::X>(position.x);
        fnOffset->SetOffset<FastNoise::Dim::Y>(position.y);
        fnOffset->SetOffset<FastNoise::Dim::Z>(position.z);
        const auto fnDomainWarp = FastNoise::New<FastNoise::DomainWarpGradient>();
        fnDomainWarp->SetSource(fnOffset);
        fnDomainWarp->SetWarpAmplitude(1.160f);
        fnDomainWarp->SetWarpFrequency(0.720f);
        const auto fnTerrace = FastNoise::New<FastNoise::Terrace>();
        fnTerrace->SetSource(fnDomainWarp);
        fnTerrace->SetMultiplier(1.0f);
        fnTerrace->SetSmoothness(1.220f);
        const auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnTerrace);
        fnScale->SetScale(1.2f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);

        return output;
    }
};
}
