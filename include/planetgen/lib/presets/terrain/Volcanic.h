#pragma once

#include "planetgen/lib/Terrain.h"

namespace planet
{
class Volcanic : public Terrain
{
    std::vector<std::pair<float, glm::vec3>> GetColors() const override
    {
        return {
                {0.054f,{0.802, 0.071, 0.071}},
                {0.389f,{0.570, 0.120, 0.518}},
                {0.538f,{0.219, 0.051, 0.241}},
                {0.630f,{0.096, 0.057, 0.447}},
                {1.000f,{0.321, 0.081, 0.020}},
            };
    }
    
public:
    std::vector<float> GetNoiseData(glm::vec3 position) override
    {
        auto fnCellular = FastNoise::New<FastNoise::CellularDistance>();
        fnCellular->SetJitterModifier(1.360f);
        fnCellular->SetReturnType(FastNoise::CellularDistance::ReturnType::Index0Add1);
        auto fnPingPong = FastNoise::New<FastNoise::FractalPingPong>();
        fnPingPong->SetSource(fnCellular);
        fnPingPong->SetGain(0.500f);
        fnPingPong->SetWeightedStrength(0.000f);
        fnPingPong->SetPingPongStrength(2.640f);
        fnPingPong->SetOctaveCount(3);
        fnPingPong->SetLacunarity(2.000);
        auto fnConstant = FastNoise::New<FastNoise::Constant>();
        fnConstant->SetValue(-1.0f);
        auto fnMax = FastNoise::New<FastNoise::Max>();
        fnMax->SetLHS(fnPingPong);
        fnMax->SetRHS(fnConstant);
        auto fnTerrace = FastNoise::New<FastNoise::Terrace>();
        fnTerrace->SetSource(fnMax);
        fnTerrace->SetMultiplier(1.5f);
        fnTerrace->SetSmoothness(-0.06f);
        auto fnScale = FastNoise::New<FastNoise::DomainScale>();
        fnScale->SetSource(fnTerrace);
        fnScale->SetScale(1.0f);

        const int size = resolution * resolution;
        std::vector<float> output(size);

        const auto coords = GetSphericalCoordinates();
        fnScale->GenPositionArray3D(output.data(), size, coords.x.data(), coords.y.data(), coords.z.data(), 0, 0, 0, seed);
        
        return output;
    }
};
}
