#include <array>
#include <optional>
#include <cassert>

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

template<int Radius, bool Blockable, SelectOnGridTypes SelectType>
void SelectOnGridAction<Radius, Blockable, SelectType>::setGameGridSquares(IndicesList&& activeSquares)
{
    static UIManager& uiManagerInstance {UIManager::getInstance()};
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares;

    for(auto pair : activeSquares)
        setSquares.set(pair.first + pair.second * GRID_SIZE);
    uiManagerInstance.setGameGridSquares(std::move(setSquares));
}

template<int Radius, bool Blockable, SelectOnGridTypes SelectType>
void SelectOnGridAction<Radius, Blockable, SelectType>::use(Game* gameInstance)
{
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
        
        auto maxIndex = std::min(static_cast<std::size_t>(GRID_SIZE), processedIndex + Radius + 1);
        for(std::size_t i {static_cast<std::size_t>(std::max(static_cast<int>(processedIndex) - Radius, 0))}; 
            i < maxIndex; ++i)
        {
            if(i == processedIndex) continue;
            std::pair<std::size_t, std::size_t> currentPos(vertical ? i : indices.first, vertical ? indices.second : i);
            if(gameGrid.at(currentPos))
            {
                if(!Blockable) continue;
                assert(blockMask.has_value());
                if(i < processedIndex) 
                {
                    if(vertical) blockMask.value().first |= (1ULL << (newIndices.size() - 2));
                    else blockMask.value().second |= (1ULL << (newIndices.size() - 2));
                    newIndices.clear();
                    continue;
                }
                int squaresLeft = maxIndex - 1 - i;
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
        auto validPosIndex = [](int x)  -> bool
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
            for(int length {Radius - 1}, offsetX {dir.first}, offsetY {dir.second};
                length >= 1; length -= 2, offsetX += dir.first, offsetY += dir.second)
            {
                int x = indices.first + offsetX;
                int y = indices.second + offsetY;
                if(!validPosIndex(x) || !validPosIndex(y)) break;
                std::size_t blockMaskX = Radius + offsetX + (offsetX < 0 ? -1: -2);
                std::size_t blockMaskY = Radius + offsetY + (offsetY < 0 ? -1: -2);
                if(!gameGrid.at(x, y))
                {
                    if(!Blockable
                        || !blockMask.value().first.test(blockMaskX)
                        || !blockMask.value().second.test(blockMaskY))
                        squaresToActivate.push_back(std::make_pair(x, y));
                }
                else if(Blockable)
                {
                    blockMask.value().first.set(blockMaskX);
                    blockMask.value().second.set(blockMaskY);
                }
                for(int i {1}; i < length; ++i)
                {
                    std::size_t newBlockMaskX = blockMaskX + (dir.first == 1 ? i : -i);
                    std::size_t newBlockMaskY = blockMaskY + (dir.second == 1 ? i : -i);
          
                    int newX = x + (dir.first == 1 ? i : -i);
                    int newY = y + (dir.second == 1 ? i : -i);
                    bool validX = validPosIndex(newX), validY = validPosIndex(newY);
                    if(!validX && !validY) break;
                    if(validX)
                    {
                        if(!gameGrid.at(newX, y))
                        {
                            if(!Blockable 
                                || !blockMask.value().first.test(newBlockMaskX)
                                || !blockMask.value().second.test(blockMaskY))
                                squaresToActivate.push_back(std::make_pair(newX, y));
                        }
                        else if(Blockable)
                        {
                            blockMask.value().first.set(newBlockMaskX);
                            blockMask.value().second.set(blockMaskY);
                        }
                    }
                    if(validY)
                    {
                        if(!gameGrid.at(x, newY))
                        {
                            if(!Blockable 
                                || !blockMask.value().first.test(blockMaskX)
                                || !blockMask.value().second.test(newBlockMaskY))
                                squaresToActivate.push_back(std::make_pair(x, newY));
                        }
                        else if(Blockable)
                        {
                            blockMask.value().first.set(blockMaskX);
                            blockMask.value().second.set(newBlockMaskY);
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
}
template<int Radius>
void MoveAction<Radius>::callback(Game* gameInstance, std::size_t x, std::size_t y) const
{
    GameGrid& gameGrid = this->getGameGrid(gameInstance);
    auto selectedIndices = this->getSelectedUnitIndices(gameInstance);
    gameGrid.moveAt(selectedIndices.first, selectedIndices.second, x, y);
}

// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class MoveAction<2>;
template class MoveAction<6>;
