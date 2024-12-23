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
        using BlockMask = std::pair<std::bitset<Radius * 2 - 2>, std::bitset<Radius * 2 - 2>>;
        std::optional<BlockMask> blockMask;
        if(SelectType == SelectOnGridTypes::area) blockMask = BlockMask();
        auto addDirection = [&](bool vertical)
        {
            std::set<GameGrid::Loc> newIndices;
            std::size_t processedIndex = vertical ? indices.first : indices.second;
            
            std::size_t i = static_cast<std::size_t>(std::max(static_cast<int>(processedIndex) - Radius, 0));
            std::size_t startIndexOffset;
            if(i == 0) startIndexOffset = std::abs(static_cast<int>(processedIndex) - Radius);
            else startIndexOffset = 0;
            std::size_t maxIndex = processedIndex + Radius + 1;
            auto targetIndex = std::min(static_cast<std::size_t>(GRID_SIZE), maxIndex);
            for(; i < targetIndex; ++i)
            {
                if(i == processedIndex) continue;
                std::pair<std::size_t, std::size_t> currentPos(vertical ? i : indices.first, vertical ? indices.second : i);
                if(gameGrid.at(currentPos))
                {
                    if constexpr(!Blockable) continue;
                    assert(blockMask.has_value());
                    if(i < processedIndex) 
                    {
                        if(vertical) blockMask.value().first |= (1ULL << newIndices.size() + startIndexOffset) - 1;
                        else blockMask.value().second |= (1ULL << newIndices.size() + startIndexOffset) - 1;
                        newIndices.clear();
                        continue;
                    }
                    int squaresLeft = maxIndex - i - 1;
                    if(vertical) blockMask.value().first |= ((1ULL << squaresLeft) - 1) << (blockMask.value().first.size() - squaresLeft);
                    else blockMask.value().second |= ((1ULL << squaresLeft) - 1) << (blockMask.value().second.size() - squaresLeft);
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
                        if(gameGrid.at(x, y))
                        {
                            assert(areaBlockMask.has_value());
                            areaBlockMask.value().first.set(blockMaskX);
                            areaBlockMask.value().second.set(blockMaskY);
                            return;
                        }
                        if(!areaBlockMask.value().first.test(blockMaskX)
                            || !areaBlockMask.value().second.test(blockMaskY))
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
        bool isPlayerOne = gameGrid.at(indices)->isTeamOne();
        auto tryAddPosition = [&](std::size_t x, std::size_t y)
        {
            auto currentPosObject = gameGrid.at(x, y);
            GameGrid::Loc loc = std::make_pair(x, y);
            squaresToEnable.insert(loc);
            if(!currentPosObject || currentPosObject->isTeamOne() == isPlayerOne)
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
template<int Price>
bool BuyActionInterface<Price>::usable(Game* gameInstance) const
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
static constexpr glm::vec3 ACTION_UNUSABLE_COLOR {.4f, .1f, .3f};
template<int Price>
glm::vec3 BuyActionInterface<Price>::getColor(Game* gameInstance) const
{
    if(usable(gameInstance)) return Action::getColor(gameInstance);
    return ACTION_UNUSABLE_COLOR;
}
template<int Price>
ActionTypes BuyAction<Price>::use(Game* gameInstance)
{
    if(!this->usable(gameInstance)) return ActionTypes::nothing;
    this->takeMoney(gameInstance);
    buy(gameInstance);

    return ActionTypes::immediate;
}
template<int Price, typename UgradeClass>
void UpgradeActionInterface<Price, UgradeClass>::buy(Game* gameInstance)
{
    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto indices = gameInstance->getSelectedUnitIndices().value();
    gameGrid.initializeAt<UgradeClass>(indices, gameGrid.at(indices)->isTeamOne());
    upgrade(gameInstance);
}
template<int Radius>
bool MoveAction<Radius>::isUsable(Game* gameInstance) const
{
    return gameInstance->canMove();
}
static constexpr float PATH_MOVE_SPEED = .4f;
template<int Radius>
float MoveAction<Radius>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    gameInstance->takeMove();

    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto selectedLocation = gameInstance->getSelectedUnitIndices().value();
    GameGrid::Path moveAlongPath = gameGrid.findPath(selectedLocation, std::make_pair(x, y));
    int steps = gameGrid.moveAlongPath(std::move(moveAlongPath), PATH_MOVE_SPEED) + 1;//+1 for smooth delay
    return steps * PATH_MOVE_SPEED;
}
template<int Radius>
glm::vec3 MoveAction<Radius>::getColor(Game* gameInstance) const
{
    if(gameInstance->canMove()) return Action::getColor(gameInstance);
    return ACTION_UNUSABLE_COLOR;
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
    return this->usable(gameInstance);
}
template<int Price, int Radius, int Damage>
float AttackAction<Price, Radius, Damage>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    static GameController& gameControllerInstance = GameController::getInstance();

    static GameGrid& gameGrid = gameInstance->getGameGrid();
    auto selectedLocation = gameInstance->getSelectedUnitIndices().value();

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
        missileObject = std::make_unique<GameObject>(constructObject<LitObject>(assets::MODELS_MISSILE_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, MISSILE_MAT),
            constructObject<LitObject>(assets::MODELS_MISSILE_STRIPES_OBJ, assets::SHADERS_VBASIC_GLSL, assets::SHADERS_FBASIC_GLSL, MISSILE_STRIPES_MAT));
        missileObject->setScale(glm::vec3(1.f / GRID_SIZE));
    }
    else missileObject->addToRenderEngine();
    missileObject->setRotation(glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, .0f, 1.f)));

    GameGrid::Path moveAlongPath = gameGrid.findPath(selectedLocation, std::make_pair(x, y));
    static constexpr float UPWARDS_MOVEMENT_DURATION = .5f;
    static constexpr float MISSILE_FOLLOW_PATH_SPEED = .3f;
    float cooldown = UPWARDS_MOVEMENT_DURATION * 2 + moveAlongPath.size() * MISSILE_FOLLOW_PATH_SPEED;

    auto moveMissile = [missile = missileObject.get(), path = std::move(moveAlongPath), currentTime = 0.f, movePathWaitTime = 0.f, goingUp = true, goingDown = false, x, y, gameInstance, selectedLocation, upStartPos, upTargetPos, downTargetPos, downStartPos](float deltaTime) mutable -> bool
    {
        static GameGrid& gameGrid = gameInstance->getGameGrid();
    
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
                    UnitObject* hitObject = gameGrid.at(x, y);
                    assert(hitObject);
                    hitObject->addHealth(-Damage);
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
bool BuyUnitAction<Price, Radius, BuyUnit>::isUsable(Game* gameInstance) const
{
    return this->usable(gameInstance);
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
        gameGrid.at(startPos)->isTeamOne());
    int steps = gameGrid.moveAlongPath(gameGrid.findPath(startPos, 
        std::make_pair(x, y)), PATH_MOVE_SPEED, obj);

    return PATH_MOVE_SPEED * steps;
}
template<int Price, typename UpgradeClass, int NewMaxMoves, int NewTurnMoney>
void BaseUpgradeAction<Price, UpgradeClass, NewMaxMoves, NewTurnMoney>::upgrade(Game* gameInstance)
{
    gameInstance->setTurnData(NewMaxMoves, NewTurnMoney);
}

#include <game/unitObject.hpp>
// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class BuyUnitAction<400,5,AircraftCarrierUnit>;
template class SellAction<100>;
template class BaseUpgradeAction<900,BaseUpgrade1,3,300>;
template class AttackAction<75,6,150>;
template class AttackAction<75,7,250>;
template class SellAction<75>;
template class MoveAction<6>;
template class UpgradeAction<600,AircraftCarrierUpgrade1>;
template class MoveAction<2+1>;
template class SellAction<50>;
template class BuyUnitAction<250,5,SubmarineUnit>;
template class MoveAction<2>;
template class AttackAction<40,3,75>;
template class BaseUpgradeAction<1800,BaseUpgrade2,4,500>;
