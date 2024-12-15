#include <array>
#include <optional>

#include <game/action.hpp>
#include <game/game.hpp>
#include <game/uiManager.hpp>

std::pair<std::size_t, std::size_t> Action::getSelectedUnitIndices(Game* gameInstance) const
{
    return gameInstance->m_selectedUnitIndices.value();
}
GameGrid& Action::getGameGrid(Game* gameInstance) const
{
    return gameInstance->m_grid;
}

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
    if(!usable(gameInstance)) return ActionTypes::nothing;
    SelectSquareCallback callback = [&](Game* gameInstance, std::size_t x, std::size_t y)
    {
        return this->callback(gameInstance, x, y);
    }; 
    SelectSquareCallbackManager::getInstance().bindCallback(std::move(callback));

    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto indices = this->getSelectedUnitIndices(gameInstance);
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
ActionTypes BuyAction<Price>::use(Game* gameInstance)
{
    if(gameInstance->getMoney() < Price) return ActionTypes::nothing;
    gameInstance->addMoney(-Price);
    buy(gameInstance);

    return ActionTypes::immediate;
}
template<std::int32_t Price>
std::string_view BuyAction<Price>::getName() const
{
    if(m_buyActionName.empty()) 
        m_buyActionName = std::string(getBuyActionName()) + '\n' + std::to_string(Price) + CURRENCY_SYMBOL;
    return m_buyActionName;
}
static constexpr glm::vec3 ACTION_UNUSABLE_COLOR {.4f, .1f, .3f};
template<std::int32_t Price>
glm::vec3 BuyAction<Price>::getColor(Game* gameInstance) const
{
    if(gameInstance->getMoney() < Price) return ACTION_UNUSABLE_COLOR;
    return Action::getColor(gameInstance);

}
template<int Radius>
bool MoveAction<Radius>::usable(Game* gameInstance) const
{
    return gameInstance->canMove();
}
template<int Radius>
float MoveAction<Radius>::callback(Game* gameInstance, std::size_t x, std::size_t y) const
{
    gameInstance->takeMove();
    static constexpr float PATH_MOVE_SPEED = .4f;

    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto selectedIndices = this->getSelectedUnitIndices(gameInstance);

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
template<std::int32_t Price, typename UgradeClass>
void UpgradeAction<Price, UgradeClass>::buy(Game* gameInstance)
{
    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto indices = this->getSelectedUnitIndices(gameInstance);
    gameGrid.initializeAt<UgradeClass>(indices, gameGrid.at(indices)->isTeamOne());
}


#include <game/unitObject.hpp>
// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class MoveAction<2>;
template class UpgradeAction<200,BaseUpgrade2>;
template class MoveAction<2+1>;
template class UpgradeAction<100,BaseUpgrade1>;
template class MoveAction<6>;
template class UpgradeAction<900,AircraftCarrierUpgrade1>;
