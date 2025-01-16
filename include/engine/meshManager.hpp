#pragma once

#include <memory>
#include <unordered_map>
#include <cstddef>

#include <glad/glad.h>

struct Mesh
{
    unsigned int VAO {};
    unsigned int indicesLength {};
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
unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength, bool normals);
class MeshManager
{
private:
    MeshManager() {}
    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager& other) = delete;

    struct MeshVariations
    {
        std::unique_ptr<Mesh> noNormalsMesh;
        std::unique_ptr<Mesh> smoothNormalsMesh;
        std::unique_ptr<Mesh> flatNormalsMesh;
    };
    std::unordered_map<MeshType, MeshVariations> m_meshes;
    std::unordered_map<int, MeshVariations> m_gridMeshes;
    std::unordered_map<const void*, Mesh> m_loadedMeshes;
public:
    static MeshManager& getInstance()
    {
        static MeshManager instance;
        return instance;
    }
    template<std::size_t V, std::size_t I>
    Mesh getMesh(const std::array<float, V>& vertices, const std::array<unsigned int, I>& indices, bool useNormals)
    {
        if(!m_loadedMeshes.contains(&vertices))
        {
            m_loadedMeshes[&vertices] = {generateVAO(vertices.data(), vertices.size(), indices.data(), 
                indices.size(), useNormals), static_cast<unsigned int>(indices.size())};
        }
        return m_loadedMeshes[&vertices];
    }
    Mesh getMesh(MeshType meshType, NormalMode normalMode);
    Mesh getGrid(int size, NormalMode normals);
};