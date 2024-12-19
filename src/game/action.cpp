#include <array>
#include <optional>

#include <game/action.hpp>
#include <game/game.hpp>
#include <game/uiManager.hpp>

static constexpr glm::vec3 ACTION_DEFAULT_COLOR {.2f, .8f, .6f};
glm::vec3 Action::getColor(Game*) const
{
    return ACTION_DEFAULT_COLOR;
}

template<int Radius, bool Blockable, SelectOnGridTypes SelectType>
void SelectOnGridAction<Radius, Blockable, SelectType>::setGameGridSquares(IndicesList&& activeSquares)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares;

    for(auto pair : activeSquares)
        setSquares.set(pair.first + pair.second * GRID_SIZE);
    uiManagerInstance.setGameGridSquares(std::move(setSquares));
}
template<int Radius, bool Blockable, SelectOnGridTypes SelectType>
ActionTypes SelectOnGridAction<Radius, Blockable, SelectType>::use(Game* gameInstance)
{
    if(!isUsable(gameInstance)) return ActionTypes::nothing;
    SelectSquareCallback callback = [&](Game* gameInstance, std::size_t x, std::size_t y)
    {
        return this->callback(gameInstance, x, y);
    }; 
    SelectSquareCallbackManager::getInstance().bindCallback(std::move(callback));

    GameGrid& gameGrid = gameInstance->getGameGrid();
    auto indices = gameInstance->getSelectedUnitIndices().value();
    IndicesList squaresToActivate;
    using BlockMask = std::pair<std::bitset<Radius * 2 - 2>, std::bitset<Radius * 2 - 2>>;
    std::optional<BlockMask> blockMask;
    if(SelectType == SelectOnGridTypes::area) blockMask = BlockMask();

    auto addDirection = [&squaresToActivate, indices, &blockMask, &gameGrid](bool vertical)
    {
        IndicesList newIndices;
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
                if(!Blockable) continue;
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
            newIndices.push_back(std::move(currentPos));   
        }
        squaresToActivate.insert(squaresToActivate.end(), 
            std::make_move_iterator(newIndices.begin()), std::make_move_iterator(newIndices.end()));
    };
    auto addArea = [&squaresToActivate, indices, &blockMask, &gameGrid]()
    {
        auto validLocIndex = [](int x) -> bool
        {
            return x < GRID_SIZE && x >= 0;
        };
        static constexpr std::array<std::pair<int, int>, 4> directions =
        {
            std::make_pair(1, -1),
            std::make_pair(1, 1),
            std::make_pair(-1, 1), 
            std::make_pair(-1, -1)
        };
        for(auto dir : directions)
        {
            //this lambda should not affect the blocking elsewhere hence the copy
            std::optional<BlockMask> areaBlockMask = blockMask;
            for(int length {Radius - 1}, offsetX {dir.first}, offsetY {dir.second};
                length >= 1; length -= 2, offsetX += dir.first, offsetY += dir.second)
            {
                int x = indices.first + offsetX;
                int y = indices.second + offsetY;
                if(!validLocIndex(x) || !validLocIndex(y)) break;
                std::size_t blockMaskX = Radius + offsetX + (offsetX < 0 ? -1 : -2);
                std::size_t blockMaskY = Radius + offsetY + (offsetY < 0 ? -1 : -2);
                if(!gameGrid.at(x, y))
                {
                    if(!Blockable
                        || !areaBlockMask.value().first.test(blockMaskX)
                        || !areaBlockMask.value().second.test(blockMaskY))
                        squaresToActivate.push_back(std::make_pair(x, y));
                }
                else if(Blockable)
                {
                    assert(areaBlockMask.has_value());
                    areaBlockMask.value().first.set(blockMaskX);
                    areaBlockMask.value().second.set(blockMaskY);
                }
                for(int i {1}; i < length; ++i)
                {
                    std::size_t newBlockMaskX = blockMaskX + (dir.first == 1 ? i : -i);
                    std::size_t newBlockMaskY = blockMaskY + (dir.second == 1 ? i : -i);
          
                    int newX = x + (dir.first == 1 ? i : -i);
                    int newY = y + (dir.second == 1 ? i : -i);
                    bool validX = validLocIndex(newX), validY = validLocIndex(newY);
                    if(!validX && !validY) break;
                    if(validX)
                    {
                        if(!gameGrid.at(newX, y))
                        {
                            if(!Blockable 
                                || !areaBlockMask.value().first.test(newBlockMaskX)
                                || !areaBlockMask.value().second.test(blockMaskY))
                                squaresToActivate.push_back(std::make_pair(newX, y));
                        }
                        else if(Blockable)
                        {
                            areaBlockMask.value().first.set(newBlockMaskX);
                            areaBlockMask.value().second.set(blockMaskY);
                        }
                    }
                    if(validY)
                    {
                        if(!gameGrid.at(x, newY))
                        {
                            if(!Blockable 
                                || !areaBlockMask.value().first.test(blockMaskX)
                                || !areaBlockMask.value().second.test(newBlockMaskY))
                                squaresToActivate.push_back(std::make_pair(x, newY));
                        }
                        else if(Blockable)
                        {
                            areaBlockMask.value().first.set(blockMaskX);
                            areaBlockMask.value().second.set(newBlockMaskY);
                        }
                    }
                }
            }
        }
    };
    addDirection(true);
    addDirection(false);
    if(SelectType == SelectOnGridTypes::area) addArea();

    this->setGameGridSquares(std::move(squaresToActivate));
    return ActionTypes::selectSquare;
}
template<std::int32_t Price>
bool BuyActionInterface<Price>::usable(Game* gameInstance) const
{
    return gameInstance->getMoney() >= Price;
}
template<std::int32_t Price>
std::string_view BuyActionInterface<Price>::getName() const
{
    if(m_buyActionName.empty()) 
        m_buyActionName = std::format("{}\n{}{}", getBuyActionName(), Price, CURRENCY_SYMBOL);
    return m_buyActionName;
}
template<std::int32_t Price>
void BuyActionInterface<Price>::takeMoney(Game* gameInstance)
{
    gameInstance->addMoney(-Price);
    assert(gameInstance->getMoney() >= 0);
}
static constexpr glm::vec3 ACTION_UNUSABLE_COLOR {.4f, .1f, .3f};
template<std::int32_t Price>
glm::vec3 BuyActionInterface<Price>::getColor(Game* gameInstance) const
{
    if(usable(gameInstance)) return Action::getColor(gameInstance);
    return ACTION_UNUSABLE_COLOR;
}
template<std::int32_t Price>
ActionTypes BuyAction<Price>::use(Game* gameInstance)
{
    if(!this->usable(gameInstance)) return ActionTypes::nothing;
    this->takeMoney(gameInstance);
    buy(gameInstance);

    return ActionTypes::immediate;
}
template<std::int32_t Price, typename UgradeClass>
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
    auto selectedIndices = gameInstance->getSelectedUnitIndices().value();
    GameGrid::Path moveAlongPath = gameGrid.findPath(selectedIndices, std::make_pair(x, y));
    int steps = gameGrid.moveAlongPath(std::move(moveAlongPath), PATH_MOVE_SPEED);
    return steps * PATH_MOVE_SPEED;
}
template<int Radius>
glm::vec3 MoveAction<Radius>::getColor(Game* gameInstance) const
{
    if(gameInstance->canMove()) return Action::getColor(gameInstance);
    return ACTION_UNUSABLE_COLOR;
}
template<std::int32_t Worth>
ActionTypes SellAction<Worth>::use(Game* gameInstance)
{
    gameInstance->addMoney(Worth);
    gameInstance->getGameGrid().destroyAt(gameInstance->getSelectedUnitIndices().value());
    return ActionTypes::immediate;
}
template<int Radius, std::int32_t Price, typename BuyUnit>
bool BuyUnitAction<Radius, Price, BuyUnit>::isUsable(Game* gameInstance) const
{
    return this->usable(gameInstance);
}
template<int Radius, std::int32_t Price, typename BuyUnit>
std::string_view BuyUnitAction<Radius, Price, BuyUnit>::getInfoText() const
{
    static constexpr std::string_view infoText = unitToString<BuyUnit>();
    return infoText;
}
template<int Radius, std::int32_t Price, typename BuyUnit>
float BuyUnitAction<Radius, Price, BuyUnit>::callback(Game* gameInstance, std::size_t x, std::size_t y)
{
    this->takeMoney(gameInstance);

    auto& gameGrid = gameInstance->getGameGrid();
    auto startPos = gameInstance->getSelectedUnitIndices().value();
    auto obj = gameGrid.initializeAt<BuyUnit>(x, y, 
        gameGrid.at(startPos)->isTeamOne());
    int steps = gameGrid.moveAlongPath(gameGrid.findPath(startPos, 
        std::make_pair(x, y)), PATH_MOVE_SPEED, obj);

    return PATH_MOVE_SPEED * 1;
}
template<std::int32_t Price, typename UpgradeClass, int NewMaxMoves, std::int32_t NewTurnMoney>
void BaseUpgradeAction<Price, UpgradeClass, NewMaxMoves, NewTurnMoney>::upgrade(Game* gameInstance)
{
    gameInstance->setTurnData(NewMaxMoves, NewTurnMoney);
}

#include <game/unitObject.hpp>
// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class MoveAction<2+1>;
template class BuyUnitAction<5,400,AircraftCarrierUnit>;
template class UpgradeAction<600,AircraftCarrierUpgrade1>;
template class BaseUpgradeAction<900,BaseUpgrade1,3,300>;
template class SellAction<75>;
template class SellAction<100>;
template class SellAction<50>;
template class BuyUnitAction<5,250,SubmarineUnit>;
template class MoveAction<2>;
template class MoveAction<6>;
template class BaseUpgradeAction<1800,BaseUpgrade2,4,500>;
