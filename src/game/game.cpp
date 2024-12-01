#include <cassert>
#include <iterator>
#include <algorithm>

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
#include <assets.hpp>

GameGrid::~GameGrid() {} 

UnitObject* GameGrid::at(std::size_t x, std::size_t y) const
{
    return m_base[x + y * GRID_SIZE].get();
}
UnitObject* GameGrid::at(std::pair<std::size_t, std::size_t> indices) const
{
    return at(indices.first, indices.second);
}
void GameGrid::destroyAt(std::size_t x, std::size_t y)
{
    m_base[x + y * GRID_SIZE].reset();
}
void GameGrid::moveAt(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2)
{
    m_base[x2 + y2 * GRID_SIZE] = std::move(m_base[x1 + y1 * GRID_SIZE]);
    m_base[x2 + y2 * GRID_SIZE]->setTransform({gridIndicesToPosition(std::make_pair(x2, y2))});
}
void GameGrid::moveAt(std::pair<std::size_t, std::size_t> indices1, std::pair<std::size_t, std::size_t> indices2)
{
    moveAt(indices1.first, indices1.second, indices2.first, indices2.second); 
}

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices);

void Game::activatePlayerSquares()
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    std::bitset<GRID_SIZE * GRID_SIZE> setSquares {};
    for(int i {}; i < m_grid.size(); ++i)
    {
        if(m_grid[i] && m_grid[i]->isTeamOne() == m_playerOneToPlay)
            setSquares.set(i);
    }

    uiManagerInstance.setGameGridSquares(std::move(setSquares));
}

static constexpr int32_t PLAYER_STARTING_MONEY = 1000;
Game::Game(bool onePlayer) : m_grid(this), m_onePlayer(onePlayer), m_playersMoney(std::make_pair(PLAYER_STARTING_MONEY, PLAYER_STARTING_MONEY))
{
    static RenderEngine& renderEngineInstance = RenderEngine::getInstance();
    static UIManager& uiManagerInstance = UIManager::getInstance();
    updateStatusTexts();
    m_grid.initializeAt<AircraftCarrierUnit>(0, 0, true);
    m_grid.initializeAt<AircraftCarrierUnit>(9, 9, true);
    m_grid.initializeAt<SubmarineUnit>(9, 3, true);
    m_grid.initializeAt<AircraftCarrierUnit>(8, 9, true);
    auto r = Random::getInstance().get<std::size_t>(0, GRID_SIZE - 1);
    m_grid.initializeAt<AircraftCarrierUnit>(2, r, false);
    m_grid.initializeAt<SubmarineUnit>(1, 2, false);
    activatePlayerSquares();
    uiManagerInstance.disableGameActionButtons(true);
}
int32_t Game::getMoney() const
{
    return m_playerOneToPlay ? m_playersMoney.first : m_playersMoney.second;
}
void Game::addMoney(int32_t money)
{
    if(m_playerOneToPlay) m_playersMoney.first += money;
    else m_playersMoney.second -= money;
    updateStatusTexts();
}
void Game::updateStatusTexts()
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    uiManagerInstance.updateGameStatusTexts({m_playerOneToPlay, getMoney()});
}
void Game::receiveGameInput(std::size_t index, ButtonTypes buttonType)
{
    static UIManager& uiManagerInstance = UIManager::getInstance();
    static ActionCallbackManager& actionCallbackManagerInstance = ActionCallbackManager::getInstance();
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
                break;
            case ActionTypes::immediate:
                m_selectedActionIndex.reset();
                returnToPlayerUnitSelection();
                break;
            case ActionTypes::nothing:
                m_selectedActionIndex.reset();
                break;
            }
        }
        break;
    
    case ButtonTypes::GridSquare:
        auto selectedActionSquare = GameGrid::convertIndexToIndices(index);
        if(m_selectedActionIndex)
        {
            actionCallbackManagerInstance.invoke<SelectSquareCallback>(this, selectedActionSquare.first, selectedActionSquare.second);
            uiManagerInstance.removeDisabledColorToGridSquare(GameGrid::convertIndicesToIndex(m_selectedUnitIndices.value()));
            uiManagerInstance.removeSavedSelection();
            returnToPlayerUnitSelection();
            m_selectedActionIndex.reset();
        }
        else
        {
            uiManagerInstance.saveCurrentSelection();
            static constexpr auto gridSquareSelectedColor = glm::vec3(.7f, .9f, .2f);
            m_selectedUnitIndices = selectedActionSquare;
            uiManagerInstance.enableGameActionButtons(m_grid[index]->getActionData());
            uiManagerInstance.addDisabledColorToGridSquare(index, gridSquareSelectedColor);
            uiManagerInstance.setGameGridSquares({});
        }
        break;
    }
}

glm::vec3 gridIndicesToPosition(std::pair<std::size_t, std::size_t>&& gridIndices)
{
    static const float squareSize {2.f / GRID_SIZE};

    return glm::vec3(-1.f + squareSize * gridIndices.first + squareSize / 2.f, 0.f, 
        -1.f + squareSize * gridIndices.second + squareSize / 2.f);
}