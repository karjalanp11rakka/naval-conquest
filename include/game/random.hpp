#pragma once

#include <random>
#include <concepts>

template<typename T>
concept IsIntegral = std::is_integral_v<T>;

class Random
{
private:
    Random();
    Random(const Random&) = delete;
    Random& operator=(const Random& other) = delete;
    std::mt19937 m_mt {};
public:
    static Random& getInstance()
    {
        static Random instance;
        return instance;
    }
    template<IsIntegral T>
    T get(T min, T max)
    {
        auto dist {std::uniform_int_distribution<T>(min, max)};
        return dist(m_mt);
    }
};