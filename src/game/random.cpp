#include <glfwController.hpp>
#include <game/random.hpp>

Random::Random()
{
    std::random_device rd {};
    std::seed_seq ss = {static_cast<std::seed_seq::result_type>(GLFWController::getInstance().getTime()),
        rd(), rd(), rd(), rd(), rd(), rd(), rd()};

    m_mt = std::mt19937(ss);
}