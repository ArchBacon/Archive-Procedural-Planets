#pragma once

#include <imgui/ImGradientHDR.h>
#include "core/ecs.hpp"
#include "lib/Planet.h"
#include "lib/PlanetFactory.h"
#include "platform/opengl/mesh_gl.hpp"

namespace bee
{
struct Transform;
struct Material;
class Model;

class PlanetGenSystem : public System
{
public:
    PlanetGenSystem();
    ~PlanetGenSystem() override = default;
    void Update(float dt) override;
    void RebuildTerrain(bool keepColors = true);
    void RebuildClouds(bool keepColor = true);

#ifdef BEE_INSPECTOR
    void Inspect() override;
#endif

protected:
    std::string planetName;
    planet::Planet* planet = nullptr;
    bee::Transform* planetTransform;

    glm::vec3 cloudsRotationVelocity = glm::vec3(0.0f, 2.0f, 0.0f);
    glm::vec3 terrainRotationVelocity = glm::vec3(0.0f, 1.0f, 0.0f);

    std::string CloudName;
    bee::Transform* cloudsTransform;

    planet::PlanetFactory* factory = nullptr;

    std::shared_ptr<bee::Mesh> CreateMesh(planet::Mesh& mesh);
    std::shared_ptr<bee::Material> CreateMaterial(planet::Material& material);

    // Color picker stuffs
    glm::vec3 cloudColor{1.0f};
    int32_t stateID = 10;
    ImGradientHDRState state;
    ImGradientHDRTemporaryState tempState;
};
}
