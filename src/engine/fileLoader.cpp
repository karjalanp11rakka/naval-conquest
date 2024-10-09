#include <iostream>
#include <fstream>
#include <sstream>

#include <engine/fileLoading.hpp>

std::string loadFile(const std::string& filePath)
{
    std::ifstream file {};
 
    file.open(filePath);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file at path: " << filePath << std::endl;
        return ""; 
    }

    std::stringstream fileStream {};

    fileStream << file.rdbuf();
    file.close();
    return fileStream.str();
}