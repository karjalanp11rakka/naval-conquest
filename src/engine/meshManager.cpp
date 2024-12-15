#include <memory>
#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>
#include <cstddef>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/meshManager.hpp>

Mesh MeshManager::getMesh(MeshType meshType, NormalMode normalMode)
{
    auto& meshVariations {m_meshes[meshType]};
    auto& meshPtr = normalMode == NormalMode::flat ? 
        meshVariations.flatNormalsMesh : (normalMode == NormalMode::smooth ? meshVariations.smoothNormalsMesh : meshVariations.noNormalsMesh);
    
    if(!meshPtr)
    {
        switch (meshType)
        {
        case MeshType::cube:
            meshPtr = std::make_unique<Mesh>(meshtools::generateCube(normalMode));
            break;
        case MeshType::tetrahedron:
            meshPtr = std::make_unique<Mesh>(meshtools::generateTetrahedron(normalMode));
            break;
        }
    }
    return *meshPtr;
}
Mesh MeshManager::getGrid(int size, NormalMode normalMode)
{
    auto& gridVariations {m_gridMeshes[size]};

    bool normals {normalMode != NormalMode::none};
    auto& gridPtr = normals ? 
        gridVariations.flatNormalsMesh : gridVariations.noNormalsMesh;

    if(!gridPtr)
        gridPtr = std::make_unique<Mesh>(meshtools::generateGrid(size, normals));
    return *gridPtr;
}
Mesh MeshManager::getFromOBJ(std::string_view objString)
{
    if (!m_loadedMeshes.contains(objString))
        m_loadedMeshes[objString] = meshtools::loadFromOBJ(objString);

    return m_loadedMeshes[objString];
}

unsigned int generateVAO(const float vertices[], int verticesLength, bool normals);
unsigned int generateVAO(const float vertices[], int verticesLength, const unsigned int indices[], int indicesLength, bool normals);
std::vector<unsigned int> generateIndices(std::unique_ptr<float[]>& vertices, int& verticesLength);
void addNormals(std::unique_ptr<float[]>& vertices, int& verticesLength, const unsigned int indices[], int indicesLength);
void addNormals(std::unique_ptr<float[]>& vertices, int& verticesLength);

template <const float* vertices, int length>
Mesh makeMesh(NormalMode normals);

Mesh meshtools::generateCube(NormalMode normalMode)
{
    static constexpr float vertices[] 
    {
        -1.f, -1.f, -1.f,
        1.f, -1.f, -1.f,
        1.f,  1.f, -1.f,

        1.f,  1.f, -1.f,
        -1.f,  1.f, -1.f,
        -1.f, -1.f, -1.f,

        -1.f, -1.f,  1.f,
        1.f, -1.f,  1.f,
        1.f,  1.f,  1.f,
        
        1.f,  1.f,  1.f,
        -1.f,  1.f,  1.f,
        -1.f, -1.f,  1.f,
        
        -1.f,  1.f,  1.f,
        -1.f,  1.f, -1.f,
        -1.f, -1.f, -1.f,
        
        -1.f, -1.f, -1.f,
        -1.f, -1.f,  1.f,
        -1.f,  1.f,  1.f,
        
        1.f,  1.f,  1.f,
        1.f,  1.f, -1.f,
        1.f, -1.f, -1.f,
    
        1.f, -1.f, -1.f,
        1.f, -1.f,  1.f,
        1.f,  1.f,  1.f,
        
        -1.f, -1.f, -1.f,
        1.f, -1.f, -1.f,
        1.f, -1.f,  1.f,
        
        1.f, -1.f,  1.f,
        -1.f, -1.f,  1.f,
        -1.f, -1.f, -1.f,
        
        -1.f,  1.f, -1.f,
        1.f,  1.f, -1.f,
        1.f,  1.f,  1.f,
        
        1.f,  1.f,  1.f,
        -1.f,  1.f,  1.f,
        -1.f,  1.f, -1.f,
    };
    static constexpr int length {std::ssize(vertices)};

    return makeMesh<vertices, length>(normalMode);
}

Mesh meshtools::generateTetrahedron(NormalMode normalMode)
{
    static constexpr float vertices[]
    {
        1.f,  1.f,  1.f,
        -1.f, -1.f,  1.f,
        -1.f,  1.f, -1.f,

        1.f,  1.f,  1.f,
        -1.f,  1.f, -1.f,
        1.f, -1.f, -1.f,

        1.f,  1.f,  1.f,
        -1.f, -1.f,  1.f,
        1.f, -1.f, -1.f,

        -1.f, -1.f,  1.f,
        -1.f,  1.f, -1.f,
        1.f, -1.f, -1.f
    };
    static constexpr int length {std::ssize(vertices)};

    return makeMesh<vertices, length>(normalMode);
}

template <const float* vertices, int length>
Mesh makeMesh(NormalMode normalMode)
{
    static std::vector<unsigned int> indices;
    static std::unique_ptr<float[]> verticesForIndices;
    static int verticesForIndicesLength {length};

    if(normalMode != NormalMode::flat && indices.empty())
    {
        verticesForIndices = std::make_unique<float[]>(length);
        std::copy(vertices, vertices + length, verticesForIndices.get());
        indices = generateIndices(verticesForIndices, verticesForIndicesLength);
    }
    if(normalMode == NormalMode::smooth)
    {
        std::unique_ptr<float[]> verticesPtr = std::make_unique<float[]>(verticesForIndicesLength);
        std::copy(verticesForIndices.get(), verticesForIndices.get() + verticesForIndicesLength, verticesPtr.get());

        int tempLength {length};
        addNormals(verticesPtr, tempLength, &indices[0], std::ssize(indices));
        
        return {generateVAO(verticesPtr.get(), verticesForIndicesLength * 2,
             &indices[0], std::ssize(indices), true), static_cast<unsigned int>(indices.size())};
    }
    if(normalMode == NormalMode::flat)
    {
        std::unique_ptr<float[]> verticesPtr = std::make_unique<float[]>(length);
        std::copy(vertices, vertices + length, verticesPtr.get());

        int tempLength {length};
        addNormals(verticesPtr, tempLength);

        return {.VAO = generateVAO(verticesPtr.get(), tempLength, true), 
            .vertexCount = static_cast<unsigned int>(tempLength)};
    }

    return {generateVAO(verticesForIndices.get(), verticesForIndicesLength, &indices[0],
        std::ssize(indices), false), static_cast<unsigned int>(indices.size())};
}

Mesh meshtools::generateGrid(int gridSize, bool normals)
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

//this loader is incomplete and only works when elements are in specific order
Mesh meshtools::loadFromOBJ(std::string_view objString)
{
    auto addVerticePositions {[](const std::string& line, std::vector<float>& positions) -> void
    {
        std::size_t startIndex {};
        for(int i {}; i < 3; ++i)
        {
            startIndex = line.substr(startIndex, line.size() - startIndex).find(' ') + startIndex + 1;
            std::size_t decimalLength = line.substr(startIndex, line.size() - startIndex - 1).find(' ');
            std::istringstream valueStream(line.substr(startIndex, decimalLength));

            float position;
            valueStream >> position;
            positions.push_back(position);
        }
    }};
    std::istringstream fileStream(objString.data());
    std::string line;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> returnVertices;
    std::vector<unsigned int> indices;
    while (std::getline(fileStream, line))
    {
        line.erase(0, line.find_first_not_of(' '));
        line.erase(line.find_last_not_of(' ') + 1);
        if(line.empty() || line[0] == '#')
            continue;
        if(line.substr(0, 2) == "v ")
        {
            addVerticePositions(line, vertices);
        }
        if(line.size() > 3 && line.substr(0, 3) == "vn ")
        {
            addVerticePositions(line, normals);
        }
        if(line.substr(0, 2) == "f ")
        {
            //get vertices
            bool useTextures {line[line.find('/') + 1] != '/'};
            bool useNormals {normals.size() != 0};
            std::size_t startIndex {};
            std::vector<unsigned int> polygonIndices {}; 

            auto spacePos = line.substr(startIndex, line.size() - startIndex).find(' ');
            while(spacePos != std::string::npos)
            {
                startIndex = spacePos + startIndex + 1;
                
                std::size_t numberLength = line.substr(startIndex, line.size() - startIndex).find(useNormals ? '/' : ' ');
                std::istringstream vertexPositionStream(line.substr(startIndex, numberLength));
                unsigned int positionIndex {};
                vertexPositionStream >> positionIndex;
                --positionIndex;
                
                float vertex[6] = {vertices[positionIndex * 3], vertices[positionIndex * 3 + 1], vertices[positionIndex * 3 + 2]};

                if(useNormals)
                {
                    std::size_t normalStartIndex;
                    if(useTextures)
                    {
                        std::size_t textureStartIndex = startIndex + numberLength + 1;
                        normalStartIndex = line.substr(textureStartIndex, line.size() - textureStartIndex).find('/') + textureStartIndex + 1;
                    }
                    else normalStartIndex = startIndex + numberLength + 2;
                    std::size_t normalNumberLength = line.substr(normalStartIndex, line.size() - normalStartIndex).find(' ');
                    std::istringstream normalIndexStream(line.substr(normalStartIndex, normalNumberLength));
                    unsigned int normalIndex {};
                    normalIndexStream >> normalIndex;
                    --normalIndex;

                    vertex[3] = normals[normalIndex * 3];
                    vertex[4] = normals[normalIndex * 3 + 1];
                    vertex[5] = normals[normalIndex * 3 + 2];
                }

                auto returnVerticesIterator = std::search(returnVertices.begin(), returnVertices.end(), vertex, vertex + (useNormals ? 6 : 3));
                std::size_t index {}; 
                if(returnVerticesIterator != returnVertices.end() 
                    && std::distance(returnVertices.begin(), returnVerticesIterator) % (useNormals ? 6 : 3) == 0)
                    index = std::distance(returnVertices.begin(), returnVerticesIterator) / (useNormals ? 6 : 3);
                else
                {
                    index = returnVertices.size() / (useNormals ? 6 : 3);
                    returnVertices.insert(returnVertices.end(), vertex, vertex + (useNormals ? 6 : 3));
                }
                polygonIndices.push_back(index);

                spacePos = line.substr(startIndex, line.size() - startIndex).find(' ');
            }
            //polygon triangulation
            indices.reserve(indices.size() + (polygonIndices.size() - 2) * 3);

            for (std::size_t i = 1; i < polygonIndices.size() - 1; ++i)
            {
                indices.push_back(polygonIndices[0]);
                indices.push_back(polygonIndices[i]);
                indices.push_back(polygonIndices[i + 1]);
            }
        }
    }
    return {generateVAO(returnVertices.data(), returnVertices.size(), indices.data(), 
        indices.size(), normals.size()), static_cast<unsigned int>(indices.size())};
}

unsigned int generateVAO(const float vertices[], int verticesLength, bool normals)
{
    return generateVAO(vertices, verticesLength, nullptr, 0, normals);
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

std::vector<unsigned int> generateIndices(std::unique_ptr<float[]>& vertices, int& verticesLength)
{
    assert(verticesLength % 3 == 0 && "The number of vertices must be divisible by three");

    std::vector<glm::vec3> oldVerticePositions(verticesLength / 3);
    for (std::size_t i {}; i < verticesLength; i += 3)
    {
        oldVerticePositions[i / 3] = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    
    std::vector<glm::vec3> newVerticePositions;
    std::vector<unsigned int> indices;

    for(std::size_t i {}; i < std::ssize(oldVerticePositions); ++i)
    {
        auto iterator = std::find(newVerticePositions.begin(), newVerticePositions.end(), oldVerticePositions[i]);
        if (iterator == newVerticePositions.end())
        {
            newVerticePositions.push_back(oldVerticePositions[i]);
            indices.push_back(std::size(newVerticePositions) - 1);
            continue;
        }
        indices.push_back(static_cast<unsigned int>(std::distance(newVerticePositions.begin(), iterator)));
    }

    verticesLength = newVerticePositions.size() * 3;
    std::unique_ptr<float[]> newVertices = std::make_unique<float[]>(verticesLength); 
    for(std::size_t i {}; i < verticesLength; i += 3)
    {
        newVertices[i] = newVerticePositions[i / 3].x;
        newVertices[i + 1] = newVerticePositions[i / 3].y;
        newVertices[i + 2] = newVerticePositions[i / 3].z;
    }
    vertices = std::move(newVertices);
    return indices;
}

void addNormals(std::unique_ptr<float[]>& vertices, int& verticesLength, const unsigned int indices[], int indicesLength)
{
    assert(verticesLength % 3 == 0 && "The number of vertices must be divisible by three");
    std::vector<glm::vec3> tempVertices(verticesLength / 3);
    
    for (std::size_t i {}; i < verticesLength; i += 3)
    {
        tempVertices[i / 3] = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
    }

    std::vector<glm::vec3> normals(tempVertices.size(), glm::vec3(0.0f));

    for (std::size_t i {}; i < indicesLength; i += 3)
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
    for (std::size_t i {}, j {}; i < verticesLength; i += 3, j += 6)
    {
        newVertices[j] = vertices[i];
        newVertices[j + 1] = vertices[i + 1];
        newVertices[j + 2] = vertices[i + 2];
        newVertices[j + 3] = normals[i / 3].x;
        newVertices[j + 4] = normals[i / 3].y;
        newVertices[j + 5] = normals[i / 3].z;
    }

    verticesLength *= 2;
    vertices = std::move(newVertices);
}

void addNormals(std::unique_ptr<float[]>& vertices, int& verticesLength)
{
    assert(verticesLength % 3 == 0 && "The number of vertices must be divisible by three");
    std::unique_ptr<float[]> newVertices {std::make_unique<float[]>(verticesLength * 2)};
    std::vector<glm::vec3> normals {};

    for (std::size_t i {}; i < verticesLength; i += 9)
    {
        glm::vec3 v0(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 v1(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        glm::vec3 v2(vertices[i + 6], vertices[i + 7], vertices[i + 8]);

        glm::vec3 v1_v0 = v1 - v0;
        glm::vec3 v2_v0 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(v1_v0, v2_v0));

        normals.push_back(faceNormal);
    }

    int normalIndex {};
    for(std::size_t i {}, j {}; i < (verticesLength * 2); i += 6, j += 3)
    {
        newVertices[i] = vertices[j];
        newVertices[i + 1] = vertices[j + 1];
        newVertices[i + 2] = vertices[j + 2];
        newVertices[i + 3] = normals[normalIndex].x;
        newVertices[i + 4] = normals[normalIndex].y;
        newVertices[i + 5] = normals[normalIndex].z;

        if((j + 3) % 9 == 0) ++normalIndex;
    }

    verticesLength *= 2;
    vertices = std::move(newVertices);
}