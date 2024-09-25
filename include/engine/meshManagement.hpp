#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>

struct Mesh
{
    unsigned int VAO {};
    unsigned int indiciesLength {};
    unsigned int vertexCount {};

    void use() const
    {
        glBindVertexArray(VAO);
    }
};

enum class MeshType
{
    cube,
    tetrahedron,
    grid
};
enum class NormalMode
{
    none,
    smooth,
    flat
};

class MeshManager
{
private:
    struct MeshVariations
    {
        std::unique_ptr<Mesh> noNormalsMesh {};
        std::unique_ptr<Mesh> smoothNormalsMesh {};
        std::unique_ptr<Mesh> flatNormalsMesh {};
    };
    std::unordered_map<MeshType, MeshVariations> m_meshes {};
    std::unordered_map<int, MeshVariations> m_gridMeshes {};
    MeshManager() {}

    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager& other) = delete;
public:
    static MeshManager& getInstance()
    {
        static MeshManager instance = MeshManager();
        return instance;
    }

    Mesh getMesh(MeshType meshType, NormalMode normalMode);
    Mesh getGrid(int size, NormalMode normals);
};

namespace meshtools
{
    Mesh makeCube(NormalMode normalMode);
    Mesh makeTetrahedron(NormalMode normalMode);
    Mesh generateGrid(int gridSize, bool normals);
}