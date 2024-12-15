#include <iterator>
#include <algorithm>
#include <cmath>
#include <set>
#include <limits>

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include <engine/object.hpp>
#include <game/unitObject.hpp>
#include <game/action.hpp>
#include <game/game.hpp>
#include <engine/renderEngine.hpp>
#include <engine/shaderManager.hpp>
#include <game/uiManager.hpp>
#include <game/random.hpp>
#include <game/uiPreset.hpp>
#include <engine/camera.hpp>
#include <assets.hpp>
#include <glfwController.hpp>

void GameGrid::destroyAt(std::size_t index)
{
    for(auto it = m_combinedLocations.begin(); it != m_combinedLocations.end(); ++it)
    {
        if(it->first.count(index))
        {
            it->second->reset();
            m_combinedLocations.erase(it);
            return;
        }
    }

    m_base[index].reset();
}
UnitObject* GameGrid::at(std::size_t x, std::size_t y) const
{
    std::size_t index = x + y * GRID_SIZE;
    for(auto& comb : m_combinedLocations)
    {
        if(comb.first.count(index)) return comb.second->get();
    }
    return m_base[index].get();
}
UnitObject* GameGrid::at(Loc loc) const
{
    return at(loc.first, loc.second);
}

void GameGrid::update()
{
    if(m_movements.empty()) return;
    static GLFWController& glfwControllerInstance = GLFWController::getInstance();
    float deltaTime = glfwControllerInstance.getDeltaTime();

    for(auto& moveData : m_movements)
    {
        moveData.currentTime -= deltaTime;
        if(moveData.currentTime <= 0.f)
        {
            moveData.currentTime = moveData.speed;
            moveData.lastPos = gridLocationToPosition(moveData.path.front());
            moveData.path.pop_front();
            if(moveData.path.empty())
            {
                moveData.moveObject->setRotation({});

                moveData.moveObject->setPosition(moveData.lastPos);
                moveData.moveObject = nullptr;
                continue;
            }

            glm::quat rotation = glm::quatLookAt(glm::normalize(gridLocationToPosition(moveData.path.front()) - moveData.lastPos), glm::vec3(0.f, 1.f, 0.f))
                //offset
                * glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));

            moveData.moveObject->setRotation(rotation);

            moveData.currentTime = moveData.speed;
            continue;
        }
        float ratio = moveData.currentTime / moveData.speed;
        float otherRatio = 1.f - ratio;
        auto currentPos
            = moveData.lastPos * ratio + gridLocationToPosition(moveData.path.front()) * otherRatio;
        moveData.moveObject->setPosition(currentPos);
    }
    m_movements.erase(std::remove_if(m_movements.begin(), m_movements.end(),
        [](const auto& data){return data.moveObject == nullptr;}), m_movements.end());
}
void GameGrid::destroyAt(std::size_t x, std::size_t y)
{
    destroyAt(x + y * GRID_SIZE);
}
void GameGrid::destroyAt(Loc loc)
{
    destroyAt(loc.first + loc.second * GRID_SIZE);
}
void GameGrid::moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2)
{
    auto& startPtr = m_base[x1 + y1 * GRID_SIZE], & movePtr = m_base[x2 + y2 * GRID_SIZE];
    assert(dynamic_cast<Base*>(startPtr.get()) && "Base cannot be moved");
    assert(dynamic_cast<Base*>(movePtr.get()) && "Base cannot be destroyed by moving");
    movePtr = std::move(startPtr);
    startPtr.reset();
    movePtr->setPosition(gridLocationToPosition(std::make_pair(x2, y2)));
}
void GameGrid::moveAt(Loc loc1, Loc loc2)
{
    moveAt(loc1.first, loc1.second, loc2.first, loc2.second); 
}
std::optional<std::unordered_set<std::size_t>*> GameGrid::getCombinedLocations(std::size_t index)
{
    for(auto& comb : m_combinedLocations)
    {
        if(comb.first.count(index)) return &comb.first;
    }
    return {};
}
GameGrid::Path GameGrid::findPath(Loc startLoc, Loc moveLoc, bool avoidObstacles)//A*
{
    if(startLoc == moveLoc)
    {
        Path returnValue;
        returnValue.push_front(startLoc);
        return returnValue;
    }
    struct PathNode
    {
        int gCost {std::numeric_limits<int>::max()};
        int hCost {};
        int fCost {};
        Loc loc;
        PathNode* parentNode {};
        void calculateFCost()
        {
            fCost = hCost + gCost;
        }
    };
    struct ComparePathNodes
    {
        bool operator()(const PathNode* lhs, const PathNode* rhs) const
        {
            return lhs->fCost < rhs->fCost;
        }
    };
    
    static constexpr int MOVE_STRAIGHT_COST = 10;
    static constexpr int MOVE_DIAGONAL_COST = 14;
    auto calculateDistanceCost = [](PathNode* pathNode1, PathNode* pathNode2) -> int
    {
        int xDistance = std::abs(static_cast<int>(pathNode1->loc.first) - static_cast<int>(pathNode2->loc.first));
        int yDistance = std::abs(static_cast<int>(pathNode1->loc.second) - static_cast<int>(pathNode2->loc.second));
        return MOVE_DIAGONAL_COST * std::min(xDistance, yDistance) + MOVE_STRAIGHT_COST * std::abs(xDistance - yDistance);
    };
    
    auto validLocIndex = [](int x) -> bool {return x < GRID_SIZE && x >= 0;};
    
    std::array<std::unique_ptr<PathNode>, GRID_SIZE * GRID_SIZE> nodes;

    auto initializeNode = [&nodes](Loc loc) -> PathNode*
    {
        std::size_t index = loc.first + loc.second * GRID_SIZE;
        auto node = std::make_unique<PathNode>(); 
        node->calculateFCost();
        node->loc = loc;
        nodes[index] = std::move(node);
        return nodes[index].get();
    };

    PathNode* startNode {};
    std::size_t startNodeIndex = convertLocationToIndex(startLoc);
    nodes[startNodeIndex] = std::make_unique<PathNode>(0);
    startNode = nodes[startNodeIndex].get();
    startNode->loc = startLoc;
    PathNode* endNode = initializeNode(moveLoc);

    startNode->hCost = calculateDistanceCost(startNode, endNode);

    std::set<PathNode*, ComparePathNodes> openList;
    openList.insert(startNode);
    std::unordered_set<PathNode*> closedList;

    while (!openList.empty())
    {
        auto currentNode = *openList.begin();
        //found
        if(currentNode == endNode)
        {
            Path returnValue;
            returnValue.push_front(currentNode->loc);
            while (currentNode->parentNode != nullptr)
            {
                currentNode = currentNode->parentNode;
                returnValue.push_front(currentNode->loc);
            }
            return returnValue;
        }
        openList.erase(currentNode);
        closedList.insert(currentNode);

        static constexpr std::array<std::pair<int, int>, 8> directions =
        {
            std::make_pair(1, -1),
            std::make_pair(1, 1),
            std::make_pair(-1, 1),
            std::make_pair(-1, -1),
            std::make_pair(0, -1),
            std::make_pair(0, 1),
            std::make_pair(-1, 0),
            std::make_pair(1, 0)
        };
        for(auto dir : directions)
        {
            auto neighborLoc = std::make_pair(currentNode->loc.first + dir.first, currentNode->loc.second + dir.second);
            if(!validLocIndex(neighborLoc.first) || !validLocIndex(neighborLoc.second) || (avoidObstacles && at(neighborLoc.first, neighborLoc.second))) continue;
            std::size_t index = convertLocationToIndex(neighborLoc);
            PathNode* neighbourNode;
            if(!nodes[index]) neighbourNode = initializeNode(neighborLoc);
            else neighbourNode = nodes[index].get();
            if(closedList.count(neighbourNode)) continue;

            int tentativeGCost = currentNode->gCost + calculateDistanceCost(currentNode, neighbourNode);
            if(tentativeGCost < neighbourNode->gCost)
            {
                neighbourNode->parentNode = currentNode;
                neighbourNode->gCost = tentativeGCost;
                neighbourNode->hCost = calculateDistanceCost(neighbourNode, endNode);
                neighbourNode->calculateFCost();

                openList.erase(neighbourNode);//remove when it's already included so it gets correctly sorted position in the set
                openList.insert(neighbourNode);
            }
        }
    }
    //no path found
    return {};
}
int GameGrid::moveAlongPath(Path&& path, float speed, bool useRotation)
{
    auto startLoc = path.front();
    auto moveObj = at(startLoc);
    m_base[convertLocationToIndex(path.back())] = std::move(m_base[convertLocationToIndex(startLoc)]);
    m_base[convertLocationToIndex(startLoc)].reset();
    assert(moveObj != nullptr && "There has to be an object to be moved");

    int pathLength = path.size();
    m_movements.emplace_back(moveObj, std::move(path), speed, useRotation);
    return pathLength;
}

UnitObject* GameGrid::operator[](std::size_t index) const noexcept
{
    for(auto& comb : m_combinedLocations)
    {
        if(comb.first.count(index)) return comb.second->get();
    }
    return m_base[index].get();
}
GameGrid::Loc GameGrid::convertIndexToLocation(std::size_t index)
{
    return std::make_pair(index % GRID_SIZE, index / GRID_SIZE);
}
std::size_t GameGrid::convertLocationToIndex(Loc loc)
{
    return loc.first + loc.second * GRID_SIZE;
}
glm::vec3 GameGrid::gridLocationToPosition(Loc loc)
{
    return glm::vec3(-1.f + SQUARE_SIZE * loc.first + SQUARE_SIZE / 2.f, 0.f, 
        -1.f + SQUARE_SIZE * loc.second + SQUARE_SIZE / 2.f);
}
// float GameGrid::distance(Loc loc1, Loc loc2)
// {
//     // d = √((|x1−x2|)² + (|y1​−y2|)²)
//     return std::sqrt(std::pow(std::abs(static_cast<int>(loc1.first) - static_cast<int>(loc2.first)), 2) 
//         + std::pow(std::abs(static_cast<int>(loc1.second) - static_cast<int>(loc2.second)), 2));
// }

void Game::activatePlayerSquares()
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares {};
    std::bitset<GRID_SIZE * GRID_SIZE / 2> setSquaresLarge {};
    std::unordered_set<std::unordered_set<std::size_t>*> combinedIndices;
    for(int i {}; i < m_grid.size(); ++i)
    {
        auto combinedLocations = m_grid.getCombinedLocations(i);
        if(combinedLocations)
        {
            if(combinedIndices.count(combinedLocations.value())) continue;
            combinedIndices.insert(combinedLocations.value());
        } 
        if(m_grid[i] && m_grid[i]->isTeamOne() == m_playerOneToPlay)
        {
            if(combinedLocations)
            {
                setSquaresLarge.set(i / 2);
            } 
            else setSquares.set(i);
        }
    }

    uiManagerInstance.setGameGridSquares(std::move(setSquares), std::move(setSquaresLarge));
}

Game::Game(bool onePlayer) : m_grid(this), m_onePlayer(onePlayer)
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    static UIManager& uiManagerInstance = UIManager::getInstance();
    updateStatusTexts();
    m_grid.initializeAt<AircraftCarrierUnit>(0, 0, true);
    m_grid.initializeAt<Base>(14, 14, true);
    m_grid.initializeAt<Base>(14, 2, false);
    m_grid.initializeAt<AircraftCarrierUnit>(9, 9, true);
    m_grid.initializeAt<SubmarineUnit>(9, 3, true);
    m_grid.initializeAt<AircraftCarrierUnit>(8, 9, true);
    auto r = Random::getInstance().get<std::size_t>(0, GRID_SIZE - 1);
    m_grid.initializeAt<AircraftCarrierUnit>(2, r, false);
    m_grid.initializeAt<SubmarineUnit>(1, 2, false);
    activatePlayerSquares();
    uiManagerInstance.disableGameActionButtons(true);
}
void Game::update()
{
    m_grid.update();
    if(m_cooldown)
    {
        static GLFWController& glfwControllerInstance = GLFWController::getInstance();
        float deltaTime = glfwControllerInstance.getDeltaTime();
        auto& cooldownVal = m_cooldown.value();
        cooldownVal.first -= deltaTime;
        if(cooldownVal.first <= 0.f)
        {
            cooldownVal.second();
            m_cooldown.reset();
        }
    }
}
std::int32_t Game::getMoney() const
{
    return m_playerOneToPlay ? m_playerData.first.money : m_playerData.second.money;
}
void Game::addMoney(std::int32_t money)
{
    if(m_playerOneToPlay) m_playerData.first.money += money;
    else m_playerData.second.money -= money;
    updateStatusTexts();
}
void Game::takeMove()
{
    if(m_playerOneToPlay) --m_playerData.first.moves.first;
    else --m_playerData.second.moves.first;
    updateStatusTexts();
}
bool Game::canMove()
{
    if(m_playerOneToPlay) return m_playerData.first.moves.first;
    else return m_playerData.second.moves.first;
}
void Game::updateStatusTexts()
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.updateGameStatusTexts(m_playerOneToPlay ? 
        m_playerData.first : m_playerData.second, m_playerOneToPlay);
}
void Game::receiveGameInput(std::size_t index, ButtonTypes buttonType)
{
    if(m_cooldown) return;
    static UIManager& uiManagerInstance = UIManager::getInstance();
    static SelectSquareCallbackManager& actionCallbackManagerInstance = SelectSquareCallbackManager::getInstance();
    static GameController& gameControllerInstance = GameController::getInstance();
    auto returnToPlayerUnitSelection = [&]()
    {
        m_selectedUnitIndices.reset();
        activatePlayerSquares();
        uiManagerInstance.disableGameActionButtons(true);
        uiManagerInstance.retrieveSavedSelection();
    };
    switch (buttonType)
    {
    case ButtonTypes::ActionButton:
        assert(m_selectedUnitIndices);
        if(index == 0)
        {
            if(m_selectedActionIndex)
            {
                uiManagerInstance.enableGameActionButtons(m_grid.at(m_selectedUnitIndices.value())->getActionData());
                uiManagerInstance.setGameGridSquares({});
                m_selectedActionIndex.reset();
                uiManagerInstance.retrieveSavedSelection();
            }
            else
            {
                gameControllerInstance.getCamera()->stopZoom();
                returnToPlayerUnitSelection();
                actionCallbackManagerInstance.reset();
            }
        }
        else
        {
            m_selectedActionIndex = index - 1;
            auto actionType = m_grid.at(m_selectedUnitIndices.value())->useAction(m_selectedActionIndex.value());
            switch (actionType)
            {
            case ActionTypes::selectSquare:
                uiManagerInstance.saveCurrentSelection();
                uiManagerInstance.disableGameActionButtons(false);
                gameControllerInstance.getCamera()->stopZoom();
                break;
            case ActionTypes::immediate:
                m_selectedActionIndex.reset();
                gameControllerInstance.getCamera()->stopZoom();
                returnToPlayerUnitSelection();
                break;
            case ActionTypes::nothing:
                m_selectedActionIndex.reset();
                break;
            }
        }
        break;
    
    case ButtonTypes::GridSquare:
        auto selectedActionSquare = GameGrid::convertIndexToLocation(index);
        if(m_selectedActionIndex)
        {
            float cooldown {};
            actionCallbackManagerInstance.invoke(this, selectedActionSquare.first, selectedActionSquare.second, cooldown);
            uiManagerInstance.removeDisabledColorToGridSquare(GameGrid::convertLocationToIndex(m_selectedUnitIndices.value()));
            uiManagerInstance.removeSavedSelection();
            
            m_selectedUnitIndices.reset();
            uiManagerInstance.disableGameActionButtons(true);
            uiManagerInstance.setGameGridSquares({});

            m_selectedActionIndex.reset();
            //cooldown
            m_cooldown = std::make_pair(cooldown, [&]()
            {
                activatePlayerSquares();
                uiManagerInstance.retrieveSavedSelection();
            });
        }
        else
        {
            gameControllerInstance.getCamera()->zoom(m_grid[index]->getPosition(), .3f, .5f, 1.f);
            uiManagerInstance.saveCurrentSelection();
            static constexpr auto SELECTED_GRID_SQUARE_COLOR = glm::vec3(.7f, .9f, .2f);
            m_selectedUnitIndices = selectedActionSquare;
            uiManagerInstance.enableGameActionButtons(m_grid[index]->getActionData());
            uiManagerInstance.addDisabledColorToGridSquare(index, SELECTED_GRID_SQUARE_COLOR);
            uiManagerInstance.setGameGridSquares({});
        }
        break;
    }
}