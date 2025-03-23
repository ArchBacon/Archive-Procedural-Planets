#include "planetgen/PlanetGenSystem.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.inl>
#include "core/engine.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "planetgen/lib/Planet.h"
#include "planetgen/lib/presets/clouds/NoClouds.h"
#include "planetgen/lib/presets/terrain/Gaia.h"
#include "platform/opengl/mesh_gl.hpp"
#include "rendering/image.hpp"
#include "rendering/model.hpp"
#include "rendering/render_components.hpp"
#include "tools/inspector.hpp"
#include "tools/log.hpp"

using namespace bee;

PlanetGenSystem::PlanetGenSystem()
{
    factory = new planet::PlanetFactory();
    factory->registerDefaultTerrains();
    factory->registerDefaultClouds();

    Title = "Planet Generation";

    // HDR
    {
        auto& renderer = Engine.ECS().CreateSystem<Renderer>();
        renderer.LoadEnvironment("environments/Nebula_16_Cube.HDR");
    }

    // Lights
    {
        auto entity = Engine.ECS().CreateEntity();
        auto& transform = Engine.ECS().CreateComponent<Transform>(entity, glm::vec3(2.0f, 1.5f, -1.5f), glm::vec3(1.0f), glm::vec3(0.0f));
        transform.Name = "Light " + std::to_string(static_cast<int>(entity));
        auto& light = Engine.ECS().CreateComponent<Light>(entity, glm::vec3{0.869f,0.726f,0.538f}, 4000.f, 6.f, Light::Type::Point);
    }

    // Camera
    {
        const auto entity = Engine.ECS().CreateEntity();
        auto& transform = Engine.ECS().CreateComponent<Transform>(entity);
        transform.Name = "Camera";
        const auto projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.2f, 1000.0f);
        const auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        auto& camera = Engine.ECS().CreateComponent<Camera>(entity);
        camera.Projection = projection;
        const auto viewInv = inverse(view);
        Decompose(viewInv, transform.Translation, transform.Scale, transform.Rotation, transform.RotationEuler);
    }

    // UV Sphere
    {
        auto clouds = new planet::NoClouds();
        auto terrain = new planet::Gaia();
        auto config = new planet::MeshConfig();

        planet = new planet::Planet(terrain, clouds, config);
        auto [myTerrain, myClouds] = planet->GetMeshes();
        auto terrainMats = planet->GetTerrainMaterial();
        auto cloudMats = planet->GetCloudMaterial();

        // Clouds
        {
            auto entity = Engine.ECS().CreateEntity();
            auto& transform = Engine.ECS().CreateComponent<Transform>(entity);
            CloudName = "Clouds " + std::to_string(static_cast<int>(entity));
            transform.Name = CloudName;
            cloudsTransform = &transform;

            auto& meshRenderer = Engine.ECS().CreateComponent<MeshRenderer>(entity);
            meshRenderer.Mesh = CreateMesh(myClouds);
            meshRenderer.Material = CreateMaterial(cloudMats);
        }

        // Terrain
        {
            auto entity = Engine.ECS().CreateEntity();
            auto& transform = Engine.ECS().CreateComponent<Transform>(entity);
            planetName = "Planet " + std::to_string(static_cast<int>(entity));
            transform.Name = planetName;
            transform.RotationEuler = {0.0f,0.0f,0.f};
            planetTransform = &transform;

            auto& meshRenderer = Engine.ECS().CreateComponent<MeshRenderer>(entity);
            meshRenderer.Mesh = CreateMesh(myTerrain);
            meshRenderer.Material = CreateMaterial(terrainMats);
        }
    }

    // Sky sphere
    {
        auto skySphere = Engine.Resources().Load<Model>("meshes/inverted_sphere.glb");
        skySphere->Instantiate();

        auto sampler = std::make_shared<Sampler>();
        auto sphere = std::make_shared<Image>("textures/Nebula_16_Cube.png");
        auto skySphereTex = std::make_shared<Texture>(sphere, sampler);

        auto skySphereMat = skySphere->GetMaterials()[0].get();
        skySphereMat->BaseColorTexture = skySphereTex;
        skySphereMat->UseBaseTexture = true;
        skySphereMat->IsUnlit = true;
    }

    auto palette = planet->GetTerrainColors();
    for (const auto& color : palette)
    {
        state.AddColorMarker(color.first, {color.second.r,color.second.g,color.second.b}, 1.0f);
    }
}

void PlanetGenSystem::Update(const float dt)
{
    cloudsTransform->RotationEuler += cloudsRotationVelocity * dt;
    cloudsTransform->Rotation = glm::quat(glm::radians(cloudsTransform->RotationEuler));
    
    planetTransform->RotationEuler += terrainRotationVelocity * dt;
    planetTransform->Rotation = glm::quat(glm::radians(planetTransform->RotationEuler));
}

void PlanetGenSystem::RebuildTerrain(bool keepColors)
{
    std::vector<std::pair<float, glm::vec3>> palette{};
    if (keepColors)
    {
        for (int i = 0; i < state.ColorCount; i++)
        {
            auto color = *state.GetColorMarker(i);
            palette.emplace_back(
                color.Position,
                glm::vec3(color.Color[0], color.Color[1], color.Color[2])
            );
        }
    }
    
    const auto view = Engine.ECS().Registry.view<const Transform, MeshRenderer>();
    for (const auto& entity : view)
    {
        auto [transform, mesh] = view.get(entity);
        if (transform.Name == planetName)
        {
            auto material = keepColors ? planet->GetTerrainMaterial(palette) : planet->GetTerrainMaterial();
            mesh.Material = CreateMaterial(material);
            for (int i = 8; i >= 0; i--)
            {
                state.RemoveColorMarker(i);
            }

            auto colors = planet->GetTerrainColors();
            for (const auto& color : colors)
            {
                state.AddColorMarker(color.first, {color.second.r,color.second.g,color.second.b}, 1.0f);
            }
            break;
        }
    }
}

void PlanetGenSystem::RebuildClouds(bool keepColor)
{
    const auto cview = Engine.ECS().Registry.view<const Transform, MeshRenderer>();
    for (const auto& entity : cview)
    {
        auto [transform, mesh] = cview.get(entity);
        if (transform.Name == CloudName)
        {
            auto material = keepColor ? planet->GetCloudMaterial(cloudColor) : planet->GetCloudMaterial();
            mesh.Material = CreateMaterial(material);

            cloudColor = planet->GetCloudColor();
            break;
        }
    }
}

#ifdef BEE_INSPECTOR
void PlanetGenSystem::Inspect()
{
    // All
    // TODO: Reset to defaults button
    // TODO: Export (gltf) planet button
    // TODO: Add planet to list for generating solar system

    // Terrain
    // TODO: Auto rebuild on water level change
    // TODO: normal strength sliders for water and land
    // TODO: Add terrain to list of structs with planet configs

    // Clouds
    // TODO: emissiveness
    
    ImGui::Begin("Planet");

    // ---------------- TERRAIN ---------------- //
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Terrain");
    ImGui::Dummy(ImVec2(0, 5));
    
    const auto terrains = factory->GetTerrains();
    static std::string currentTerrain = terrains[0];
    if (ImGui::BeginCombo("Preset##terrain", currentTerrain.c_str()))
    {
        for (const auto& terrain : terrains)
        {
            bool is_selected = (currentTerrain == terrain);
            if (ImGui::Selectable(terrain.c_str(), is_selected))
            {
                currentTerrain = terrain;
                auto newTerrain = factory->instantiateTerrain(currentTerrain);
                newTerrain->SetSeed(planet->GetTerrain()->GetSeed());
                newTerrain->SetTextureResolution(planet->GetTerrain()->GetTextureResolution());
                planet->SetTerrain(newTerrain);
                
                RebuildTerrain(false);
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    static int terrainSeed = 1337;
    if (ImGui::InputInt("Seed##terrain", &terrainSeed))
    {
        planet->GetTerrain()->SetSeed(terrainSeed);
        planet->SetTerrain(planet->GetTerrain());
        RebuildTerrain();
    }

    static int terrainResolution = 1024;
    static int terrainPrevResolution = 1024;
    if (ImGui::InputInt("Resolution##terrain", &terrainResolution, 1, 1)) {
        if (terrainResolution >= terrainPrevResolution)
        {
            terrainResolution = terrainPrevResolution * 2;
        }
        else
        {
            terrainResolution = terrainPrevResolution / 2;
        }

        terrainResolution = std::clamp(terrainResolution, 64, 4096);

        if (terrainResolution != terrainPrevResolution)
        {
            planet->GetTerrain()->SetTextureResolution(terrainResolution);
            planet->SetTerrain(planet->GetTerrain());
            RebuildTerrain();
        }

        terrainPrevResolution = terrainResolution;
    }

    bool emissive = planet->GetTerrain()->IsEmissive();
    if (ImGui::Checkbox("Emissive", &emissive))
    {
        planet->GetTerrain()->SetEmissive(emissive);
        planet->SetTerrain(planet->GetTerrain());
        RebuildTerrain();
    }

    ImGui::DragFloat3("Rotation", glm::value_ptr(terrainRotationVelocity));

    static float waterLevel = planet->GetWaterLevel();
    if (ImGui::DragFloat("Water Level", &waterLevel, 0.01f, 0.0f, 1.0f))
    {
        planet->SetWaterLevel(waterLevel);
        RebuildTerrain();
    }

    bool isMarkerShown = true;
    ImGradientHDR(stateID, state, tempState, isMarkerShown);

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Gradient");
    }

    if (tempState.selectedMarkerType == ImGradientHDRMarkerType::Color)
    {
        auto selectedColorMarker = state.GetColorMarker(tempState.selectedIndex);
        if (selectedColorMarker != nullptr)
        {
            ImGui::ColorEdit3("Color", selectedColorMarker->Color.data(), ImGuiColorEditFlags_Float);
            ImGui::InputFloat("Position", &selectedColorMarker->Position, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
    }

    if (tempState.selectedMarkerType != ImGradientHDRMarkerType::Unknown)
    {
        if (ImGui::Button("Delete"))
        {
            if (tempState.selectedMarkerType == ImGradientHDRMarkerType::Color)
            {
                state.RemoveColorMarker(tempState.selectedIndex);
                tempState = ImGradientHDRTemporaryState{};
            }
            else if (tempState.selectedMarkerType == ImGradientHDRMarkerType::Alpha)
            {
                state.RemoveAlphaMarker(tempState.selectedIndex);
                tempState = ImGradientHDRTemporaryState{};
            }
        }
    }

    // ---------------- CLOUDS ---------------- //
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Clouds");
    ImGui::Dummy(ImVec2(0, 5));

    const auto clouds = factory->GetClouds();
    static std::string currentCloud = clouds[0];
    if (ImGui::BeginCombo("Preset##clouds", currentCloud.c_str()))
    {
        for (const auto& cloud : clouds)
        {
            bool is_selected = (currentCloud == cloud);
            if (ImGui::Selectable(cloud.c_str(), is_selected))
            {
                currentCloud = cloud;
                auto newClouds = factory->instantiateClouds(currentCloud);
                newClouds->SetSeed(planet->GetClouds()->GetSeed());
                newClouds->SetTextureResolution(planet->GetClouds()->GetTextureResolution());
                planet->SetClouds(newClouds);

                RebuildClouds(false);
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    static int cloudSeed = 1337;
    if (ImGui::InputInt("Seed##clouds", &cloudSeed))
    {
        planet->GetClouds()->SetSeed(cloudSeed);
        planet->SetClouds(planet->GetClouds());
        RebuildClouds();
    }

    static int cloudResolution = 1024;
    static int cloudPrevResolution = 1024;
    if (ImGui::InputInt("Resolution##clouds", &cloudResolution, 1, 1)) {
        if (cloudResolution >= cloudPrevResolution)
        {
            cloudResolution = cloudPrevResolution * 2;
        }
        else
        {
            cloudResolution = cloudPrevResolution / 2;
        }

        cloudResolution = std::clamp(cloudResolution, 64, 4096);

        if (cloudResolution != cloudPrevResolution)
        {
            planet->GetClouds()->SetTextureResolution(cloudResolution);
            planet->SetClouds(planet->GetClouds());
            RebuildClouds();
        }

        cloudPrevResolution = cloudResolution;
    }
    
    ImGui::DragFloat3("Cloud Velocity", glm::value_ptr(cloudsRotationVelocity));
    ImGui::ColorEdit3("Color##Clouds", glm::value_ptr(cloudColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

    // ---------------- OPTIONS ---------------- //
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));
    if (ImGui::Button("Rebuild Planet"))
    {
        std::vector<std::pair<float, glm::vec3>> palette{};
        for (int i = 0; i < state.ColorCount; i++)
        {
            auto color = *state.GetColorMarker(i);
            palette.emplace_back(
                color.Position,
                glm::vec3(color.Color[0], color.Color[1], color.Color[2])
            );
        }

        auto terrainMats = planet->GetTerrainMaterial(palette);
        const auto view = Engine.ECS().Registry.view<const Transform, MeshRenderer>();
        for (const auto& entity : view)
        {
            auto [transform, mesh] = view.get(entity);
            if (transform.Name == planetName)
            {
                mesh.Material = CreateMaterial(terrainMats);
                break;
            }
        }

        auto cloudMats = planet->GetCloudMaterial(cloudColor);
        const auto cloudview = Engine.ECS().Registry.view<const Transform, MeshRenderer>();
        for (const auto& entity : cloudview)
        {
            auto [transform, mesh] = cloudview.get(entity);
            if (transform.Name == CloudName)
            {
                mesh.Material = CreateMaterial(cloudMats);
                break;
            }
        }
    }

    ImGui::End();
}
#endif

std::shared_ptr<bee::Mesh> PlanetGenSystem::CreateMesh(planet::Mesh& mesh)
{
    auto output = Engine.Resources().Create<Mesh>();
    output->SetIndices(mesh.indices);
    output->SetAttribute(Mesh::Attribute::Position, mesh.positions);
    output->SetAttribute(Mesh::Attribute::Normal, mesh.normals);
    output->SetAttribute(Mesh::Attribute::Texture, mesh.uvs);

    std::vector<uint32_t> indices(mesh.indices.begin(), mesh.indices.end());
    auto tans = output->ComputeTangents(mesh.positions, mesh.normals, mesh.uvs, indices);
    output->SetAttribute(Mesh::Attribute::Tangent, tans);

    return output;
}

std::shared_ptr<bee::Material> PlanetGenSystem::CreateMaterial(planet::Material& material)
{
    auto output = std::make_shared<Material>();
    auto sampler = std::make_shared<Sampler>();

    // Albedo
    if (!material.albedo.empty())
    {
        Log::Info("Albedo");
        auto albedo = std::make_shared<Image>("Albedo", true);
        albedo->CreateGLTextureWithData(material.albedo.data(), material.resolution, material.resolution, material.channels, true);
        const auto albedoTexture = std::make_shared<Texture>(albedo, sampler);

        output->BaseColorTexture = albedoTexture;
        output->UseBaseTexture = true;
        output->BaseColorFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Emissive
    if (!material.emissive.empty())
    {
        Log::Info("Emissive");
        auto emissive = std::make_shared<Image>("Emissive", true);
        emissive->CreateGLTextureWithData(material.emissive.data(), material.resolution, material.resolution, material.channels, true);
        const auto emissiveTexture = std::make_shared<Texture>(emissive, sampler);

        output->EmissiveTexture = emissiveTexture;
        output->UseEmissiveTexture = true;
        output->EmissiveFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    // Normal
    if (!material.normal.empty())
    {
        Log::Info("Normal");
        auto normal = std::make_shared<Image>("Normal", true);
        normal->CreateGLTextureWithData(material.normal.data(), material.resolution, material.resolution, material.channels, true);
        const auto normalTexture = std::make_shared<Texture>(normal, sampler);

        output->NormalTexture = normalTexture;
        output->UseNormalTexture = true;
        output->NormalTextureScale = 1.0f;
    }

    // Occlusion
    if (!material.occlusion.empty())
    {
        Log::Info("Occlusion");
        auto occlusion = std::make_shared<Image>("Occlusion", true);
        occlusion->CreateGLTextureWithData(material.occlusion.data(), material.resolution, material.resolution, material.channels, true);
        auto occlusionTexture = std::make_shared<Texture>(occlusion, sampler);

        output->OcclusionTexture = occlusionTexture;
        output->UseOcclusionTexture = true;
        output->OcclusionTextureStrength = 1.0f;
    }

    // Metallic/Roughness
    if (!material.metallicRoughness.empty())
    {
        Log::Info("Metallic/Roughness");
        auto metallicRoughness = std::make_shared<Image>("Metallic/Roughness", true);
        metallicRoughness->CreateGLTextureWithData(material.metallicRoughness.data(), material.resolution, material.resolution, material.channels, true);
        auto metallicRoughnessTexture = std::make_shared<Texture>(metallicRoughness, sampler);

        output->MetallicRoughnessTexture = metallicRoughnessTexture;
        output->UseMetallicRoughnessTexture = true;
        output->MetallicFactor = 1.0f;
        output->RoughnessFactor = 1.0f;
    }

    return output;
}
