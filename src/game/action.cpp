#include <array>

#include <game/action.hpp>
#include <game/game.hpp>

std::pair<std::size_t, std::size_t> Action::getSelectedUnitIndices(Game* gameInstance) const
{
    return gameInstance->m_selectedUnitIndices.value();
}
GameGrid& Action::getGameGrid(Game* gameInstance) const
{
    return gameInstance->m_grid;
}

template<int Radius, bool Blockable>
void SelectOnGridAction<Radius, Blockable>::use(Game* gameInstance)
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
                if(!Blockable) continue;
                if(i < processedIndex) 
                {
                    newIndices.clear();
                    continue;
                }
                break;
            }
            newIndices.push_back(std::move(currentPos));   
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

// Generated with 'tools/templates_instantiations.py'
// Do not add or modify anything after these comments
template class MoveAction<2>;
template class MoveAction<4>;
