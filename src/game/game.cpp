#include <utility>

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include <engine/objectManagement.hpp>
#include <game/gameObject.hpp>
#include <game/game.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <game/uiManager.hpp>
#include <game/random.hpp>
#include <assets.hpp>

template<std::size_t N>
GameGrid<N>::~GameGrid() {} 

template<std::size_t N>
GameObject* GameGrid<N>::at(std::size_t x, std::size_t y) const
{
    return m_base[x + y * GRID_SIZE].get();
}

template<std::size_t N>
void GameGrid<N>::destroyAt(std::size_t x, std::size_t y)
{
    m_base[x + y * GRID_SIZE].reset();
}

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices);

void Game::activatePlayerSquares()
{
    static UIManager& uiManagerInstance {UIManager::getInstance()};
    std::bitset<GRID_SIZE*GRID_SIZE> setSquares {};
    for(int i {}; i < m_grid.size(); ++i)
    {
        if(m_grid[i] && m_grid[i]->isTeamOne() == m_playerOneTwoPlay)
            setSquares.set(i);
    }

    uiManagerInstance.setGridSquares(std::move(setSquares));
}

Game::Game(bool onePlayer) : m_onePlayer(onePlayer)
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    static UIManager& uiManagerInstance {UIManager::getInstance()};

    m_grid.initializeAt<AircraftCarrier>(0, 0, true);
    m_grid.initializeAt<AircraftCarrier>(9, 9, true);
    m_grid.initializeAt<AircraftCarrier>(8, 9, true);
    auto r = Random::getInstance().get<std::size_t>(0, GRID_SIZE - 1);
    m_grid.initializeAt<AircraftCarrier>(2, r, false);

    uiManagerInstance.setGameSquareCallback([&](std::size_t)
    {
        uiManagerInstance.enableGameActionButtons();
    });
    activatePlayerSquares();
    uiManagerInstance.disableGameActionButtons();
}

Game::~Game() {}

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices)
{
    static const float squareSize {2.f / GRID_SIZE};

    return glm::vec3(-1.f + squareSize * gridIndices.first + squareSize / 2.f, 0.f, 
        -1.f + squareSize * gridIndices.second + squareSize / 2.f);
}