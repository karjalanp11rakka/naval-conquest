#include <memory>
#include <map>
#include <vector>
#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/meshManagement.hpp"

Mesh Meshes::getCube(bool normals)
{
    auto& cube {normals ? m_cubeNormals : m_cube};
    if(!cube)
        cube = std::make_unique<Mesh>(MeshTools::makeCube(normals));
    return *cube;
}

Mesh Meshes::getTetrahedron(bool normals)
{
    auto& tetrahedron {normals ? m_tetrahedronNormals : m_tetrahedron};
    if(!tetrahedron)
        tetrahedron = std::make_unique<Mesh>(MeshTools::makeTetrahedron(normals));
    return *tetrahedron;
}

Mesh Meshes::getGrid(int size, bool normals)
{
    auto& grids {normals ? m_gridsNormals : m_grids};
    if(!grids[size])
        grids[size] = std::make_unique<Mesh>(MeshTools::generateGrid(size, normals));
    return *grids[size];
}

unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength, bool normals);
void addNormals(std::unique_ptr<float[]>& vertices, int verticesLength, const unsigned int indices[], int indicesLength);

Mesh MeshTools::makeCube(bool normals)
{
    static constexpr float vertices[] 
    {
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
    };

    static constexpr unsigned int indices[] 
    {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        0, 3, 7,
        7, 4, 0,

        1, 5, 6,
        6, 2, 1,

        3, 2, 6,
        6, 7, 3,

        0, 4, 5,
        5, 1, 0
    };

    if(normals)
    {
        std::unique_ptr<float[]> verticesWithNormals = std::make_unique<float[]>(std::ssize(vertices));
        std::copy(std::begin(vertices), std::end(vertices), verticesWithNormals.get());
        addNormals(verticesWithNormals, std::ssize(vertices), indices, std::ssize(indices));
        
        return {generateVAO(verticesWithNormals.get(), std::ssize(vertices) * 2, indices, std::ssize(indices), normals), std::size(indices)};
    }

    return {generateVAO(vertices, std::ssize(vertices), indices, std::ssize(indices), normals), std::size(indices)};
}
Mesh MeshTools::makeTetrahedron(bool normals)
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

    if(normals)
    {
        std::unique_ptr<float[]> verticesWithNormals = std::make_unique<float[]>(std::ssize(vertices));
        std::copy(std::begin(vertices), std::end(vertices), verticesWithNormals.get());
        addNormals(verticesWithNormals, std::ssize(vertices), indices, std::ssize(indices));
        
        return {generateVAO(verticesWithNormals.get(), std::ssize(vertices) * 2, indices, std::ssize(indices), normals), std::size(indices)};
    }

    return {generateVAO(vertices, std::ssize(vertices), indices, std::ssize(indices), normals), std::size(indices)};
}

Mesh MeshTools::generateGrid(int gridSize, bool normals)
{
    assert(gridSize > 0 && "Grid size must be positive");
    assert(gridSize % 2 == 0 && "Grid size can't be odd");

    const int verticesLength {(gridSize + 1) * (gridSize + 1) * 3 * (normals ? 2 : 1)};
    std::unique_ptr<float[]> vertices = std::make_unique<float[]>(verticesLength);

    int verticleIndex {};
    for(int x {-(gridSize / 2)}; x <= (gridSize / 2); ++x)
    {
        for(int y {-(gridSize / 2)}; y <= (gridSize / 2); ++y)
        {
            vertices[verticleIndex++] = (static_cast<float>(x * 2) / gridSize);
            vertices[verticleIndex++] = (static_cast<float>(y * 2) / gridSize);
            vertices[verticleIndex++] = .0f;

            if(normals)
            {
                vertices[verticleIndex++] = .0f;
                vertices[verticleIndex++] = .0f;
                vertices[verticleIndex++] = 1.f;
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesLength, indices, GL_STATIC_DRAW);

    //pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (normals ? 6 : 3) * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals
    if(normals)
    {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

void addNormals(std::unique_ptr<float[]>& vertices, int verticesLength, const unsigned int indices[], int indicesLength)
{
    std::vector<glm::vec3> tempVertices(verticesLength / 3);
    
    for (int i = 0; i < verticesLength; i += 3)
    {
        tempVertices[i / 3] = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
    }

    std::vector<glm::vec3> normals(tempVertices.size(), glm::vec3(0.0f));

    for (unsigned int i {}; i < indicesLength; i += 3)
    {
        unsigned int i0 {indices[i]};
        unsigned int i1 {indices[i + 1]};
        unsigned int i2 {indices[i + 2]};

        glm::vec3 v1 = tempVertices[i1] - tempVertices[i0];
        glm::vec3 v2 = tempVertices[i2] - tempVertices[i0];
        glm::vec3 faceNormal = glm::cross(v1, v2);

        if (glm::dot(faceNormal, glm::normalize(tempVertices[i0])) < 0)
        {
            faceNormal = -faceNormal;
        }

        normals[i0] += faceNormal;
        normals[i1] += faceNormal;
        normals[i2] += faceNormal;
    }

    for (auto& norm : normals)
    {
        norm = glm::normalize(norm);   
    }

    std::unique_ptr<float[]> newVertices = std::make_unique<float[]>(verticesLength * 2);
    for (int i = 0, j = 0; i < verticesLength; i += 3, j += 6)
    {
        newVertices[j] = vertices[i];
        newVertices[j + 1] = vertices[i + 1];
        newVertices[j + 2] = vertices[i + 2];
        newVertices[j + 3] = normals[i / 3].x;
        newVertices[j + 4] = normals[i / 3].y;
        newVertices[j + 5] = normals[i / 3].z;
    }

    vertices = std::move(newVertices);
}