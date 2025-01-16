#include <array>
#include <optional>
#include <bitset>
#include <unordered_set>
#include <memory>

#include <game/action.hpp>
#include <game/game.hpp>
#include <game/gameObject.hpp>
#include <game/uiManager.hpp>
#include <assets.hpp>
#include <game/gameController.hpp>
#include <game/random.hpp>

static constexpr glm::vec3 ACTION_DEFAULT_COLOR {.2f, .8f, .6f};
glm::vec3 Action::getColor(Game*) const
{
    return ACTION_DEFAULT_COLOR;
}
template<int Radius, SelectOnGridTypes SelectType, bool Blockable>
ActionTypes SelectOnGridAction<Radius, SelectType, Blockable>::use(Game* gameInstance)
{
    if(!isUsable(gameInstance)) return ActionTypes::nothing;
    SelectSquareCallback callback = [&](Game* gameInstance, std::size_t x, std::size_t y)
    {
        return this->callback(gameInstance, x, y);
    }; 
    SelectSquareCallbackManager::getInstance().bindCallback(std::move(callback));

    GameGrid& gameGrid = gameInstance->getGameGrid();
    GameGrid::Loc indices = gameInstance->getSelectedUnitIndices().value();
    std::set<GameGrid::Loc> squaresToEnable {indices};

    static constexpr auto validLocIndex = [](int a) -> bool
    {
        return a < GRID_SIZE && a >= 0;
    };
    static constexpr std::array<std::pair<int, int>, 4> directions =
    {
        std::make_pair(1, -1),
        std::make_pair(1, 1),
        std::make_pair(-1, 1), 
        std::make_pair(-1, -1)
    };

    if constexpr(SelectType != SelectOnGridTypes::selectEnemyUnit)
    {
        constexpr std::size_t blockMaskN = Radius * 2 - 2;
        using BlockMask = std::pair<std::bitset<blockMaskN>, std::bitset<blockMaskN>>;
        std::optional<BlockMask> blockMask;
        constexpr bool hasBlockMask = Blockable ? SelectType == SelectOnGridTypes::area : false;//blockMask.has_value() but constexpr
        GridObject* self {};
        if constexpr(Blockable)
        {
            self = gameGrid.at(indices);
            if constexpr(hasBlockMask)
                blockMask = BlockMask();
        }
        auto addDirection = [&](bool vertical)
        {
            std::set<GameGrid::Loc> newIndices;
            std::size_t processedIndex = vertical ? indices.first : indices.second;
            
            std::size_t startIndex = static_cast<std::size_t>(std::max(static_cast<int>(processedIndex) - Radius, 0)); 
            std::size_t startIndexOffset {};
            if(startIndex == 0) startIndexOffset = std::abs(static_cast<int>(processedIndex) - Radius);
            std::size_t maxIndex = processedIndex + Radius + 1;
            auto targetIndex = std::min(static_cast<std::size_t>(GRID_SIZE), maxIndex);
            for(std::size_t i = startIndex; i < targetIndex; ++i)
            {
                if(i == processedIndex) continue;
                std::pair<std::size_t, std::size_t> currentPos(vertical ? i : indices.first, vertical ? indices.second : i);
                GridObject* currentLocObj = gameGrid.at(currentPos);
                if(currentLocObj)
                {
                    if constexpr(!Blockable) continue;
                    if(currentLocObj == self) continue;
                    if(i < processedIndex)
                    {
                        if(hasBlockMask)
                        {
                            if(vertical) blockMask->first |= (1ull << startIndexOffset + i - startIndex) - 1;
                            else blockMask->second |= (1ull << startIndexOffset + i - startIndex) - 1;
                        }
                        newIndices.clear();
                        continue;
                    }
                    if(hasBlockMask)
                    {
                        if(vertical) blockMask->first |= ((1ull << (maxIndex - 1 - i)) - 1) << blockMaskN / 2 + i - processedIndex - 1;
                        else blockMask->second |= ((1ull << (maxIndex - 1 - i)) - 1) << blockMaskN / 2 + i - processedIndex - 1;
                    }
                    break;
                }
                newIndices.insert(currentPos);
            }
            squaresToEnable.insert(std::make_move_iterator(newIndices.begin()), std::make_move_iterator(newIndices.end()));
        };
        auto addArea = [&]()
        {
            for(auto dir : directions)
            {
                //this lambda should not affect the blocking elsewhere hence the copied blockmask
                std::optional<BlockMask> areaBlockMask = blockMask;
                auto tryAddPosition = [&](std::size_t x, std::size_t y, std::size_t blockMaskX, std::size_t blockMaskY)
                {
                    if constexpr(Blockable)
                    {
                        auto& block = areaBlockMask.value();
                        GridObject* currentLocObject = gameGrid.at(x, y);
                        if(currentLocObject)
                        {
                            if(currentLocObject == self) return;
                            block.first.set(blockMaskX);
                            block.second.set(blockMaskY);
                            return;
                        }
                        if(!block.first.test(blockMaskX)
                            || !block.second.test(blockMaskY))
                            squaresToEnable.insert(std::make_pair(x, y));
                    }
                    else
                    {
                        if(!gameGrid.at(x, y))
                            squaresToEnable.insert(std::make_pair(x, y));                    
                    }
                };
                for(int length {Radius - 1}, offsetX {dir.first}, offsetY {dir.second};
                    length >= 1; length -= 2, offsetX += dir.first, offsetY += dir.second)
                {
                    int x = indices.first + offsetX;
                    int y = indices.second + offsetY;
                    if(!validLocIndex(x) || !validLocIndex(y)) break;
                    std::size_t blockMaskX = Radius + offsetX + (offsetX < 0 ? -1 : -2);
                    std::size_t blockMaskY = Radius + offsetY + (offsetY < 0 ? -1 : -2);
                    tryAddPosition(x, y, blockMaskX, blockMaskY);
                    for(int i {1}; i < length; ++i)
                    {
                        int newX = x + (dir.first == 1 ? i : -i);
                        int newY = y + (dir.second == 1 ? i : -i);
                        bool validX = validLocIndex(newX), validY = validLocIndex(newY);

                        std::size_t newBlockMaskX = blockMaskX + (dir.first == 1 ? i : -i);
                        std::size_t newBlockMaskY = blockMaskY + (dir.second == 1 ? i : -i);

                        if(!validX && !validY) break;
                        if(validX)
                            tryAddPosition(newX, y, newBlockMaskX, blockMaskY);
                        if(validY)
                            tryAddPosition(x, newY, blockMaskX, newBlockMaskY);
                    }
                }
            }
        };
        addDirection(true);
        addDirection(false);
        if constexpr(SelectType == SelectOnGridTypes::area) addArea();

        gameGrid.setSquares(std::move(squaresToEnable));
    }
    else //select enemy unit logic
    {
        std::unordered_set<std::size_t> squaresToDisplay;
        auto targetTeam = gameGrid.at(indices)->getTeam() == GridObject::Team::playerOne ? GridObject::Team::playerTwo : GridObject::Team::playerOne;
        auto tryAddPosition = [&](std::size_t x, std::size_t y)
        {
            auto currentPosObject = gameGrid.at(x, y);
            GameGrid::Loc loc = std::make_pair(x, y);
            squaresToEnable.insert(loc);
            if(!currentPosObject || currentPosObject->getTeam() != targetTeam)
                squaresToDisplay.insert(GameGrid::convertLocationToIndex(loc));
        };
        auto addDirection = [&](bool vertical)
        {
            std::size_t processedIndex = vertical ? indices.first : indices.second;
            
            std::size_t i = static_cast<std::size_t>(std::max(static_cast<int>(processedIndex) - Radius, 0));
            auto targetIndex = std::min(static_cast<std::size_t>(GRID_SIZE), processedIndex + Radius + 1);
            for(; i < targetIndex; ++i)
            {
                if(i != processedIndex) 
                    tryAddPosition(vertical ? i : indices.first, vertical ? indices.second : i);
            }
        };
        auto addArea = [&]()
        {
            for(auto dir : directions)
            {
                for(int length {Radius - 1}, offsetX {dir.first}, offsetY {dir.second};
                    length >= 1; length -= 2, offsetX += dir.first, offsetY += dir.second)
                {
                    int x = indices.first + offsetX;
                    int y = indices.second + offsetY;
                    if(!validLocIndex(x) || !validLocIndex(y)) break;

                    tryAddPosition(x, y);
                    for(int i {1}; i < length; ++i)
                    {
                        int newX = x + (dir.first == 1 ? i : -i);
                        int newY = y + (dir.second == 1 ? i : -i);
                        bool validX = validLocIndex(newX), validY = validLocIndex(newY);
                        if(!validX && !validY) break;
                        if(validX)
                            tryAddPosition(newX, y);
                        if(validY)
                            tryAddPosition(x, newY);
                    }
                }
            }
        };
        addDirection(true);
        addDirection(false);
        addArea();

        gameGrid.setSquares(std::move(squaresToEnable));
        for(auto index : squaresToDisplay) 
            gameGrid.makeSquareNonInteractable(index, SELECTED_GRID_NONINTERACTABLE_COLOR);
    }
    return ActionTypes::selectSquare;
}
static constexpr glm::vec3 ACTION_UNUSABLE_COLOR {.4f, .1f, .3f};
glm::vec3 ActionColorInterface::getColor(Game* gameInstance) const
{
    if(isUsable(gameInstance)) return Action::getColor(gameInstance);
    return ACTION_UNUSABLE_COLOR;
}
template<int Price>
bool BuyActionInterface<Price>::isUsable(Game* gameInstance) const
{
    return gameInstance->getMoney() >= Price;
}
template<int Price>
std::string_view BuyActionInterface<Price>::getName() const
{
    if(m_buyActionName.empty()) 
        m_buyActionName = std::format("{}\n-{}{}", getBuyActionName(), Price, CURRENCY_SYMBOL);
    return m_buyActionName;
}
template<int Price>
void BuyActionInterface<Price>::takeMoney(Game* gameInstance)
{
    gameInstance->addMoney(-Price);
    assert(gameInstance->getMoney() >= 0);
}
template<int Price>
ActionTypes BuyAction<Price>::use(Game* gameInstance)
{
    if(!this->isUsable(gameInstance)) return ActionTypes::nothing;
    this->takeMoney(gameInstance);
    buy(gameInstance);

    return ActionTypes::immediate;
}
template<int Price, typename UgradeClass>
void UpgradeActionInterface<Price, UgradeClass>::buy(Game* gameInstance)
{
    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto indices = gameInstance->getSelectedUnitIndices().value();
    gameGrid.initializeAt<UgradeClass>(indices, gameGrid.at(indices)->getTeam() == GridObject::Team::playerOne);
    upgrade(gameInstance);
}
template<int Radius, SelectOnGridTypes MoveType>
bool MoveAction<Radius, MoveType>::isUsable(Game* gameInstance) const
{
    return gameInstance->canMove();
}
static constexpr float PATH_MOVE_SPEED = .4f;
template<int Radius, SelectOnGridTypes MoveType>
float MoveAction<Radius, MoveType>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    gameInstance->takeMove();

    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto selectedLocation = gameInstance->getSelectedUnitIndices().value();
    GameGrid::Path moveAlongPath = gameGrid.findPath(selectedLocation, std::make_pair(x, y));
    int steps = gameGrid.moveAlongPath(std::move(moveAlongPath), PATH_MOVE_SPEED) + 1;//+1 for smooth delay
    return steps * PATH_MOVE_SPEED;
}
template<int Worth>
ActionTypes SellAction<Worth>::use(Game* gameInstance)
{
    gameInstance->addMoney(Worth);
    gameInstance->getGameGrid().destroyAt(gameInstance->getSelectedUnitIndices().value());
    return ActionTypes::immediate;
}
template<int Price, int Radius, int Damage>
bool AttackAction<Price, Radius, Damage>::isUsable(Game* gameInstance) const
{
    if(m_usedTurn == gameInstance->getTurnNumber())
    {
        GameGrid& gameGrid = gameInstance->getGameGrid();
        if(m_usedUnits.contains(gameGrid.at(gameInstance->getSelectedUnitIndices().value())))
            return false;
    }
    else m_usedUnits.clear();
    return BuyActionInterface<Price>::isUsable(gameInstance);
}
template<int Price, int Radius, int Damage>
float AttackAction<Price, Radius, Damage>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto selectedLocation = gameInstance->getSelectedUnitIndices().value();
    m_usedTurn = gameInstance->getTurnNumber();
    m_usedUnits.insert(gameGrid.at(selectedLocation));

    this->takeMoney(gameInstance);
    static GameController& gameControllerInstance = GameController::getInstance();

    static constexpr float MISSILE_MAX_HEIGHT = .4f;
    glm::vec3 upStartPos = GameGrid::gridLocationToPosition(selectedLocation);
    glm::vec3 upTargetPos = upStartPos + glm::vec3(0.f, MISSILE_MAX_HEIGHT, 0.f);
    glm::vec3 downTargetPos = GameGrid::gridLocationToPosition(std::make_pair(x ,y));
    glm::vec3 downStartPos = downTargetPos + glm::vec3(0.f, MISSILE_MAX_HEIGHT, 0.f);    

    static constexpr Material MISSILE_MAT {glm::vec3(.5f, .7f, .3f), .3f, 180.f, .6f};
    static constexpr Material MISSILE_STRIPES_MAT {glm::vec3(.9f, .9f, .1f), .6f, 90.f, .2f};
    static std::unique_ptr<GameObject> missileObject;
    if(!missileObject)
    {
        missileObject = std::make_unique<GameObject>(constructObject<LitObject>(MODELS_MISSILE, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, MISSILE_MAT),
            constructObject<LitObject>(MODELS_MISSILE_STRIPES, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, MISSILE_STRIPES_MAT));
        missileObject->setScale(glm::vec3(1.f / GRID_SIZE));
    }
    else missileObject->addToRenderEngine();
    missileObject->setRotation(glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, .0f, 1.f)));

    GameGrid::Path moveAlongPath = gameGrid.findPath(selectedLocation, std::make_pair(x, y), false);
    static constexpr float UPWARDS_MOVEMENT_DURATION = .5f;
    static constexpr float MISSILE_FOLLOW_PATH_SPEED = .3f;
    float cooldown = UPWARDS_MOVEMENT_DURATION * 2 + moveAlongPath.size() * MISSILE_FOLLOW_PATH_SPEED;

    auto moveMissile = [missile = missileObject.get(), path = std::move(moveAlongPath), currentTime = 0.f, movePathWaitTime = 0.f, goingUp = true, goingDown = false, x, y, gameInstance, selectedLocation, upStartPos, upTargetPos, downTargetPos, downStartPos](float deltaTime) mutable -> bool
    {
        GameGrid& gameGrid = gameInstance->getGameGrid();
    
        if(goingUp || goingDown)
        {
            currentTime += deltaTime;
            if(currentTime >= UPWARDS_MOVEMENT_DURATION)
            {
                currentTime = 0.f;
                if(goingUp)
                {
                    missile->setPosition(upTargetPos);
                    goingUp = false;
                    movePathWaitTime = gameGrid.moveAlongPath(std::move(path), MISSILE_FOLLOW_PATH_SPEED, missile, false) * MISSILE_FOLLOW_PATH_SPEED;
                    return false;
                }
                else
                {
                    //target is hit
                    auto hitObject = static_cast<UnitObject*>(gameGrid.at(x, y));
                    assert(hitObject);
                    static_assert(Damage >= 25); 
                    hitObject->addHealth(Random::getInstance().get<int>(-Damage + 25, -Damage - 25));
                    missile->removeFromRenderEngine();
                    return true;
                }
            }
            float otherRatio = currentTime / UPWARDS_MOVEMENT_DURATION;
            float ratio = 1.f - otherRatio;
            auto currentPos = 
                goingUp ? upStartPos * ratio + upTargetPos * otherRatio : downStartPos * ratio + downTargetPos * otherRatio;
            missile->setPosition(currentPos);
        }
        else
        {
            movePathWaitTime -= deltaTime;
            if(movePathWaitTime <= 0.f)
            {
                goingDown = true;
                missileObject->setRotation(glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, .0f, 1.f)));
            }
        }
        return false;
    };
    gameControllerInstance.addUpdateFunction(std::move(moveMissile));
    return cooldown;
}
template<int Price, int Radius, typename BuyUnit>
std::string_view BuyUnitAction<Price, Radius, BuyUnit>::getInfoText() const
{
    static std::string infoText = std::format("BUY {}", unitToString<BuyUnit>());
    return infoText;
}
template<int Price, int Radius, typename BuyUnit>
float BuyUnitAction<Price, Radius, BuyUnit>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    this->takeMoney(gameInstance);

    auto& gameGrid = gameInstance->getGameGrid();
    auto startPos = gameInstance->getSelectedUnitIndices().value();
    auto obj = gameGrid.initializeAt<BuyUnit>(x, y, 
        gameGrid.at(startPos)->getTeam() == GridObject::Team::playerOne);
    int steps = gameGrid.moveAlongPath(gameGrid.findPath(startPos, 
        std::make_pair(x, y)), PATH_MOVE_SPEED, obj);

    return PATH_MOVE_SPEED * steps;
}
template<int Price, typename UpgradeClass, int NewMaxMoves, int NewTurnMoney>
void BaseUpgradeAction<Price, UpgradeClass, NewMaxMoves, NewTurnMoney>::upgrade(Game* gameInstance)
{
    gameInstance->setTurnData(NewMaxMoves, NewTurnMoney);
}


// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class AttackAction<20,3,100>;
template class MoveAction<5>;
template class BuyUnitAction<250,5,SubmarineUnit>;
template class BuyUnitAction<450,5,AircraftCarrierUnit>;
template class UpgradeAction<400,SubmarineUnitUpgrade1>;
template class SellAction<125>;
template class SellAction<100>;
template class AttackAction<30,4,100>;
template class AttackAction<40,7,200>;
template class MoveAction<3,SelectOnGridTypes::cross>;
template class AttackAction<20,4,150>;
template class BuyUnitAction<350,5,ShipUnit>;
template class SellAction<50>;
template class UpgradeAction<600,AircraftCarrierUpgrade1>;
template class AttackAction<40,6,150>;
template class MoveAction<3>;
template class BaseUpgradeAction<600,BaseUpgrade1,3,250>;
template class MoveAction<6>;
template class BaseUpgradeAction<900,BaseUpgrade2,4,300>;
template class SellAction<75>;
template class MoveAction<4,SelectOnGridTypes::cross>;
