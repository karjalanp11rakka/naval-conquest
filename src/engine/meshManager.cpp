#include <memory>
#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/meshManager.hpp>

Mesh generateGrid(int gridSize, bool normals);
Mesh MeshManager::getGrid(int size, NormalMode normalMode)
{
    auto& gridVariations {m_gridMeshes[size]};

    bool normals {normalMode != NormalMode::none};
    auto& gridPtr = normals ? 
        gridVariations.flatNormalsMesh : gridVariations.noNormalsMesh;

    if(!gridPtr)
        gridPtr = std::make_unique<Mesh>(generateGrid(size, normals));
    return *gridPtr;
}

Mesh generateGrid(int gridSize, bool normals)
{
    if(gridSize == 1)
    {
        static constexpr float vertices[]
        {
            -1.f, 1.f, 0.f,
            1.f, 1.f, 0.f,
            1.f, -1.f, 0.f,
            -1.f, -1.f, 0.f,
        };
        static constexpr float normalVertices[]
        {
            -1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 
            1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 
            1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 
            -1.f, -1.f, 0.f, 0.f, 0.f, 1.f
        };

        static constexpr unsigned int indices[]
        {
            0, 1, 2,
            0, 3, 2
        };
        
        return {generateVAO(normals ? normalVertices : vertices, normals ? std::size(normalVertices) : std::size(vertices),
            indices, std::size(indices), normals), std::size(indices)};

    }
    assert(gridSize > 0 && "Grid size must be positive.");
    assert(gridSize % 2 == 0 && "Grid size can't be odd.");

    const int verticesLength {(gridSize + 1) * (gridSize + 1) * 3 * (normals ? 2 : 1)};
    std::unique_ptr<float[]> vertices = std::make_unique<float[]>(verticesLength);

    int vertexIndex {};
    for(int x {-(gridSize / 2)}; x <= (gridSize / 2); ++x)
    {
        for(int y {-(gridSize / 2)}; y <= (gridSize / 2); ++y)
        {
            vertices[vertexIndex++] = (static_cast<float>(x * 2) / gridSize);
            vertices[vertexIndex++] = (static_cast<float>(y * 2) / gridSize);
            vertices[vertexIndex++] = .0f;

            if(normals)
            {
                vertices[vertexIndex++] = .0f;
                vertices[vertexIndex++] = .0f;
                vertices[vertexIndex++] = 1.f;
            }
        }
    }

    const unsigned int indicesLength
    { 6 * static_cast<unsigned int>(gridSize) * static_cast<unsigned int>(gridSize)};
    
    std::unique_ptr<unsigned int[]> indices = std::make_unique<unsigned int[]>(indicesLength);

    int indexIndex {};
    for(int row {}; row < ((gridSize + 1) * gridSize); row += (gridSize + 1))
    {
        for (int squareStartPos {row}; squareStartPos < (gridSize + row); ++squareStartPos)
        {
            indices[indexIndex++] = squareStartPos;
            indices[indexIndex++] = squareStartPos + 1;
            indices[indexIndex++] = squareStartPos + (gridSize + 2);

            indices[indexIndex++] = squareStartPos;
            indices[indexIndex++] = squareStartPos + (gridSize + 1);
            indices[indexIndex++] = squareStartPos + (gridSize + 2);
        }
    }

    return {generateVAO(vertices.get(), verticesLength, indices.get(), indicesLength, normals), indicesLength};
}
unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength, bool normals)
{
    unsigned int EBO {}, VBO {}, VAO {};
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesLength, vertices, GL_STATIC_DRAW);

    if(indices != nullptr)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLength, indices, GL_STATIC_DRAW);
    }

    //pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (normals ? 6 : 3) * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals
    if(normals)
    {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);

    return VAO;
}