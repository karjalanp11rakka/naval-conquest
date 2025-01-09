#pragma once

#include <string_view>
#include <map>
#include <memory>
#include <utility>
#include <functional>

#include <engine/shader.hpp>

class Shader;

struct StringViewPairHash
{
    std::size_t operator()(const std::pair<const std::string_view, const std::string_view> sp) const
    {
        std::size_t hash1 = std::hash<std::string_view>()(sp.first);
        std::size_t hash2 = std::hash<std::string_view>()(sp.second);

        return hash1 ^ (hash2 << 1); 
    }
};
struct StringViewPairEqual 
{
    bool operator()(const std::pair<const std::string_view, const std::string_view>& first,
                    const std::pair<const std::string_view, const std::string_view>& second) const {
    return first.first == second.first && first.second == second.second;    }
};

class ShaderManager
{
private:
    ShaderManager() {}
    ~ShaderManager();
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager& other) = delete;

    std::unordered_map<std::pair<std::string_view, std::string_view>, Shader, StringViewPairHash, StringViewPairEqual> m_shaders;
public:
    static ShaderManager& getInstance()
    {
        static ShaderManager instance;
        return instance;
    }

    Shader* getShader(std::string_view vertexString, std::string_view fragmentString);
};