#pragma once

#include <string>
#include <map>
#include <memory>
#include <utility>
#include <functional>

#include <engine/shader.hpp>

class Shader;

struct StringPtrPairHash
{
    std::size_t operator()(const std::pair<const std::string*, const std::string*>& sp) const
    {
        std::size_t hash1 = std::hash<std::string>()(*sp.first);
        std::size_t hash2 = std::hash<std::string>()(*sp.second);

        return hash1 ^ (hash2 << 1); 
    }
};
struct StringPtrPairEqual 
{
    bool operator()(const std::pair<const std::string*, const std::string*>& first,
                    const std::pair<const std::string*, const std::string*>& second) const {
        return first == first && second == second;
    }
};

class ShaderManager
{
private:
    ShaderManager() {}
    ~ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    std::unordered_map<std::pair<const std::string*, const std::string*>, Shader, StringPtrPairHash, StringPtrPairEqual> m_shaders {};
public:
    static ShaderManager& getInstance()
    {
        static ShaderManager instance {ShaderManager()};
        return instance;
    }

    Shader* getShader(const std::string& vertexString, const std::string& fragmentString);
};