//This file is as a library for 'assets_converter.py'. It takes OBJ input and generates strings representing std::vectors, which are then pasted into 'assets.hpp' by 'assets_converter.py'.
//Compiled with g++ into 'objLoader.so'
//g++ -shared -fPIC -o objLoader.so objLoader.cpp

#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <cmath>

std::vector<char*> outputs;

extern "C" {
char* loadFromObj(const char* obj, const char* name)
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
    std::istringstream fileStream(obj);
    std::string line;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> finalVertices;
    std::vector<unsigned int> indices;
    std::ostringstream output;
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

                auto returnVerticesIterator = std::search(finalVertices.begin(), finalVertices.end(), vertex, vertex + (useNormals ? 6 : 3));
                std::size_t index {}; 
                if(returnVerticesIterator != finalVertices.end() 
                    && std::distance(finalVertices.begin(), returnVerticesIterator) % (useNormals ? 6 : 3) == 0)
                    index = std::distance(finalVertices.begin(), returnVerticesIterator) / (useNormals ? 6 : 3);
                else
                {
                    index = finalVertices.size() / (useNormals ? 6 : 3);
                    finalVertices.insert(finalVertices.end(), vertex, vertex + (useNormals ? 6 : 3));
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
    auto returnVerticesSize = finalVertices.size();
    auto indicesSize = indices.size();
    output << "#define " << name << " _" << name << "_VERTICES, _" << name << "_INDICES, " << (normals.size() == 0 ? "false\n" : "true\n"); 
    output << "inline constexpr std::array<float, " << returnVerticesSize << "> _" << name << "_VERTICES {";
    for(std::size_t i {}; i < returnVerticesSize; ++i)
    {
        float data = finalVertices[i]; 
        if(std::floor(data) == data)
            output << std::fixed << std::setprecision(1) << data << "f";
        else output << std::fixed << std::setprecision(6) << data << "f";
        if(i != returnVerticesSize - 1) output << ", ";
    }
    output << "};\n";
    output << "inline constexpr std::array<unsigned int, " << indicesSize << "> _" << name << "_INDICES {";
    for(std::size_t i {}; i < indicesSize; ++i)
    {
        output << indices[i];
        if(i != indicesSize - 1) output << ", ";
    }
    output << "};\n";
    std::string resultStr = output.str();
    char* result = new char[resultStr.size() + 1];
    outputs.push_back(result);
    std::copy(resultStr.begin(), resultStr.end(), result);
    result[resultStr.size()] = '\0';
    return result;
}
void freeData()
{
    for(auto output : outputs) delete[] output;
}
}

