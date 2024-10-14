#include <engine/objectManagement.hpp>
#include <game/gameObject.hpp>
#include <game/game.hpp>

Game::Game(bool onePlayer) : m_onePlayer(onePlayer)
{
    // m_waterObj = std::make_unique<GameObject>(meshManagerInstance.getGrid(16, NormalMode::flat), waterShader);
    // m_waterObj->setTransform();
    // Material aircraftCarrierMaterial {glm::vec3(.4f, .4f, .4f), .3f, 150.f, .6f};
    // m_aircraftCarrier = std::make_unique<GameObject>(meshManagerInstance.getFromOBJ("../assets/models/sphereMixed.obj"), basicShader, aircraftCarrierMaterial);
}

Game::~Game() 
{

}