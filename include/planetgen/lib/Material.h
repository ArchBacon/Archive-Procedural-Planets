#pragma once
#include <vector>

namespace planet
{
struct Material
{
    int resolution = 256;
    int channels = 4;
    std::vector<unsigned char> albedo{};
    std::vector<unsigned char> emissive{};
    std::vector<unsigned char> normal{};
    std::vector<unsigned char> occlusion{};
    std::vector<unsigned char> metallicRoughness{};
};
}
