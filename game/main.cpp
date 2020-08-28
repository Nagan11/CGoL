//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <iostream>
#include <fstream>
#include <chrono>
#include <future>
#include <thread>
#include <ctime>
#include <mach-o/dyld.h>

#include <SFML/Graphics.hpp>

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>

#include "AdvancedMouse.hpp"
#include "RectangleBounds.hpp"
#include "RectangleButton.hpp"
#include "ImprovedText.hpp"
#include "ResourcePath.hpp"

const int MIN_TIMER_VALUE = 100;
const int MAX_TIMER_VALUE = 1000;

int _fieldWidth = 50, _fieldHeight = 36;
std::vector< std::vector<bool> > _lastMatrix(_fieldWidth, std::vector<bool>(_fieldHeight, false));
std::queue< std::vector< std::vector<bool> > > _nextTurns;

std::future<void> _calculationHandle;
bool _exceptionBool = false;

enum GameStage
{
    DrawChoice,
    DrawField,
    Game
};


// <Helper functions>
void clearQueue(std::queue< std::vector< std::vector<bool> > > &q)
{
   std::queue< std::vector< std::vector<bool> > > empty;
   std::swap(q, empty);
}

std::string cropZeroes(std::string s)
{
    for (int i = s.size() - 1; i >= 0; i--)
    {
        if (s[i] == '0' || s[i] == '.')
        {
            s.pop_back();
        }
        else
        {
            return s;
        }
    }
    return s;
}

std::string doubleToString(double d)
{
    return cropZeroes(std::to_string(d));
}
// </Helper functions>

// <Game logic>
void tileSetToMatrix(std::vector< std::vector<RectangleButton> >& tileSet, std::vector< std::vector<bool> >& matrix)
{
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            if (tileSet[i][j].getFillColor() == sf::Color::White)
            {
                matrix[i][j] = true;
            }
            else
            {
                matrix[i][j] = false;
            }
        }
    }
}

void matrixToTileSet(std::vector< std::vector<RectangleButton> >& tileSet, std::vector< std::vector<bool> >& matrix)
{
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            if (matrix[i][j])
            {
                tileSet[i][j].setFillColor(sf::Color::White);
            }
            else
            {
                tileSet[i][j].setFillColor(sf::Color::Black);
            }
        }
    }
}

bool cellIsLucky()
{
    srand(time(0) * rand());
    if ((rand() % 100) > 70)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void placeRandomCells(std::vector< std::vector<bool> >& matrix)
{
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            matrix[i][j] = cellIsLucky();
        }
    }
}

int amountOfNeighbors(int i, int j, std::vector< std::vector<bool> >& matrix)
{
    int counter = 0;
    if (i > 0 && i < (_fieldWidth - 1) && j > 0 && j < (_fieldHeight - 1))
    {
        if (matrix[i - 1][j - 1])
        {
            counter++;
        }
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i - 1][j + 1])
        {
            counter++;
        }
        if (matrix[i][j - 1])
        {
            counter++;
        }
        if (matrix[i][j + 1])
        {
            counter++;
        }
        if (matrix[i + 1][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
        if (matrix[i + 1][j + 1])
        {
            counter++;
        }
    }
    else if (i == 0 && j > 0 && j < (_fieldHeight - 1))
    {
        if (matrix[i][j - 1])
        {
            counter++;
        }
        if (matrix[i][j + 1])
        {
            counter++;
        }
        if (matrix[i + 1][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
        if (matrix[i + 1][j + 1])
        {
            counter++;
        }
    }
    else if (i == (_fieldWidth - 1) && j > 0 && j < (_fieldHeight - 1))
    {
        if (matrix[i - 1][j - 1])
        {
            counter++;
        }
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i - 1][j + 1])
        {
            counter++;
        }
        if (matrix[i][j - 1])
        {
            counter++;
        }
        if (matrix[i][j + 1])
        {
            counter++;
        }
    }
    else if (i > 0 && i < (_fieldWidth - 1) && j == 0)
    {
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i - 1][j + 1])
        {
            counter++;
        }
        if (matrix[i][j + 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
        if (matrix[i + 1][j + 1])
        {
            counter++;
        }
    }
    else if (i > 0 && i < (_fieldWidth - 1) && j == (_fieldHeight - 1))
    {
        if (matrix[i - 1][j - 1])
        {
            counter++;
        }
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
    }
    else if (i == 0 && j == 0)
    {
        if (matrix[i][j + 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
        if (matrix[i + 1][j + 1])
        {
            counter++;
        }
    }
    else if (i == 0 && j == (_fieldHeight - 1))
    {
        if (matrix[i][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j - 1])
        {
            counter++;
        }
        if (matrix[i + 1][j])
        {
            counter++;
        }
    }
    else if (i == (_fieldWidth - 1) && j == 0)
    {
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i - 1][j + 1])
        {
            counter++;
        }
        if (matrix[i][j + 1])
        {
            counter++;
        }
    }
    else if (i == (_fieldWidth - 1) && j == (_fieldHeight - 1))
    {
        if (matrix[i - 1][j - 1])
        {
            counter++;
        }
        if (matrix[i - 1][j])
        {
            counter++;
        }
        if (matrix[i][j - 1])
        {
            counter++;
        }
    }
    return counter;
}

void calculateNextTurn()
{
    int temp;
    std::vector< std::vector<bool> > newMatrix(_fieldWidth, std::vector<bool>(_fieldHeight));
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            temp = amountOfNeighbors(i, j, _lastMatrix);
            
            if (_lastMatrix[i][j])
            {
                if (temp == 2 || temp == 3)
                {
                    newMatrix[i][j] = true;
                }
                else
                {
                    newMatrix[i][j] = false;
                }
            }
            else
            {
                if (temp == 3)
                {
                    newMatrix[i][j] = true;
                }
                else
                {
                    newMatrix[i][j] = false;
                }
            }
        }
    }
    
    _nextTurns.push(newMatrix);
    _lastMatrix = newMatrix;
}

void setNextTurn(std::vector< std::vector<RectangleButton> >& tileSet, unsigned int& amountOfTurns, ImprovedText& amountOfTurnsText)
{
    if (_nextTurns.size() > 0)
    {
        matrixToTileSet(tileSet, _nextTurns.front());
        amountOfTurns++;
        amountOfTurnsText.setText(std::to_string(amountOfTurns));
        _nextTurns.pop();
    }
}

void calculateNextTurnsSecondThread()
{
    while (true)
    {
        if (_exceptionBool) // cause exception to stop function
        {
            throw std::runtime_error("stop");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (_nextTurns.size() < 10)
        {
            calculateNextTurn();
        }
    }
}
// </Game logic>

void windowStart(sf::RenderWindow& window)
{
    window.setFramerateLimit(120);
    window.clear(sf::Color::Black);
    window.display();
}

// <DrawChoice functions>
void enableDrawChoice(RectangleButton& randomFieldFill, RectangleButton& drawField)
{
    randomFieldFill.interactable = true;
    drawField.interactable = true;
}

void displayDrawChoice(sf::RenderWindow& window, RectangleButton& randomFieldFill, ImprovedText& randomFieldFillText,
                       RectangleButton& drawField, ImprovedText& drawFieldText)
{
    window.clear(sf::Color::Black);
    
    window.draw(randomFieldFill);
    window.draw(randomFieldFillText);
    
    window.draw(drawField);
    window.draw(drawFieldText);
    
    window.display();
}

void updateDrawChoice(sf::RenderWindow& window, AdvancedMouse& mouse,
                      RectangleButton& randomFieldFill, RectangleButton& drawField)
{
    mouse.updateCondition(window);
    randomFieldFill.updateButtonCondition(mouse, window);
    drawField.updateButtonCondition(mouse, window);
}

void disableDrawChoice(sf::RenderWindow& window, RectangleButton& randomFieldFill, RectangleButton& drawField)
{
    randomFieldFill.interactable = false;
    drawField.interactable = false;
}
// </DrawChoice functions>

// <DrawField functions>
void enableDrawField(RectangleButton& colorSwitcher, RectangleButton& gridSwitcher, RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                     RectangleButton& eraseAllButton, RectangleButton& moveToGameStageButton, std::vector< std::vector<RectangleButton> >& tileSet)
{
    colorSwitcher.interactable = true;
    gridSwitcher.interactable = true;
    minusSpeedButton.interactable = true;
    plusSpeedButton.interactable = true;
    eraseAllButton.interactable = true;
    moveToGameStageButton.interactable = true;
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            tileSet[i][j].interactable = true;
        }
    }
}

void displayDrawField(sf::RenderWindow& window, sf::RectangleShape& divisorRect,
                      RectangleButton& colorSwitcher,         ImprovedText& colorSwitcherText,
                      RectangleButton& gridSwitcher,          ImprovedText& gridSwitcherText,
                      sf::RectangleShape& speedSwitchBackground, ImprovedText& currentSpeedText,
                      RectangleButton& minusSpeedButton,      ImprovedText& minusSpeedButtonText,
                      RectangleButton& plusSpeedButton,       ImprovedText& plusSpeedButtonText,
                      RectangleButton& eraseAllButton,        ImprovedText& eraseAllButtonText,
                      RectangleButton& moveToGameStageButton, ImprovedText& moveToGameStageButtonText,
                      std::vector<sf::RectangleShape>& grid, std::vector< std::vector<RectangleButton> >& tileSet)
{
    window.clear(sf::Color::Black);
    
    window.draw(colorSwitcher);
    window.draw(colorSwitcherText);
    
    window.draw(gridSwitcher);
    window.draw(gridSwitcherText);
    
    window.draw(speedSwitchBackground);
    window.draw(currentSpeedText);
    window.draw(minusSpeedButton);
    window.draw(minusSpeedButtonText);
    window.draw(plusSpeedButton);
    window.draw(plusSpeedButtonText);
    
    window.draw(eraseAllButton);
    window.draw(eraseAllButtonText);
    
    window.draw(moveToGameStageButton);
    window.draw(moveToGameStageButtonText);
    
    window.draw(divisorRect);
    
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            window.draw(tileSet[i][j]);
        }
    }
    
    for (int i = 0; i < grid.size(); i++)
    {
        window.draw(grid[i]);
    }
    
    window.display();
}

void updateDrawField(sf::RenderWindow& window, AdvancedMouse& mouse,
                     RectangleButton& colorSwitcher,    RectangleButton& gridSwitcher,
                     RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                     RectangleButton& eraseAllButton,   RectangleButton& moveToGameStageButton,
                     std::vector< std::vector<RectangleButton> >& tileSet)
{
    mouse.updateCondition(window);
    colorSwitcher.updateButtonCondition(mouse, window);
    gridSwitcher.updateButtonCondition(mouse, window);
    minusSpeedButton.updateButtonCondition(mouse, window);
    plusSpeedButton.updateButtonCondition(mouse, window);
    eraseAllButton.updateButtonCondition(mouse, window);
    moveToGameStageButton.updateButtonCondition(mouse, window);
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            tileSet[i][j].updateButtonCondition(mouse, window);
        }
    }
}

void disableDrawField(RectangleButton& colorSwitcher, RectangleButton& moveToGameStageButton,
                      std::vector< std::vector<RectangleButton> >& tileSet,
                      ImprovedText& colorSwitcherText, sf::Color& drawColor)
{
    colorSwitcher.interactable = false;
    moveToGameStageButton.interactable = false;
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            tileSet[i][j].interactable = false;
        }
    }
    
    colorSwitcherText.setText("White");
    drawColor = sf::Color::White;
}
// </DrawField functions>

// <Game functions>
void enableGame(RectangleButton& gridSwitcher, RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                RectangleButton& newGameButton, RectangleButton& pauseButton)
{
    gridSwitcher.interactable = true;
    minusSpeedButton.interactable = true;
    plusSpeedButton.interactable = true;
    newGameButton.interactable = true;
    pauseButton.interactable = true;
}

void initializeGame(std::vector< std::vector<RectangleButton> >& tileSet, bool isRandom,
                    std::chrono::steady_clock::time_point& timePoint)
{
    if (isRandom)
    {
        placeRandomCells(_lastMatrix);
        matrixToTileSet(tileSet, _lastMatrix);
    }
    else
    {
        tileSetToMatrix(tileSet, _lastMatrix);
    }
    
    _exceptionBool = false;
    if (_nextTurns.size() != 0)
    {
        clearQueue(_nextTurns);
    }
    _calculationHandle = std::async(calculateNextTurnsSecondThread);
    
    timePoint = std::chrono::steady_clock::now();
}

void displayGame(sf::RenderWindow& window, sf::RectangleShape& divisorRect,
                 sf::RectangleShape& speedSwitchBackground, ImprovedText& currentSpeedText,
                 sf::RectangleShape& amountOfTurnsRect, ImprovedText& amountOfTurnsText,
                 RectangleButton&    gridSwitcher,      ImprovedText& gridSwitcherText,
                 RectangleButton&    minusSpeedButton,  ImprovedText& minusSpeedButtonText,
                 RectangleButton&    plusSpeedButton,   ImprovedText& plusSpeedButtonText,
                 RectangleButton&    newGameButton,     ImprovedText& newGameButtonText,
                 RectangleButton&    pauseButton,       ImprovedText& pauseButtonText,
                 std::vector<sf::RectangleShape>& grid, std::vector< std::vector<RectangleButton> >& tileSet,
                 sf::RectangleShape& aysBackground, ImprovedText& aysText,
                 RectangleButton&    aysYes,        ImprovedText& aysYesText,
                 RectangleButton&    aysNo,         ImprovedText& aysNoText,
                 std::vector<sf::RectangleShape>& aysBackgroundBorders, bool aysMode)
{
    window.clear(sf::Color::Black);
    
    // <Standart display>
    window.draw(divisorRect);
    
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            window.draw(tileSet[i][j]);
        }
    }
    
    window.draw(amountOfTurnsRect);
    window.draw(gridSwitcher);
    window.draw(speedSwitchBackground);
    window.draw(currentSpeedText);
    window.draw(minusSpeedButton);
    window.draw(plusSpeedButton);
    window.draw(newGameButton);
    window.draw(pauseButton);
    
    for (int i = 0; i < grid.size(); i++)
    {
        window.draw(grid[i]);
    }
    // </Standart display>
    
    if (aysMode)
    {
        window.draw(aysBackground);
        for (int i = 0; i < 4; i++)
        {
            window.draw(aysBackgroundBorders[i]);
        }
        window.draw(aysText);
        
        window.draw(aysYes);
        window.draw(aysYesText);
        
        window.draw(aysNo);
        window.draw(aysNoText);
    }
    else
    {
        window.draw(amountOfTurnsText);
        window.draw(gridSwitcherText);
        window.draw(minusSpeedButtonText);
        window.draw(plusSpeedButtonText);
        window.draw(newGameButtonText);
        window.draw(pauseButtonText);
    }
    
    window.display();
}

void updateGame(sf::RenderWindow& window, AdvancedMouse& mouse,
                RectangleButton& gridSwitcher, RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                RectangleButton& newGameButton, RectangleButton& pauseButton,
                RectangleButton& aysYes, RectangleButton& aysNo, bool aysMode)
{
    mouse.updateCondition(window);
    if (aysMode)
    {
        aysYes.updateButtonCondition(mouse, window);
        aysNo.updateButtonCondition(mouse, window);
    }
    else
    {
        gridSwitcher.updateButtonCondition(mouse, window);
        minusSpeedButton.updateButtonCondition(mouse, window);
        plusSpeedButton.updateButtonCondition(mouse, window);
        newGameButton.updateButtonCondition(mouse, window);
        pauseButton.updateButtonCondition(mouse, window);
    }
}

void disableGame(unsigned int& amountOfTurns,
                 std::vector<sf::RectangleShape>& grid, std::vector< std::vector<RectangleButton> >& tileSet,
                 RectangleButton& gridSwitcher, RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                 RectangleButton& newGameButton, RectangleButton& pauseButton,
                 ImprovedText& amountOfTurnsText, ImprovedText& gridSwitcherText, ImprovedText& pauseButtonText)
{
    amountOfTurns = 0;
    
    for (int i = 0; i < grid.size(); i++)
    {
        grid[i].setFillColor(sf::Color::Black);
    }
    
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            tileSet[i][j].setFillColor(sf::Color::Black);
        }
    }
    
    gridSwitcher.interactable = false;
    minusSpeedButton.interactable = false;
    plusSpeedButton.interactable = false;
    newGameButton.interactable = false;
    pauseButton.interactable = false;
    
    amountOfTurnsText.setText("0");
    gridSwitcherText.setText("Grid: OFF");
    pauseButtonText.setText("Pause");
    
    _exceptionBool = true;
}

void darkenGame(sf::RectangleShape& divisorRect,
                std::vector< std::vector<RectangleButton> >& tileSet, std::vector<sf::RectangleShape>& grid,
                sf::RectangleShape& amountOfTurnsRect, sf::RectangleShape& speedSwitchBackground,
                RectangleButton& gridSwitcher,  RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                RectangleButton& newGameButton, RectangleButton& pauseButton,
                sf::Color& darkGrid, sf::Color& darkTileSet)
{
    divisorRect.setFillColor(sf::Color(128, 128, 128, 255));
    
    amountOfTurnsRect.setFillColor(sf::Color(128, 128, 128, 255));
    gridSwitcher.setFillColor(sf::Color(128, 128, 128, 255));
    speedSwitchBackground.setFillColor(sf::Color(128, 128, 128, 255));
    minusSpeedButton.setFillColor(sf::Color(128, 128, 128, 255));
    plusSpeedButton.setFillColor(sf::Color(128, 128, 128, 255));
    newGameButton.setFillColor(sf::Color(128, 128, 128, 255));
    pauseButton.setFillColor(sf::Color(128, 128, 128, 255));
    
    if (grid[0].getFillColor() == sf::Color::Red)
    {
        for (int i = 0; i < grid.size(); i++)
        {
            grid[i].setFillColor(darkGrid);
        }
    }
    
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            if (tileSet[i][j].getFillColor() == sf::Color::White)
            {
                tileSet[i][j].setFillColor(darkTileSet);
            }
        }
    }
}

void lightenGame(sf::RectangleShape& divisorRect,
                 std::vector< std::vector<RectangleButton> >& tileSet, std::vector<sf::RectangleShape>& grid,
                 sf::RectangleShape& amountOfTurnsRect, sf::RectangleShape& speedSwitchBackground,
                 RectangleButton& gridSwitcher,  RectangleButton& minusSpeedButton, RectangleButton& plusSpeedButton,
                 RectangleButton& newGameButton, RectangleButton& pauseButton,
                 sf::Color& darkGrid, sf::Color& darkTileSet)
{
    divisorRect.setFillColor(sf::Color::White);
    
    amountOfTurnsRect.setFillColor(sf::Color::White);
    gridSwitcher.setFillColor(sf::Color::White);
    speedSwitchBackground.setFillColor(sf::Color::White);
    minusSpeedButton.setFillColor(sf::Color::White);
    plusSpeedButton.setFillColor(sf::Color::White);
    newGameButton.setFillColor(sf::Color::White);
    pauseButton.setFillColor(sf::Color::White);
    
    if (grid[0].getFillColor() == darkGrid)
    {
        for (int i = 0; i < grid.size(); i++)
        {
            grid[i].setFillColor(sf::Color::Red);
        }
    }
    
    for (int i = 0; i < _fieldWidth; i++)
    {
        for (int j = 0; j < _fieldHeight; j++)
        {
            if (tileSet[i][j].getFillColor() == darkTileSet)
            {
                tileSet[i][j].setFillColor(sf::Color::White);
            }
        }
    }
}

// </Game functions>

int main(int, char const**)
{
    // <MainInitialization>
    sf::RenderWindow mainWindow(sf::VideoMode(1280, 720), "Conway's Game of Life", sf::Style::Titlebar | sf::Style::Close);
    
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "icon.png"))
    {
        std::cout << "hell no" << std::endl;
    }
    mainWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    
    windowStart(mainWindow);
    
    AdvancedMouse mouse;
    
    sf::Font courierNewFont;
    if (!courierNewFont.loadFromFile(resourcePath() + "Courier New.ttf"))
    {
        std::cout << "hell no" << std::endl;
    }
    // </MainInitialization>
    
    // <DrawChoiceInitialization>
    RectangleButton randomFieldFill(sf::Vector2f(320.f, 150.f), sf::Vector2f(640.f, 150.f), sf::Color::White);
    randomFieldFill.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText randomFieldFillText("Use random field", courierNewFont, 80, sf::Color::Black, randomFieldFill.getGlobalBounds());
    

    RectangleButton drawField(sf::Vector2f(320.f, 420.f), sf::Vector2f(640.f, 150.f), sf::Color::White);
    drawField.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText drawFieldText("Draw field", courierNewFont, 80, sf::Color::Black, drawField.getGlobalBounds());
    // </DrawChoiceInitialization>
    
    
    
    // <DrawFieldInitialization>
    sf::RectangleShape divisorRect;
    divisorRect.setPosition(sf::Vector2f(274.f, 0.f));
    divisorRect.setSize(sf::Vector2f(5.f, 720.f));
    divisorRect.setFillColor(sf::Color::White);
    

    RectangleButton colorSwitcher(sf::Vector2f(50.f, 50.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    colorSwitcher.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText colorSwitcherText("White", courierNewFont, 50, sf::Color::Black, colorSwitcher.getGlobalBounds());
    

    RectangleButton gridSwitcher(sf::Vector2f(50.f, 175.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    gridSwitcher.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText gridSwitcherText("Grid: OFF", courierNewFont, 40, sf::Color::Black, gridSwitcher.getGlobalBounds());
    
    
    _fieldWidth--;
    _fieldHeight--;
    std::vector<sf::RectangleShape> grid(_fieldWidth + _fieldHeight);
    float posX = 298.f, posY = 1.f;
    int gridPlaceCounter = 0;
    for ( ; gridPlaceCounter < _fieldWidth; gridPlaceCounter++)
    {
        grid[gridPlaceCounter].setPosition(sf::Vector2f(posX, posY));
        grid[gridPlaceCounter].setSize(sf::Vector2f(2.f, 718.f));
        grid[gridPlaceCounter].setFillColor(sf::Color::Black);
        posX += 20.f;
    }
    posX = 280.f;
    posY = 19.f;
    for ( ; gridPlaceCounter < grid.size(); gridPlaceCounter++)
    {
        grid[gridPlaceCounter].setPosition(sf::Vector2f(posX, posY));
        grid[gridPlaceCounter].setSize(sf::Vector2f(998.f, 2.f));
        grid[gridPlaceCounter].setFillColor(sf::Color::Black);
        posY += 20.f;
    }
    sf::Color darkGrid = sf::Color(128, 0, 0, 255);
    _fieldWidth++;
    _fieldHeight++;
    
    
    int nextTurnTimer = 400;
    double textTurnTimer = 0.4;
    
    sf::RectangleShape speedSwitchBackground;
    speedSwitchBackground.setPosition(sf::Vector2f(50.f, 300.f));
    speedSwitchBackground.setSize(sf::Vector2f(174.f, 60.f));
    speedSwitchBackground.setFillColor(sf::Color::White);
    
    ImprovedText currentSpeedText("1/" + doubleToString(textTurnTimer) + "s", courierNewFont, 30, sf::Color::Black, speedSwitchBackground.getGlobalBounds());
    
    RectangleButton minusSpeedButton(sf::Vector2f(50.f, 362.f), sf::Vector2f(86.f, 58.f), sf::Color::White);
    minusSpeedButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText minusSpeedButtonText("-", courierNewFont, 40, sf::Color::Black, minusSpeedButton.getGlobalBounds());
    
    RectangleButton plusSpeedButton(sf::Vector2f(138.f, 362.f), sf::Vector2f(86.f, 58.f), sf::Color::White);
    plusSpeedButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText plusSpeedButtonText("+", courierNewFont, 40, sf::Color::Black, plusSpeedButton.getGlobalBounds());
    
    
    RectangleButton moveToGameStageButton(sf::Vector2f(50.f, 595.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    moveToGameStageButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText moveToGameStageButtonText("Start", courierNewFont, 50, sf::Color::Black, moveToGameStageButton.getGlobalBounds());
    
    
    RectangleButton eraseAllButton(sf::Vector2f(50.f, 470.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    eraseAllButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText eraseAllButtonText("Erase all", courierNewFont, 40, sf::Color::Black, eraseAllButton.getGlobalBounds());
    
    
    sf::Color drawColor = sf::Color::White;

    std::vector< std::vector<RectangleButton> > tileSet(50, std::vector<RectangleButton>(36));
    posX = 280.f;
    for (int i = 0; i < _fieldWidth; i++)
    {
        posY = 1.f;
        for (int j = 0; j < _fieldHeight; j++)
        {
            tileSet[i][j] = RectangleButton(sf::Vector2f(posX, posY), sf::Vector2f(18.f, 18.f), sf::Color::Black);
            tileSet[i][j].buttonType = ButtonType::DrawButton;
            posY += 20.f;
        }
        posX += 20.f;
    }
    sf::Color darkTileSet = sf::Color(128, 128, 128, 255);
    
    std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
    int timer;
    // </DrawFieldInitialization>
    
    
    
    // <GameInitialization>
    unsigned int amountOfTurns = 0;
    
    sf::RectangleShape amountOfTurnsRect;
    amountOfTurnsRect.setPosition(sf::Vector2f(50.f, 50.f));
    amountOfTurnsRect.setSize(sf::Vector2f(174.f, 75.f));
    amountOfTurnsRect.setFillColor(sf::Color::White);
    
    ImprovedText amountOfTurnsText("0", courierNewFont, 50, sf::Color::Black, amountOfTurnsRect.getGlobalBounds());
    
    
    RectangleButton newGameButton(sf::Vector2f(50.f, 470.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    newGameButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText newGameButtonText("New Game", courierNewFont, 40, sf::Color::Black, newGameButton.getGlobalBounds());
    
    
    bool gameIsPaused = false;
    bool gameWasPaused = false;
    
    RectangleButton pauseButton(sf::Vector2f(50.f, 595.f), sf::Vector2f(174.f, 75.f), sf::Color::White);
    pauseButton.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText pauseButtonText("Pause", courierNewFont, 50, sf::Color::Black, pauseButton.getGlobalBounds());
    
    
    bool aysMode = false;
    
    sf::RectangleShape aysBackground;
    aysBackground.setPosition(sf::Vector2f(320.f, 140.f));
    aysBackground.setSize(sf::Vector2f(640.f, 440.f));
    aysBackground.setFillColor(sf::Color::Black);
    
    ImprovedText aysText("Are you sure?", courierNewFont, 90, sf::Color::White, sf::Rect<float>(320.f, 140.f, 640.f, 220.f));
    
    std::vector<sf::RectangleShape> aysBackgroundBorders(4);
    
    aysBackgroundBorders[0].setPosition(sf::Vector2f(320.f, 140.f));
    aysBackgroundBorders[0].setSize(sf::Vector2f(2.f, 440.f));
    aysBackgroundBorders[0].setFillColor(sf::Color::White);
    
    aysBackgroundBorders[1].setPosition(sf::Vector2f(958.f, 140.f));
    aysBackgroundBorders[1].setSize(sf::Vector2f(2.f, 440.f));
    aysBackgroundBorders[1].setFillColor(sf::Color::White);
    
    aysBackgroundBorders[2].setPosition(sf::Vector2f(320.f, 140.f));
    aysBackgroundBorders[2].setSize(sf::Vector2f(640.f, 2.f));
    aysBackgroundBorders[2].setFillColor(sf::Color::White);
    
    aysBackgroundBorders[3].setPosition(sf::Vector2f(320.f, 578.f));
    aysBackgroundBorders[3].setSize(sf::Vector2f(640.f, 2.f));
    aysBackgroundBorders[3].setFillColor(sf::Color::White);
    
    RectangleButton aysYes(sf::Vector2f(360.f, 400.f), sf::Vector2f(260.f, 140.f), sf::Color::White);
    aysYes.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText aysYesText("Yes", courierNewFont, 80, sf::Color::Black, aysYes.getGlobalBounds());
    
    RectangleButton aysNo(sf::Vector2f(660.f, 400.f), sf::Vector2f(260.f, 140.f), sf::Color::White);
    aysNo.buttonType = ButtonType::ClickOnRelease;
    
    ImprovedText aysNoText("No", courierNewFont, 80, sf::Color::Black, aysNo.getGlobalBounds());
    
    
    // </GameInitialization>
    
    GameStage gameStage = GameStage::DrawChoice;
    enableDrawChoice(randomFieldFill, drawField);
    
    while (mainWindow.isOpen())
    {
        switch (gameStage)
        {
            case GameStage::DrawChoice:
                displayDrawChoice(mainWindow, randomFieldFill, randomFieldFillText, drawField, drawFieldText);
                updateDrawChoice(mainWindow, mouse, randomFieldFill, drawField);
                
                if (randomFieldFill.click)
                {
                    disableDrawChoice(mainWindow, randomFieldFill, drawField);
                    initializeGame(tileSet, true, timePoint);
                    enableGame(gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton);
                    gameStage = GameStage::Game;
                }
                
                if (drawField.click)
                {
                    gameStage = GameStage::DrawField;
                    disableDrawChoice(mainWindow, randomFieldFill, drawField);
                    enableDrawField(colorSwitcher, gridSwitcher, minusSpeedButton, plusSpeedButton, eraseAllButton, moveToGameStageButton, tileSet);
                }
                
                if (randomFieldFill.isPressed)
                {
                    randomFieldFill.setFillColor(sf::Color::Blue);
                    randomFieldFillText.setTextColor(sf::Color::White);
                }
                else
                {
                    randomFieldFill.setFillColor(sf::Color::White);
                    randomFieldFillText.setTextColor(sf::Color::Black);
                }
                
                if (drawField.isPressed)
                {
                    drawField.setFillColor(sf::Color::Blue);
                    drawFieldText.setTextColor(sf::Color::White);
                }
                else
                {
                    drawField.setFillColor(sf::Color::White);
                    drawFieldText.setTextColor(sf::Color::Black);
                }
                break; // DrawChoice
                
            case GameStage::DrawField:
                displayDrawField(mainWindow, divisorRect, colorSwitcher, colorSwitcherText, gridSwitcher, gridSwitcherText,
                                 speedSwitchBackground, currentSpeedText, minusSpeedButton, minusSpeedButtonText, plusSpeedButton, plusSpeedButtonText,
                                 eraseAllButton, eraseAllButtonText, moveToGameStageButton, moveToGameStageButtonText,
                                 grid, tileSet);
                updateDrawField(mainWindow, mouse, colorSwitcher, gridSwitcher, minusSpeedButton, plusSpeedButton, eraseAllButton, moveToGameStageButton, tileSet);
                
                if (colorSwitcher.click)
                {
                    if (drawColor == sf::Color::White)
                    {
                        drawColor = sf::Color::Black;
                        colorSwitcherText.setText("Black");
                    }
                    else
                    {
                        drawColor = sf::Color::White;
                        colorSwitcherText.setText("White");
                    }
                }

                if (gridSwitcher.click)
                {
                    if (grid[0].getFillColor() == sf::Color::Black)
                    {
                        for (int i = 0; i < grid.size(); i++)
                        {
                            grid[i].setFillColor(sf::Color::Red);
                        }
                        gridSwitcherText.setText("Grid: ON");
                    }
                    else
                    {
                        for (int i = 0; i < grid.size(); i++)
                        {
                            grid[i].setFillColor(sf::Color::Black);
                        }
                        gridSwitcherText.setText("Grid: OFF");
                    }
                }
                
                if (minusSpeedButton.click)
                {
                    if (nextTurnTimer < MAX_TIMER_VALUE)
                    {
                        nextTurnTimer += 10;
                        textTurnTimer += 0.01;
                        currentSpeedText.setText("1/" + doubleToString(textTurnTimer) + "s");
                    }
                }
                
                if (plusSpeedButton.click)
                {
                    if (nextTurnTimer > MIN_TIMER_VALUE)
                    {
                        nextTurnTimer -= 10;
                        textTurnTimer -= 0.01;
                        currentSpeedText.setText("1/" + doubleToString(textTurnTimer) + "s");
                    }
                }
                
                if (eraseAllButton.click)
                {
                    for (int i = 0; i < _fieldWidth; i++)
                    {
                        for (int j = 0; j < _fieldHeight; j++)
                        {
                            tileSet[i][j].setFillColor(sf::Color::Black);
                        }
                    }
                }
                
                if (moveToGameStageButton.click)
                {
                    disableDrawField(colorSwitcher, moveToGameStageButton, tileSet, colorSwitcherText, drawColor);
                    initializeGame(tileSet, false, timePoint);
                    enableGame(gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton);
                    gameStage = GameStage::Game;
                }
                
                for (int i = 0; i < _fieldWidth; i++)
                {
                    for (int j = 0; j < _fieldHeight; j++)
                    {
                        if (tileSet[i][j].isPressed)
                        {
                            tileSet[i][j].setFillColor(drawColor);
                        }
                        mainWindow.draw(tileSet[i][j]);
                    }
                }

                if (colorSwitcher.isPressed)
                {
                    colorSwitcher.setFillColor(sf::Color::Blue);
                    colorSwitcherText.setTextColor(sf::Color::White);
                }
                else
                {
                    colorSwitcher.setFillColor(sf::Color::White);
                    colorSwitcherText.setTextColor(sf::Color::Black);
                }

                if (gridSwitcher.isPressed)
                {
                    gridSwitcher.setFillColor(sf::Color::Blue);
                    gridSwitcherText.setTextColor(sf::Color::White);
                }
                else
                {
                    gridSwitcher.setFillColor(sf::Color::White);
                    gridSwitcherText.setTextColor(sf::Color::Black);
                }
                
                if (minusSpeedButton.isPressed)
                {
                    minusSpeedButton.setFillColor(sf::Color::Blue);
                    minusSpeedButtonText.setTextColor(sf::Color::White);
                }
                else
                {
                    minusSpeedButton.setFillColor(sf::Color::White);
                    minusSpeedButtonText.setTextColor(sf::Color::Black);
                }
                
                if (plusSpeedButton.isPressed)
                {
                    plusSpeedButton.setFillColor(sf::Color::Blue);
                    plusSpeedButtonText.setTextColor(sf::Color::White);
                }
                else
                {
                    plusSpeedButton.setFillColor(sf::Color::White);
                    plusSpeedButtonText.setTextColor(sf::Color::Black);
                }
                
                if (eraseAllButton.isPressed)
                {
                    eraseAllButton.setFillColor(sf::Color::Blue);
                    eraseAllButtonText.setTextColor(sf::Color::White);
                }
                else
                {
                    eraseAllButton.setFillColor(sf::Color::White);
                    eraseAllButtonText.setTextColor(sf::Color::Black);
                }
                
                if (moveToGameStageButton.isPressed)
                {
                    moveToGameStageButton.setFillColor(sf::Color::Blue);
                    moveToGameStageButtonText.setTextColor(sf::Color::White);
                }
                else
                {
                    moveToGameStageButton.setFillColor(sf::Color::White);
                    moveToGameStageButtonText.setTextColor(sf::Color::Black);
                }
                break; // DrawField
                
            case GameStage::Game:
                displayGame(mainWindow, divisorRect, speedSwitchBackground, currentSpeedText,
                            amountOfTurnsRect, amountOfTurnsText, gridSwitcher, gridSwitcherText,
                            minusSpeedButton, minusSpeedButtonText, plusSpeedButton, plusSpeedButtonText,
                            newGameButton, newGameButtonText, pauseButton, pauseButtonText, grid, tileSet,
                            aysBackground, aysText, aysYes, aysYesText, aysNo, aysNoText, aysBackgroundBorders, aysMode);
                updateGame(mainWindow, mouse, gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton, aysYes, aysNo, aysMode);
                
                if (gameIsPaused)
                {
                    timePoint = std::chrono::steady_clock::now() - std::chrono::milliseconds(timer);
                }
                else
                {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timePoint).count() > nextTurnTimer)
                    {
                        timePoint = std::chrono::steady_clock::now();
                        setNextTurn(tileSet, amountOfTurns, amountOfTurnsText);
                    }
                }
                
                if (gridSwitcher.click)
                {
                    if (grid[0].getFillColor() == sf::Color::Black)
                    {
                        for (int i = 0; i < grid.size(); i++)
                        {
                            grid[i].setFillColor(sf::Color::Red);
                        }
                        gridSwitcherText.setText("Grid: ON");
                    }
                    else
                    {
                        for (int i = 0; i < grid.size(); i++)
                        {
                            grid[i].setFillColor(sf::Color::Black);
                        }
                        gridSwitcherText.setText("Grid: OFF");
                    }
                }
                
                if (minusSpeedButton.click)
                {
                    if (nextTurnTimer < MAX_TIMER_VALUE)
                    {
//                        timePoint = std::chrono::steady_clock::now();
                        nextTurnTimer += 10;
                        textTurnTimer += 0.01;
                        currentSpeedText.setText("1/" + doubleToString(textTurnTimer) + "s");
                    }
                }
                
                if (plusSpeedButton.click)
                {
                    if (nextTurnTimer > MIN_TIMER_VALUE)
                    {
//                        timePoint = std::chrono::steady_clock::now();
                        nextTurnTimer -= 10;
                        textTurnTimer -= 0.01;
                        currentSpeedText.setText("1/" + doubleToString(textTurnTimer) + "s");
                    }
                }
                
                std::cout << minusSpeedButton.interactable << std::endl;
                
                if (newGameButton.click)
                {
                    if (gameIsPaused)
                    {
                        gameWasPaused = true;
                    }
                    else
                    {
                        gameWasPaused = false;
                    }
                    gameIsPaused = true;
                    timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timePoint).count();
                    
                    aysMode = true;
                    darkenGame(divisorRect, tileSet, grid, amountOfTurnsRect, speedSwitchBackground,
                               gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton, darkGrid, darkTileSet);
                    
                    gridSwitcher.interactable = false;
                    newGameButton.interactable = false;
                    pauseButton.interactable = false;
                    
                    aysYes.interactable = true;
                    aysNo.interactable = true;
                    newGameButton.click = false;
                }
                
                if (pauseButton.click)
                {
                    gameIsPaused = !gameIsPaused;
                    if (gameIsPaused)
                    {
                        pauseButtonText.setText("Play");
                        timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timePoint).count();
                    }
                    else
                    {
                        pauseButtonText.setText("Pause");
                    }
                }
                
                if (aysYes.click)
                {
                    lightenGame(divisorRect, tileSet, grid, amountOfTurnsRect, speedSwitchBackground,
                                gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton, darkGrid, darkTileSet);
                    disableGame(amountOfTurns, grid, tileSet, gridSwitcher, minusSpeedButton, plusSpeedButton,
                                newGameButton, pauseButton, amountOfTurnsText, gridSwitcherText, pauseButtonText);
                    aysMode = false;
                    gameIsPaused = false;
                    
                    gridSwitcher.interactable = false;
                    newGameButton.interactable = false;
                    pauseButton.interactable = false;
                    
                    aysYes.interactable = false;
                    aysNo.interactable = false;
                    
                    gameStage = GameStage::DrawChoice;
                    enableDrawChoice(randomFieldFill, drawField);
                    
                    aysYes.click = false;
                    aysYes.isPressed = false;
                }
                
                if (aysNo.click)
                {
                    lightenGame(divisorRect, tileSet, grid, amountOfTurnsRect, speedSwitchBackground,
                                gridSwitcher, minusSpeedButton, plusSpeedButton, newGameButton, pauseButton, darkGrid, darkTileSet);
                    aysMode = false;
                    if (gameWasPaused)
                    {
                        gameIsPaused = true;
                    }
                    else
                    {
                        gameIsPaused = false;
                    }
                    
                    gridSwitcher.interactable = true;
                    newGameButton.interactable = true;
                    pauseButton.interactable = true;
                    
                    aysYes.interactable = false;
                    aysNo.interactable = false;
                    
                    aysNo.click = false;
                    aysNo.isPressed = false;
                }
                
                
                if (aysMode)
                {
                    if (aysYes.isPressed)
                    {
                        aysYes.setFillColor(sf::Color::Blue);
                        aysYesText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        aysYes.setFillColor(sf::Color::White);
                        aysYesText.setTextColor(sf::Color::Black);
                    }
                    
                    if (aysNo.isPressed)
                    {
                        aysNo.setFillColor(sf::Color::Blue);
                        aysNoText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        aysNo.setFillColor(sf::Color::White);
                        aysNoText.setTextColor(sf::Color::Black);
                    }
                }
                else
                {
                    if (gridSwitcher.isPressed)
                    {
                        gridSwitcher.setFillColor(sf::Color::Blue);
                        gridSwitcherText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        gridSwitcher.setFillColor(sf::Color::White);
                        gridSwitcherText.setTextColor(sf::Color::Black);
                    }
                    
                    if (minusSpeedButton.isPressed)
                    {
                        minusSpeedButton.setFillColor(sf::Color::Blue);
                        minusSpeedButtonText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        minusSpeedButton.setFillColor(sf::Color::White);
                        minusSpeedButtonText.setTextColor(sf::Color::Black);
                    }
                    
                    if (plusSpeedButton.isPressed)
                    {
                        plusSpeedButton.setFillColor(sf::Color::Blue);
                        plusSpeedButtonText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        plusSpeedButton.setFillColor(sf::Color::White);
                        plusSpeedButtonText.setTextColor(sf::Color::Black);
                    }
                    
                    if (newGameButton.isPressed)
                    {
                        newGameButton.setFillColor(sf::Color::Blue);
                        newGameButtonText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        newGameButton.setFillColor(sf::Color::White);
                        newGameButtonText.setTextColor(sf::Color::Black);
                    }
                    
                    if (pauseButton.isPressed)
                    {
                        pauseButton.setFillColor(sf::Color::Blue);
                        pauseButtonText.setTextColor(sf::Color::White);
                    }
                    else
                    {
                        pauseButton.setFillColor(sf::Color::White);
                        pauseButtonText.setTextColor(sf::Color::Black);
                    }
                }
                
                break; // Game
        }
        
        sf::Event event;
        while (mainWindow.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    _exceptionBool = true;
                    mainWindow.close();
                    break;
                    
                case sf::Event::LostFocus:
                    switch(gameStage)
                    {
                        case GameStage::DrawChoice:
                            randomFieldFill.interactable = false;
                            drawField.interactable = false;
                            break; // DrawChoice
                            
                        case GameStage::DrawField:
                            colorSwitcher.interactable = false;
                            gridSwitcher.interactable = false;
                            for (int i = 0; i < _fieldWidth; i++)
                            {
                                for (int j = 0; j < _fieldHeight; j++)
                                {
                                    tileSet[i][j].interactable = false;
                                }
                            }
                            break; // DrawField
                            
                        case GameStage::Game:
                            if (aysMode)
                            {
                                aysYes.interactable = false;
                                aysNo.interactable = false;
                            }
                            else
                            {
                                gridSwitcher.interactable = false;
                                newGameButton.interactable = false;
                                pauseButton.interactable = false;
                            }
                            break; // Game
                    }
                    break;
                    
                case sf::Event::GainedFocus:
                    switch(gameStage)
                    {
                        case GameStage::DrawChoice:
                            randomFieldFill.interactable = true;
                            drawField.interactable = true;
                            break; // DrawChoice
                            
                        case GameStage::DrawField:
                            colorSwitcher.interactable = true;
                            gridSwitcher.interactable = true;
                            for (int i = 0; i < _fieldWidth; i++)
                            {
                                for (int j = 0; j < _fieldHeight; j++)
                                {
                                    tileSet[i][j].interactable = true;
                                }
                            }
                            break; // DrawField
                            
                        case GameStage::Game:
                            if (aysMode)
                            {
                                aysYes.interactable = true;
                                aysNo.interactable = true;
                            }
                            else
                            {
                                gridSwitcher.interactable = true;
                                newGameButton.interactable = true;
                                pauseButton.interactable = true;
                            }
                            break; // Game
                    }
                    
                default:
                    break;
            }
        }
    }
}