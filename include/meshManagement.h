#pragma once

#include <memory>
#include <map>

#include <glad/glad.h>

struct Mesh
{
    unsigned int VAO {};
    unsigned int indiciesLength {};

    void use()
    {
        glBindVertexArray(VAO);
    }
};

class Meshes
{
private:
    std::unique_ptr<Mesh> m_cube {}, m_tetrahedron {};
    std::map<int, std::unique_ptr<Mesh>> m_grids{};
    Meshes() {}

    Meshes(const Meshes&) = delete;
    Meshes& operator=(const Meshes& other) = delete;
public:
    static Meshes& getInstance()
    {
        static Meshes instance {};
        return instance;
    }

    Mesh getCube();
    Mesh getTetrahedron();
    Mesh getGrid(int size);
};

namespace MeshTools
{
    Mesh makeCube();
    Mesh makeTetrahedron();
    Mesh generateGrid(int gridSize);
}