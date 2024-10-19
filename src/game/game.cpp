#include <utility>
#include <cstddef>

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include <engine/objectManagement.hpp>
#include <game/gameObject.hpp>
#include <game/game.hpp>
#include <engine/meshManager.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <game/random.hpp>
#include <assets.hpp>

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices);

Game::Game(bool onePlayer) : m_onePlayer(onePlayer)
{
    static MeshManager& meshManagerInstance {MeshManager::getInstance()};
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};

    m_waterObj = std::make_unique<Object3D>(meshManagerInstance.getGrid(GRID_SIZE, NormalMode::flat), 
        ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FWATER_GLSL));
    glm::mat4 waterModel(1.f);
    renderEngineInstance.addObject(m_waterObj.get());
    
    waterModel = glm::rotate(waterModel, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    m_waterObj->setModel(std::move(waterModel));

    m_grid[0][0] = std::make_unique<AircraftCarrier>(true);
    m_grid[0][1] = std::make_unique<AircraftCarrier>(false);
    m_grid[0][1]->setTransform({gridIndicesToPosition(std::make_pair(Random::getInstance().get(0, 10), 5))});
    m_grid[0][0]->setTransform({gridIndicesToPosition(std::make_pair(1, 7))});
}

Game::~Game()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.removeObject(m_waterObj.get());
}

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices)
{
    static const float squareSize {2.f / GRID_SIZE};

    return glm::vec3(-1.f + squareSize * gridIndices.first + squareSize / 2.f, 0.f, 
        -1.f + squareSize * gridIndices.second + squareSize / 2.f);
}