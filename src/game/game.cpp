#include <iterator>
#include <algorithm>
#include <cmath>
#include <limits>
#include <format>

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include <engine/object.hpp>
#include <game/gridObject.hpp>
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

bool GameGrid::update(float deltaTime)
{
    constexpr auto addY = [](glm::vec3 vec, float y) -> glm::vec3
    {
        return glm::vec3(vec.x, y, vec.z);
    };
    for(auto& moveData : m_movements)
    {
        moveData.currentTime -= deltaTime;
        float y = moveData.moveObject->getPosition().y;
        if(moveData.currentTime <= 0.f)
        {
            moveData.currentTime = moveData.speed;
            moveData.lastPos = gridLocationToPosition(moveData.path.front());
            moveData.path.pop_front();
            if(moveData.path.empty())
            {
                if(moveData.resetRotationOnEnd) moveData.moveObject->setRotation({});
                moveData.moveObject->setPosition(addY(moveData.lastPos, y));
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
        auto currentPos = 
            moveData.lastPos * ratio + gridLocationToPosition(moveData.path.front()) * otherRatio;
        moveData.moveObject->setPosition(addY(currentPos, y));
    }
    m_movements.erase(std::remove_if(m_movements.begin(), m_movements.end(),
        [](const auto& data){return data.moveObject == nullptr;}), m_movements.end());
    if(m_movements.empty()) return true;
    return false;
}
GameGrid::GameGrid(Game* gameInstance) : m_gameInstance(gameInstance) {}
GridObject* GameGrid::at(std::size_t x, std::size_t y) const
{
    return this->operator[](x + y * GRID_SIZE).first;
}
GridObject* GameGrid::at(Loc loc) const
{
    return this->operator[](loc.first + loc.second * GRID_SIZE).first;
}
void GameGrid::destroy(GridObject* ptr)
{
    for(auto it = m_combinedLocations.begin(); it != m_combinedLocations.end(); ++it)
    {
        if(it->second->get() == ptr)
        {
            it->second->reset();
            m_combinedLocations.erase(it);
            return;
        }
    }
    std::find_if(m_base.begin(), m_base.end(), [ptr](auto& unitObject) -> bool
    {
        return unitObject.get() == ptr;
    })->reset();
}
void GameGrid::destroyAt(Loc loc)
{
    destroyAt(loc.first + loc.second * GRID_SIZE);
}
void GameGrid::destroyAt(std::size_t index)
{
    for(auto it = m_combinedLocations.begin(); it != m_combinedLocations.end(); ++it)
    {
        if(std::find(it->first.begin(), it->first.end(), index) != it->first.end())
        {
            it->second->reset();
            m_combinedLocations.erase(it);
            return;
        }
    }

    m_base[index].reset();
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
    nodes[startNodeIndex] = std::make_unique<PathNode>();
    startNode = nodes[startNodeIndex].get();
    startNode->loc = startLoc;
    startNode->gCost = 0;

    startNode = nodes[startNodeIndex].get();
    startNode->loc = startLoc;
    PathNode* endNode = initializeNode(moveLoc);

    startNode->hCost = calculateDistanceCost(startNode, endNode);
    startNode->calculateFCost();

    std::set<PathNode*, ComparePathNodes> openList;
    openList.insert(startNode);
    std::unordered_set<PathNode*> closedList;

    GridObject* startLocObject = at(startLoc), * moveLocUnit = at(moveLoc);//not considered as an obstacle
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
            
            if(!validLocIndex(neighborLoc.first) || !validLocIndex(neighborLoc.second))
                continue;
            if(avoidObstacles)
            {
                auto currentPosObject = at(neighborLoc);
                if(currentPosObject && currentPosObject != startLocObject && currentPosObject != moveLocUnit) continue;
            }
            std::size_t index = convertLocationToIndex(neighborLoc);
            PathNode* neighbourNode;
            if(!nodes[index]) neighbourNode = initializeNode(neighborLoc);
            else neighbourNode = nodes[index].get();
            if(closedList.contains(neighbourNode)) continue;

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
int GameGrid::moveAlongPath(Path&& path, float speed, bool resetRotationOnEnd)
{
    auto startLoc = path.front();
    auto moveObj = at(startLoc);
    m_base[convertLocationToIndex(path.back())] = std::move(m_base[convertLocationToIndex(startLoc)]);
    m_base[convertLocationToIndex(startLoc)].reset();
    assert(moveObj != nullptr && "There has to be an object to be moved");
    return moveAlongPath(std::move(path), speed, moveObj, resetRotationOnEnd);
}
int GameGrid::moveAlongPath(Path&& path, float speed, GameObject* moveObject, bool resetRotationOnEnd)
{
    int pathLength = path.size();
    if(m_movements.empty())
    {
        static GameController& gameControllerInstance = GameController::getInstance();
        gameControllerInstance.addUpdateFunction(std::bind(&GameGrid::update, this, std::placeholders::_1));
    }
    m_movements.emplace_back(moveObject, std::move(path), speed, resetRotationOnEnd);
    return pathLength - 1;
}
void GameGrid::setSquares(std::set<Loc>&& locations)
{
    std::unordered_set<std::size_t> indices;
    std::transform(locations.cbegin(), locations.cend(), std::inserter(indices, indices.end()), [](Loc loc) -> std::size_t
    {
        return loc.first + loc.second * GRID_SIZE;
    });
    setSquares(std::move(indices));
}
void GameGrid::setSquares(std::unordered_set<std::size_t>&& indices)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares {};
    std::bitset<GRID_SIZE * GRID_SIZE / 2> setSquaresLarge {};
    std::unordered_set<GridObject*> usedObjects;

    for(auto index : indices)
    {
        GridObject* currentObj {};
        bool indexIsCombined {};
        for(auto& comb : m_combinedLocations)
        {
            if(std::find(comb.first.begin(), comb.first.end(), index) != comb.first.end())
            {
                currentObj = comb.second->get();
                indexIsCombined = true;
                index = *std::min_element(comb.first.begin(), comb.first.end());
                break;
            }
        }
        if(!indexIsCombined) currentObj = m_base[index].get();
        if(usedObjects.contains(currentObj)) continue;
        if(currentObj) usedObjects.insert(currentObj);
        if(indexIsCombined)
            setSquaresLarge.set(index / 2);
        else setSquares.set(index);
    }
    uiManagerInstance.setGameGridSquares(std::move(setSquares), std::move(setSquaresLarge));
}
void GameGrid::makeSquareNonInteractable(std::size_t index, glm::vec3 color)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    for(auto& comb : m_combinedLocations)
    {
        if(std::find(comb.first.begin(), comb.first.end(), index) != comb.first.end())
        {
            uiManagerInstance.makeLargeGridSquareNonInteractable(*comb.first.begin() / 2, color);
            return;
        }
    }
    uiManagerInstance.makeGridSquareNonInteractable(index, color);
}
std::pair<GridObject*, std::size_t> GameGrid::operator[](std::size_t index) const
{
    for(auto& comb : m_combinedLocations)
    {
        if(std::find(comb.first.begin(), comb.first.end(), index) != comb.first.end()) return std::make_pair(comb.second->get(), comb.first[0]);
    }
    return std::make_pair(m_base[index].get(), index);
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
    uiManagerInstance.setGameGridSquares({});
    std::unordered_set<std::size_t> indices;
    for(std::size_t i {}; i < m_grid.size(); ++i)
    {
        auto atIndex = m_grid[i].first;
        if(atIndex && atIndex->getTeam() == (m_playerOneToPlay ? GridObject::Team::playerOne : GridObject::Team::playerTwo))
            indices.insert(i);
    }
    m_grid.setSquares(std::move(indices));
}

Game::Game() : m_grid(this)
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    static UIManager& uiManagerInstance = UIManager::getInstance();
    static Random& randomInstance = Random::getInstance();
    uiManagerInstance.disableGameActionButtons(true);
    updateStatusTexts();
    
    auto basesRandomSeed = randomInstance.get<std::size_t>(2, (GRID_SIZE - 4) / 2) * 2;
    m_grid.initializeAt<Base>(0, basesRandomSeed, true);
    int otherBaseY = (GRID_SIZE - 2) - basesRandomSeed;
    m_grid.initializeAt<Base>(GRID_SIZE - 2, otherBaseY, false);
    m_grid.initializeAt<SubmarineUnit>(GRID_SIZE - 3, otherBaseY + 1, false);

    //generate islands
    std::vector<GameGrid::Loc> validIslandIndices;
    for(std::size_t x = 2; x < GRID_SIZE - 2; x += 2)//When there are no islands in the same column as the bases and not enough islands to fill a column, it is guaranteed that the path from base to another is not blocked.
        for(std::size_t y {}; y < GRID_SIZE; y += 2)
            {
                if(!m_grid.at(x, y) && !m_grid.at(x + 1, y) && !m_grid.at(x, y + 1) && !m_grid.at(x + 1, y + 1))
                    validIslandIndices.push_back(std::make_pair(x, y));
            }
    static constexpr std::size_t ISLAND_COUNT = 7;
    static_assert(GRID_SIZE / 2 > ISLAND_COUNT);
    assert(ISLAND_COUNT <= validIslandIndices.size());
    for(std::size_t initializedIslands {}; initializedIslands != ISLAND_COUNT; ++initializedIslands)
    {
        auto islandIndex = randomInstance.get<std::size_t>(0, validIslandIndices.size() - 1);
        m_grid.initializeAt<IslandObject>(validIslandIndices[islandIndex]);
        validIslandIndices.erase(validIslandIndices.begin() + islandIndex);
    }

    activatePlayerSquares();
    uiManagerInstance.moveSelection();
}
int Game::getMoney() const
{
    return m_playerOneToPlay ? m_playerData.first.money : m_playerData.second.money;
}
void Game::addMoney(int money)
{
    if(m_playerOneToPlay) m_playerData.first.money += money;
    else m_playerData.second.money += money;
    updateStatusTexts();
}
void Game::setTurnData(int maxMoves, int money)
{
    if(m_playerOneToPlay)
    {
        m_playerData.first.moves.second = maxMoves;
        m_playerData.first.turnMoney = money;
    }
    else
    {
        m_playerData.second.moves.second = maxMoves;
        m_playerData.second.turnMoney = money;
    }
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
void Game::endGame(bool playerOneWins)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.endGame(playerOneWins);
    m_gameOver = true;
}
void Game::updateStatusTexts()
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    auto& playerData = m_playerOneToPlay ? m_playerData.first : m_playerData.second;
    UnitObject* selectedUnit = m_selectedUnitIndices.has_value() ? static_cast<UnitObject*>(m_grid.at(m_selectedUnitIndices.value())) : nullptr;
    uiManagerInstance.updateGameStatusTexts(
        std::format("TURN: {1}{0}MONEY: {2}{3}{0}MOVES: {4}/{5}{0}MONEY PER TURN: {6}\n\n{7}",
        "        ",
        m_playerOneToPlay ? "PLAYER ONE" : "PLAYER TWO", 
        playerData.money, CURRENCY_SYMBOL,
        playerData.moves.first, playerData.moves.second,
        playerData.turnMoney,
        selectedUnit ? std::format("SELECTED UNIT HEALTH: {}/{}", selectedUnit->getHealth().first, selectedUnit->getHealth().second) : ""));
}
void Game::endTurn()
{
    ++m_turnNumber;  
    if(m_playerOneToPlay) 
    {
        m_playerData.first.moves.first = m_playerData.first.moves.second;
        m_playerData.first.money += m_playerData.first.turnMoney;
    }
    else
    {
        m_playerData.second.moves.first = m_playerData.second.moves.second;
        m_playerData.second.money += m_playerData.second.turnMoney;
    }
    m_playerOneToPlay = !m_playerOneToPlay;
    
    activatePlayerSquares();
    updateStatusTexts();
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.moveSelection();  
}
void Game::receiveGameInput(std::size_t index, ButtonTypes buttonType)
{
    if(m_gameOver || m_cooldown) return;
    static UIManager& uiManagerInstance = UIManager::getInstance();
    static SelectSquareCallbackManager& actionCallbackManagerInstance = SelectSquareCallbackManager::getInstance();
    static GameController& gameControllerInstance = GameController::getInstance();
    auto stopUnitSelection = [&]()
    {
        m_selectedUnitIndices.reset();
        uiManagerInstance.disableGameActionButtons(true);
    };
    auto returnToPlayerUnitSelection = [&]()
    {
        stopUnitSelection();
        activatePlayerSquares();
        uiManagerInstance.setEndTurnButton(true);
        uiManagerInstance.retrieveSavedSelection();
    };
    switch(buttonType)
    {
        using enum ButtonTypes;
    case actionButton:
        assert(m_selectedUnitIndices);
        if(index == 0)
        {
            if(m_selectedActionIndex)
            {
                UnitObject* selectedUnit = static_cast<UnitObject*>(m_grid.at(m_selectedUnitIndices.value()));
                uiManagerInstance.enableGameActionButtons(selectedUnit->getActionData());
                gameControllerInstance.getCamera()->zoom(selectedUnit->getPosition(), .3f, .5f, 1.f);
                m_grid.setSquares({m_selectedUnitIndices.value()});
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
            UnitObject* selectedUnit = static_cast<UnitObject*>(m_grid.at(m_selectedUnitIndices.value()));
            m_selectedActionIndex = index - 1;
            auto actionType = selectedUnit->useAction(m_selectedActionIndex.value());
            switch(actionType)
            {
                using enum ActionTypes;
            case selectSquare:
                uiManagerInstance.saveCurrentSelection();
                uiManagerInstance.disableGameActionButtons(false);
                gameControllerInstance.getCamera()->stopZoom();
                break;
            case immediate:
                m_selectedActionIndex.reset();
                gameControllerInstance.getCamera()->stopZoom();
                returnToPlayerUnitSelection();
                break;
            case nothing:
                m_selectedActionIndex.reset();
                break;
            }
        }
        break;
    case endTurnButton:
        endTurn();
        break;
    case gridSquare:
        if(m_selectedActionIndex)
        {
            //selected a a square for an action
            m_selectedActionIndex.reset();
            float cooldown {};
            auto selectedActionSquare = GameGrid::convertIndexToLocation(index);
            actionCallbackManagerInstance.invoke(this, selectedActionSquare.first, selectedActionSquare.second, cooldown);
            uiManagerInstance.removeSavedSelection();
            uiManagerInstance.setGameGridSquares({});
            stopUnitSelection();

            //cooldown
            m_cooldown = std::make_pair(cooldown, [&]()
            {
                if(this->m_gameOver) return;
                activatePlayerSquares();
                uiManagerInstance.setEndTurnButton(true);
                uiManagerInstance.retrieveSavedSelection();
            });

            gameControllerInstance.addUpdateFunction([&](float deltaTime) -> bool
            {
                auto& cooldownVal = m_cooldown.value();
                cooldownVal.first -= deltaTime;
                if(cooldownVal.first <= 0.f)
                {
                    cooldownVal.second();
                    m_cooldown.reset();
                    return true;
                }
                return false;
            });
        }
        else
        {
            //selected an unit
            auto atSelectedIndex = m_grid[index];
            UnitObject* selectedUnit = static_cast<UnitObject*>(atSelectedIndex.first);
            gameControllerInstance.getCamera()->zoom(selectedUnit->getPosition(), .3f, .5f, 1.f);
            uiManagerInstance.saveCurrentSelection();
            m_selectedUnitIndices = GameGrid::convertIndexToLocation(atSelectedIndex.second);
            uiManagerInstance.enableGameActionButtons(selectedUnit->getActionData());
            m_grid.makeSquareNonInteractable(index, SELECTED_GRID_SQUARE_COLOR);
            m_grid.setSquares({index});
            uiManagerInstance.setEndTurnButton(false);
        }
        break;
    }
    updateStatusTexts();
}