#pragma once

#include <memory>
#include <map>

#include <glad/glad.h>

struct Mesh
{
    unsigned int VAO {};
    unsigned int indiciesLength {};

    void use() const
    {
        glBindVertexArray(VAO);
    }
};

class Meshes
{
private:
    std::unique_ptr<Mesh> m_cube {}, m_tetrahedron {};
    std::unique_ptr<Mesh> m_cubeNormals {}, m_tetrahedronNormals {};
    std::map<int, std::unique_ptr<Mesh>> m_grids{}, m_gridsNormals{};
    Meshes() {}

    Meshes(const Meshes&) = delete;
    Meshes& operator=(const Meshes& other) = delete;
public:
    static Meshes& getInstance()
    {
        static Meshes instance = Meshes();
        return instance;
    }

    Mesh getCube(bool normals = true);
    Mesh getTetrahedron(bool normals = true);
    Mesh getGrid(int size, bool normals = true);
};

namespace MeshTools
{
    Mesh makeCube(bool normals = true);
    Mesh makeTetrahedron(bool normals = true);
    Mesh generateGrid(int gridSize, bool normals = true);
}