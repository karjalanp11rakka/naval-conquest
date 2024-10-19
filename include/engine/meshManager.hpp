#pragma once

#include <memory>
#include <unordered_map>
#include <string>

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
    tetrahedron
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
    MeshManager() {}
    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager& other) = delete;

    struct MeshVariations
    {
        std::unique_ptr<Mesh> noNormalsMesh {};
        std::unique_ptr<Mesh> smoothNormalsMesh {};
        std::unique_ptr<Mesh> flatNormalsMesh {};
    };
    std::unordered_map<MeshType, MeshVariations> m_meshes {};
    std::unordered_map<int, MeshVariations> m_gridMeshes {};
    std::unordered_map<const std::string*, Mesh> m_loadedMeshes {};
public:
    static MeshManager& getInstance()
    {
        static MeshManager instance {MeshManager()};
        return instance;
    }

    Mesh getMesh(MeshType meshType, NormalMode normalMode);
    Mesh getGrid(int size, NormalMode normals);
    Mesh getFromOBJ(const std::string& objString);
};

namespace meshtools
{
    Mesh generateCube(NormalMode normalMode);
    Mesh generateTetrahedron(NormalMode normalMode);
    Mesh generateGrid(int gridSize, bool normals);
    Mesh loadFromOBJ(const std::string& objString);
}