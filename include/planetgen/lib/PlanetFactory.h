#pragma once
#include <memory>

#include "Clouds.h"
#include "Terrain.h"
#include "presets/clouds/CirrusClouds.h"
#include "presets/clouds/DenseClouded.h"
#include "presets/clouds/NoClouds.h"
#include "presets/clouds/PlanetaryShield.h"
#include "presets/terrain/Alien.h"
#include "presets/terrain/Gaia.h"
#include "presets/terrain/Machine.h"
#include "presets/terrain/Barren.h"
#include "presets/terrain/Moon.h"
#include "presets/terrain/Sun.h"
#include "presets/terrain/Volcanic.h"

namespace planet
{
// A functor type that creates and returns a unique_ptr to a Planet
using TerrainFactory = Terrain*(*)();
using CloudFactory = Clouds*(*)();

// A global function to create instances of a generic Planet subclass T
template<typename T>
Terrain* createTerrainInstance() { return new T(); }
template<typename T>
Clouds* createCloudsInstance() { return new T(); }

class PlanetFactory
{
    std::unordered_map<std::string, TerrainFactory> terrainFactory;
    std::unordered_map<std::string, CloudFactory> cloudFactory;
    
public:
    // ----------------- TERRAIN ----------------- //
    Terrain* instantiateTerrain(const std::string& className) {
        const auto iter = terrainFactory.find(className);
        if (iter != terrainFactory.end())
        {
            return iter->second();
        }
        return nullptr;
    }
    
    template<typename T>
    void registerTerrain(const std::string& name)
    {
        terrainFactory[name] = &createTerrainInstance<T>;
    }

    void registerDefaultTerrains()
    {
        registerTerrain<Gaia>("Gaia");
        registerTerrain<Barren>("Barren");
        registerTerrain<Volcanic>("Volcanic"); // TODO: improve visual appeal
        registerTerrain<Sun>("Sun");
        registerTerrain<Alien>("Alien");
        registerTerrain<Machine>("Machine World");
        registerTerrain<Moon>("Moon");
    }

    std::vector<std::string> GetTerrains() const
    {
        std::vector<std::string> terrains{};
        terrains.reserve(terrainFactory.size());
        for (const auto& terrain : terrainFactory)
        {
            terrains.push_back(terrain.first);
        }
    
        return terrains;
    }

    // ----------------- CLOUDS ----------------- //
    Clouds* instantiateClouds(const std::string& className) {
        const auto iter = cloudFactory.find(className);
        if (iter != cloudFactory.end())
        {
            return iter->second();
        }
        return nullptr;
    }
    
    template<typename T>
    void registerCloud(const std::string& name)
    {
        cloudFactory[name] = &createCloudsInstance<T>;
    }

    void registerDefaultClouds()
    {
        registerCloud<NoClouds>("None");
        registerCloud<CirrusClouds>("Cirrus");
        registerCloud<DenseClouded>("Densely Clouded");
        registerCloud<PlanetaryShield>("Planetary Shield");
    }

    std::vector<std::string> GetClouds() const
    {
        std::vector<std::string> clouds{};
        clouds.reserve(cloudFactory.size());
        for (const auto& cloud : cloudFactory)
        {
            clouds.push_back(cloud.first);
        }

        return clouds;
    }
};
}
