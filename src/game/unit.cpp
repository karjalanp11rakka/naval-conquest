#include <utility>
#include <cassert>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/renderEngine.hpp>
#include <game/unit.hpp>
#include <game/game.hpp>
#include <engine/shaderManager.hpp>
#include <engine/meshManager.hpp>
#include <game/uiManager.hpp>
#include <assets.hpp>

std::pair<std::size_t, std::size_t> Action::getSelectedUnitIndices(Game* gameInstance) const
{
    return gameInstance->m_selectedUnitIndices.value();
}
GameGrid& Action::getGameGrid(Game* gameInstance) const
{
    return gameInstance->m_grid;
}

using IndicesList = std::vector<std::pair<std::size_t, std::size_t>>;

template<int Radius>
void SelectOnGridAction<Radius>::setGameGridSquares(IndicesList&& activeSquares)
{
    static UIManager& uiManagerInstance {UIManager::getInstance()};
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares;

    for(auto pair : activeSquares)
        setSquares.set(pair.first + pair.second * GRID_SIZE);

    uiManagerInstance.setGameGridSquares(std::move(setSquares));
}
template<int Radius>
void SelectOnGridAction<Radius>::use(Game* gameInstance)
{
    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto indices = this->getSelectedUnitIndices(gameInstance);
    IndicesList squaresToActivate;

    auto addDirections = [&squaresToActivate, indices, &gameGrid](bool vertical)
    {
        IndicesList newIndices;
        std::size_t processedIndex = vertical ? indices.first : indices.second;

        for(std::size_t i {static_cast<std::size_t>(std::max(static_cast<int>(processedIndex) - Radius, 0))}; 
            i < std::min(static_cast<std::size_t>(GRID_SIZE), processedIndex + Radius + 1); ++i)
        {
            if(i == processedIndex) continue;
            std::pair<std::size_t, std::size_t> currentPos(vertical ? i : indices.first, vertical ? indices.second : i);
            if(gameGrid.at(currentPos))
            {
                if(i < processedIndex) 
                {
                    newIndices.clear();
                    i = processedIndex - 1;
                }
                else break;
            }
            else newIndices.push_back(std::move(currentPos));   
        }
        squaresToActivate.insert(squaresToActivate.end(), 
            std::make_move_iterator(newIndices.begin()), std::make_move_iterator(newIndices.end()));
    };
    addDirections(true);
    addDirections(false);
    this->setGameGridSquares(std::move(squaresToActivate));
}
template<int Radius>
void MoveAction<Radius>::callback(Game* gameInstance, std::size_t x, std::size_t y) const
{
    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto selectedIndices = this->getSelectedUnitIndices(gameInstance);
    gameGrid.moveAt(selectedIndices.first, selectedIndices.second, x, y);
}

void Unit::updateModelMatrix()
{
    m_model = glm::mat4(1.f);
    m_model *= glm::mat4_cast(m_transform.rotation);
    m_model = glm::translate(m_model, m_transform.position);
    m_model = glm::scale(m_model, m_transform.scale);
}

Unit::Unit(Game* gameInstance, Mesh mesh, Shader* shader, const Material& material, bool teamOne, std::vector<Action*>&& actions)
    : m_gameInstance(gameInstance), LitObject(mesh, shader, material), m_teamOne(teamOne), m_actions(std::move(actions))
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.addObject(this);

    m_actionNames.resize(m_actions.size());
    assert(m_actions.size() < GAME_ACTION_BUTTONS_COUNT && "Unit cannot have more actions than there are buttons");//the first button is back button so < instead of <=
    std::transform(m_actions.begin(), m_actions.end(), m_actionNames.begin(), [](Action* action)
    {
        return action->getName();
    }); 
}

Unit::~Unit()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};
    renderEngineInstance.removeObject(this);
}
Action& Unit::getAction(std::size_t actionIndex)
{
    assert(m_actions[actionIndex]);
    return *m_actions[actionIndex];
}
void Unit::setTransform(Transform&& transform)
{
    m_transform = std::move(transform);
    updateModelMatrix();
}
static constexpr int AIRCRAFT_CARRIER_MOVE_RADIUS = 2;
static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE {glm::vec3(.3f, .3f, .6f), .3f, 150.f, .6f};
static constexpr Material AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO {glm::vec3(.6f, .3f, .3f), .3f, 150.f, .6f};
AircraftCarrier::AircraftCarrier(Game* gameInstance, bool teamOne) 
    : Unit(gameInstance, MeshManager::getInstance().getFromOBJ(assets::MODELS_AIRCRAFT_CARRIER_OBJ),
    ShaderManager::getInstance().getShader(assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL),
    teamOne ? AIRCRAFT_CARRIER_MATERIAL_TEAM_ONE : AIRCRAFT_CARRIER_MATERIAL_TEAM_TWO, teamOne, 
    {&MoveAction<AIRCRAFT_CARRIER_MOVE_RADIUS>::getInstance()})
{
    updateModelMatrix();
}