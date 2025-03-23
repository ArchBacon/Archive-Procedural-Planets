#include "planetgen/lib/Planet.h"

#include <tinygltf/stb_image_write.h>

#include "math/math.hpp"
#include "omp.h"

planet::Planet::Planet(Terrain* terrain, Clouds* clouds, MeshConfig* config)
    : terrain(terrain), clouds(clouds), config(config)
{
    terrainMesh = Sphere::UV(config->radius, config->stacks, config->sectors, config->inverted);
    cloudMesh = Sphere::UV(config->radius + 0.05f, config->stacks, config->sectors, config->inverted);

    terrain->offset = config->offset;
    terrain->radius = config->radius;
    clouds->offset = config->offset;
    clouds->radius = config->radius;

    terrainColorPalette = terrain->GetColors();
    cloudColor = clouds->GetColor();

    // Generate textures...
    GenerateTerrainMaterial();
    GenerateCloudsMaterial();
}

const planet::Material& planet::Planet::GetTerrainMaterial(const std::vector<std::pair<float, glm::vec3>>& colors)
{
    terrainColorPalette = colors;
    GenerateTerrainMaterial();

    return terrainMaterial;
}
const planet::Material& planet::Planet::GetCloudMaterial(const glm::vec3 color)
{
    cloudColor = color;
    GenerateCloudsMaterial();

    return cloudMaterial;
}
void planet::Planet::SetTerrain(Terrain* inTerrain)
{
    const auto offset = terrain->offset;
    const auto radius = terrain->radius;
    // const auto resolution = terrain->resolution;

    terrain = inTerrain;
    terrain->offset = offset;
    terrain->radius = radius;
    // terrain->resolution = resolution;
    terrainColorPalette = terrain->GetColors();

    GenerateTerrainMaterial();
}
void planet::Planet::SetClouds(Clouds* inClouds)
{
    const auto offset = clouds->offset;
    const auto radius = clouds->radius;
    // const auto resolution = clouds->resolution;

    clouds = inClouds;
    clouds->offset = offset;
    clouds->radius = radius;
    // clouds->resolution = resolution;
    cloudColor = clouds->GetColor();

    GenerateCloudsMaterial();
}

void planet::Planet::GenerateTerrainMaterial()
{
    const auto noise = terrain->GetNoiseData(config->offset);
    terrainMaterial.resolution = terrain->resolution;

    std::vector<unsigned char> albedo(terrain->resolution * terrain->resolution * 4);
    std::vector<unsigned char> normal(terrain->resolution * terrain->resolution * 4);
    std::vector<unsigned char> OcRoMa(terrain->resolution * terrain->resolution * 4);
    std::vector<unsigned char> emissive(terrain->resolution * terrain->resolution * 4);
    
    #pragma omp parallel for
    for (size_t i = 0; i < noise.size(); i++)
    {
        float noiseValue = (noise[i] + 1.0f) * 0.5f;
        glm::vec3 color = GetColorByHeight(noiseValue);
        albedo[i * 4 + 0] = (unsigned char)(255.f * color.r);
        albedo[i * 4 + 1] = (unsigned char)(255.f * color.g);
        albedo[i * 4 + 2] = (unsigned char)(255.f * color.b);
        albedo[i * 4 + 3] = 255;
    }
    terrainMaterial.albedo = albedo;

    float waterStrength = 3.f;
    float landStrength = 15.0f;
    auto height = terrain->resolution;
    auto width = terrain->resolution;
    float difference = (float)terrain->resolution / 256.f;
    #pragma omp parallel for
    for (int y = 0; y < terrain->resolution; ++y)
    {
        for (int x = 0; x < terrain->resolution; ++x)
        {
            float noiseValue = (noise[y * width + x] + 1.0f) * 0.5f;
            auto strength = waterStrength;
            if (noiseValue >= waterLevel)
            {
                strength = landStrength * difference;
            }

            // Use Sobel filter to generate normals from heightmap
            float tl = (noise[((y - 1 + height) % height) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f; //top left
            float t = (noise[((y - 1 + height) % height) * width + (x)] + 1.0f) * 0.5f; //top center
            float tr = (noise[((y - 1 + height) % height) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //top right

            float l = (noise[(y) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f;// center left
            float r = (noise[(y) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //center right

            float bl = (noise[((y + 1) % height) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f; //bottom left
            float b = (noise[((y + 1) % height) * width + (x)] + 1.0f) * 0.5f; //bottom center
            float br = (noise[((y + 1) % height) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //bottom right

            float dX = -((tr + 2.0f * r + br) - (tl + 2.0f * l + bl));
            float dY = -((bl + 2.0f * b + br) - (tl + 2.0f * t + tr));

            float dZ = 1.0f / strength;
            float len = sqrtf(dX * dX + dY * dY + dZ * dZ);
            dX /= len;
            dY /= len;
            dZ /= len;

            int index = (y * width + x) * 4;
            normal[index + 0] = (unsigned char)((dX * 0.5f + 0.5f) * 255.f);
            normal[index + 1] = (unsigned char)((dY * 0.5f + 0.5f) * 255.f);
            normal[index + 2] = 255;
            normal[index + 3] = 255;
        }
    }
    terrainMaterial.normal = normal;

    auto map = [](float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };

    #pragma omp parallel for
    for (size_t i = 0; i < noise.size(); i++)
    {
        // rgb = orm
        const float value = (noise[i] + 1.0f) * 0.5f;
        float threshold = waterLevel;
        unsigned char roughness;

        if (value <= threshold)
        {
            roughness = static_cast<unsigned char>(std::round(map(value, 0.0f, threshold, 255.0f, 80.0f)));
        }
        else
        {
            roughness = static_cast<unsigned char>(std::round(map(value, threshold, 1.0f, 255.0f, 0.0f)));
        }

        OcRoMa[i * 4 + 0] = 0;
        OcRoMa[i * 4 + 1] = roughness;
        OcRoMa[i * 4 + 2] = 0;
        OcRoMa[i * 4 + 3] = 255;
    }
    terrainMaterial.metallicRoughness = OcRoMa;
    if (terrain->IsEmissive())
    {
        terrainMaterial.emissive = albedo;
    }
    else
    {
        terrainMaterial.emissive.clear();
    }
    // TODO: Occlusion
}

void planet::Planet::GenerateCloudsMaterial()
{
    const auto noise = clouds->GetNoiseData(config->offset);
    std::vector<unsigned char> albedo(clouds->resolution * clouds->resolution * 4);
    std::vector<unsigned char> normal(clouds->resolution * clouds->resolution * 4);
    std::vector<unsigned char> OcRoMa(clouds->resolution * clouds->resolution * 4);
    #pragma omp parallel for
    for (size_t i = 0; i < noise.size(); i++)
    {
        const float height = (noise[i] + 1.0f) * 0.5f;
        auto color = GetCloudColorByHeight(height);

        albedo[i * 4 + 0] = (unsigned char)(255.f * color.r);
        albedo[i * 4 + 1] = (unsigned char)(255.f * color.g);
        albedo[i * 4 + 2] = (unsigned char)(255.f * color.b);
        albedo[i * 4 + 3] = (unsigned char)(255.f * glm::clamp(noise[i], 0.0f, 1.0f));
    }

    cloudMaterial.resolution = clouds->resolution;
    cloudMaterial.albedo = albedo;

    auto map = [](float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };

    #pragma omp parallel for
    for (size_t i = 0; i < noise.size(); i++)
    {
        // rgb = orm
        const float height = (noise[i] + 1.0f) * 0.5f;
        float threshold = 0.540f;
        unsigned char roughness;

        if (height <= threshold)
        {
            roughness = static_cast<unsigned char>(std::round(map(height, 0.0f, threshold, 255.0f, 80.0f)));
        }
        else
        {
            roughness = static_cast<unsigned char>(std::round(map(height, threshold, 1.0f, 255.0f, 0.0f)));
        }

        OcRoMa[i * 4 + 0] = 0;
        OcRoMa[i * 4 + 1] = roughness;
        OcRoMa[i * 4 + 2] = 0;
        OcRoMa[i * 4 + 3] = 255;
    }
    cloudMaterial.metallicRoughness = OcRoMa;

    if (noise.empty())
    {
        cloudMaterial.normal.clear();
        return;
    }

    int height = clouds->resolution;
    int width = clouds->resolution;
    float strength = 3.f;

    #pragma omp parallel for
    for (int y = 0; y < clouds->resolution; ++y)
    {
        for (int x = 0; x < clouds->resolution; ++x)
        {
            float tl = (noise[((y - 1 + height) % height) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f; //top left
            float t = (noise[((y - 1 + height) % height) * width + (x)] + 1.0f) * 0.5f; //top center
            float tr = (noise[((y - 1 + height) % height) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //top right

            float l = (noise[(y) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f;// center left
            float r = (noise[(y) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //center right

            float bl = (noise[((y + 1) % height) * width + ((x - 1 + width) % width)] + 1.0f) * 0.5f; //bottom left
            float b = (noise[((y + 1) % height) * width + (x)] + 1.0f) * 0.5f; //bottom center
            float br = (noise[((y + 1) % height) * width + ((x + 1) % width)] + 1.0f) * 0.5f; //bottom right

            float dX = -((tr + 2.0f * r + br) - (tl + 2.0f * l + bl));
            float dY = -((bl + 2.0f * b + br) - (tl + 2.0f * t + tr));

            float dZ = 1.0f / strength;
            float len = sqrtf(dX * dX + dY * dY + dZ * dZ);
            dX /= len;
            dY /= len;

            int index = (y * width + x) * 4;
            normal[index + 0] = (unsigned char)((dX * 0.5f + 0.5f) * 255.f);
            normal[index + 1] = (unsigned char)((dY * 0.5f + 0.5f) * 255.f);
            normal[index + 2] = 255;
            normal[index + 3] = 255;
        }
    }
    cloudMaterial.normal = normal;

    // TODO: Emissive
    // TODO: Normal
    // TODO: Occlusion
}

glm::vec3 planet::Planet::LerpColor(glm::vec3 a, glm::vec3 b, float t)
{
    return {
            bee::Lerp(a.r, b.r, t),
            bee::Lerp(a.g, b.g, t),
            bee::Lerp(a.b, b.b, t),
        };
}

glm::vec3 planet::Planet::GetColorByHeight(float height)
{
    if (terrainColorPalette.empty())
    {
        const auto color = static_cast<unsigned char>(255.f * height);
        return glm::vec3(color);
    }

    for (size_t i = 0; i < terrainColorPalette.size() - 1; i++)
    {
        if (height < terrainColorPalette[i + 1].first)
        {
            float t = (height - terrainColorPalette[i].first) / (
                          terrainColorPalette[i + 1].first - terrainColorPalette[i].first);
            return LerpColor(terrainColorPalette[i].second, terrainColorPalette[i + 1].second, t);
        }
    }

    return terrainColorPalette.back().second; // If we've gone past the last gradient stop, return the last color
}

glm::vec3 planet::Planet::GetCloudColorByHeight(float height)
{
    float colorVariationRange = 0.8f;  // range of possible color variation (0 to 1)

    // Generate color variation offsets for each color component based on noise.
    // noise is in the range (0,1), here we first transform it to (-0.5,0.5) by subtracting 0.5 from it,
    // in order to create some negative offsets as well, and then we multiply by colorVariationRange to control the amplitude of variation.
    float offsetR = (height - 0.5f) * colorVariationRange;
    float offsetG = (height - 0.5f) * colorVariationRange;
    float offsetB = (height - 0.5f) * colorVariationRange;

    // Create the new color by adding the offsets to each color component, and using glm::clamp to ensure the value limits.
    glm::vec3 newColor;
    newColor.r = glm::clamp(cloudColor.r + offsetR, 0.0f, 1.0f);
    newColor.g = glm::clamp(cloudColor.g + offsetG, 0.0f, 1.0f);
    newColor.b = glm::clamp(cloudColor.b + offsetB, 0.0f, 1.0f);

    return newColor;
}
