#include <memory>
#include <map>
#include <stdexcept>

#include <glad/glad.h>

#include "engine/meshManagement.h"

Mesh Meshes::getCube()
{
    if(!m_tetrahedron)
        m_tetrahedron = std::make_unique<Mesh>(MeshTools::makeCube());
    return *m_tetrahedron;
}

Mesh Meshes::getTetrahedron()
{
    if(!m_tetrahedron)
        m_tetrahedron = std::make_unique<Mesh>(MeshTools::makeTetrahedron());
    return *m_tetrahedron;
}

Mesh Meshes::getGrid(int size)
{
    if(!m_grids[size])
        m_grids[size] = std::make_unique<Mesh>(MeshTools::generateGrid(size));
    return *m_grids[size];
}

unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength);

Mesh MeshTools::makeCube()
{
    static constexpr float vertices[] 
    {
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
    };
    static constexpr unsigned int indices[]
    {
        0, 1, 2,
        2, 3, 1,

        4, 5, 6,
        6, 7, 5,

        0, 1, 4,
        4, 5, 1,

        2, 3, 6,
        6, 7, 3,

        0, 4, 6,
        6, 2, 0,
    
        1, 5, 7,
        7, 3, 1
    };

    return {generateVAO(vertices, std::ssize(vertices), indices, std::ssize(indices)), std::size(indices)};
}

Mesh MeshTools::makeTetrahedron()
{
    static constexpr float vertices[] 
    {
        1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
    };

    static constexpr unsigned int indices[] 
    {
        0, 1, 2,
        0, 1, 3,
        0, 2, 3,
        1, 2, 3 
    };

    return {generateVAO(vertices, std::ssize(vertices), indices, std::ssize(indices)), std::size(indices)};
}

Mesh MeshTools::generateGrid(int gridSize)
{
    if(gridSize <= 0) throw std::invalid_argument("Grid size must be positive");
    if(gridSize % 2 != 0) throw std::invalid_argument("Grid size can't be odd");

    const int verticesLength {(gridSize + 1) * (gridSize + 1) * 3};
    std::unique_ptr<float[]> vertices = std::make_unique<float[]>(verticesLength);

    int verticleIndex {};
    for(int x {-(gridSize / 2)}; x <= (gridSize / 2); ++x)
    {
        for(int y {-(gridSize / 2)}; y <= (gridSize / 2); ++y)
        {
            vertices[verticleIndex++] = (static_cast<float>(x * 2) / gridSize);
            vertices[verticleIndex++] = (static_cast<float>(y * 2) / gridSize);
            vertices[verticleIndex++] = .0f;
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

    return {generateVAO(vertices.get(), verticesLength, indices.get(), indicesLength), indicesLength};
}

unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength)
{
    unsigned int EBO {}, VBO {}, VAO {};
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesLength, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLength, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}