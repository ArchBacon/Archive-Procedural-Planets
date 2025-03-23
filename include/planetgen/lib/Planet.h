#pragma once

#include <tinygltf/stb_image.h>

#include "Clouds.h"
#include "Material.h"
#include "Sphere.h"
#include "Terrain.h"
#include "tools/log.hpp"

namespace planet
{
class Terrain;
class Clouds;

class Planet
{
    Mesh terrainMesh{}; // 1. Planet mesh 2. Cloud mesh
    Mesh cloudMesh{}; // 1. Planet mesh 2. Cloud mesh
    Material terrainMaterial{};
    Material cloudMaterial{};
    Terrain* terrain = nullptr;
    Clouds* clouds = nullptr;
    MeshConfig* config = nullptr;
    float waterLevel = 0.540f;

public:
    Planet(Terrain* terrain, Clouds* clouds, MeshConfig* config);

    // 1. Planet Mesh
    // 2. Cloud Mesh
    // Get as `auto [planet, cloud] = GetMeshes()`
    [[nodiscard]] std::tuple<Mesh, Mesh> GetMeshes() const { return {terrainMesh, cloudMesh}; }
    [[nodiscard]] const Material& GetTerrainMaterial() const { return terrainMaterial; }
    [[nodiscard]] const Material& GetTerrainMaterial(const std::vector<std::pair<float, glm::vec3>>& colors);
    [[nodiscard]] const Material& GetCloudMaterial() const { return cloudMaterial; }
    [[nodiscard]] const Material& GetCloudMaterial(glm::vec3 color);
    [[nodiscard]] const std::vector<std::pair<float, glm::vec3>>& GetTerrainColors() const { return terrainColorPalette; }
    [[nodiscard]] const glm::vec3& GetCloudColor() const { return cloudColor; }
    [[nodiscard]] Terrain* GetTerrain() const { return terrain; }
    [[nodiscard]] Clouds* GetClouds() const { return clouds; }
    [[nodiscard]] float GetWaterLevel() const { return waterLevel; }
    void SetWaterLevel(float level) { waterLevel = level; }

    void SetTerrain(Terrain* inTerrain);
    void SetClouds(Clouds* inClouds);
    // void SetConfig(MeshConfig* inConfig);

protected:
    void GenerateTerrainMaterial();
    void GenerateCloudsMaterial();

    glm::vec3 cloudColor{1.0f};
    std::vector<std::pair<float, glm::vec3>> terrainColorPalette{};

    glm::vec3 LerpColor(glm::vec3 a, glm::vec3 b, float t);
    glm::vec3 GetColorByHeight(float height);
    glm::vec3 GetCloudColorByHeight(float height);
};
}
