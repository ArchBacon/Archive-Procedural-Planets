#include "planetgen/lib/Sphere.h"

#include "tools/log.hpp"

std::unordered_map<int, planet::SphericalCoordinates> planet::Sphere::coords{};

planet::Mesh planet::Sphere::UV(const float radius, const int stacks, const int sectors, const bool inverted)
{
    Mesh mesh{};

    const float PI = glm::pi<float>();
    const float stacksf = (float)stacks;
    const float sectorsf = (float)sectors;

    // Generate positions and normals
    for (int i = 0; i <= stacks; ++i)
    {
        // V texture coordinate
        float v = (float)i / stacksf;
        const float phi = v * PI;

        for (int j = 0; j <= sectors; ++j)
        {
            // U texture coordinate
            float u = (float)j / sectorsf;
            const float theta = u * PI * 2.0f;

            // Using spherical coordinates, convert them to cartesian coordinates
            const float x = cos(theta) * sin(phi);
            const float y = cos(phi);
            const float z = sin(theta) * sin(phi);

            mesh.positions.push_back(glm::vec3(x, y, z) * radius);
            mesh.normals.push_back(glm::normalize(glm::vec3(x, y, z)));
            mesh.uvs.emplace_back(u, v);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i)
    {
        short k1 = (short)(i * (sectors + 1));     // Beginning of current stack
        short k2 = (short)(k1 + sectors + 1);      // Beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (!inverted)
            {
                if (i != 0)
                {
                    mesh.indices.push_back(k1 + 1);
                    mesh.indices.push_back(k2);
                    mesh.indices.push_back(k1);
                }

                if (i != (stacks - 1))
                {
                    mesh.indices.push_back(k2 + 1);
                    mesh.indices.push_back(k2);
                    mesh.indices.push_back(k1 + 1);
                }
            }
            else
            {
                if (i != 0)
                {
                    mesh.indices.push_back(k1);
                    mesh.indices.push_back(k2);
                    mesh.indices.push_back(k1 + 1);
                }

                if (i != (stacks - 1))
                {
                    mesh.indices.push_back(k1 + 1);
                    mesh.indices.push_back(k2);
                    mesh.indices.push_back(k2 + 1);
                }
            }
        }
    }

    return mesh;
}

planet::SphericalCoordinates planet::Sphere::GetSphericalCoordinates(const float radius, const int resolution, const glm::vec3 offset)
{
    const auto it = coords.find(resolution);
    if (it != coords.end())
    {
        auto v =  it->second;
        #pragma omp parallel for
        for (size_t i = 0; i < v.x.size(); i++)
        {
            v.x[i] *= radius + offset.x;
            v.y[i] *= radius + offset.y;
            v.z[i] *= radius + offset.z;
        }

        return v;
    }

    CalculateSphericalCoordinates(resolution);
    return GetSphericalCoordinates(radius, resolution, offset);
}

void planet::Sphere::CalculateSphericalCoordinates(int resolution)
{
    SphericalCoordinates output(resolution * resolution);

#pragma omp parallel for
    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            // Map x, y to [-1, 1] range
            const float u = 2.0f * (((float)x / (float)resolution) - 0.5f);
            float v = 2.0f * (((float)y / (float)resolution) - 0.5f);
            // Fix the pole locations issue (poles where rendered equator instead of the poles)
            v = 1.0f - v;
            
            // Convert u, v to spherical coordinates
            const float theta = u * glm::pi<float>();
            const float phi = (v * glm::half_pi<float>()) - glm::half_pi<float>();

            // Convert spherical coordinates to Cartesian coordinates
            const int index = y * resolution + x;
            output.x[index] = cos(phi) * cos(theta);
            output.y[index] = cos(phi) * sin(theta);
            output.z[index] = sin(phi);
        }
    }

    coords.emplace(resolution, output);
}
