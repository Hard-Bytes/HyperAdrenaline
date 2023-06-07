#include "game.hpp"
#include "states/gameState.hpp"
#include "states/menuState.hpp"
#include "util/macros.hpp"
#include "util/version.hpp"

Game::Game()
{}

Game::~Game()
{}

void Game::start()
{
    this->gameInit();
    this->gameRun();
    this->gameClose();
}

/*Game Init*/
// Initializes what the game needs to run
void Game::gameInit()
{
    graphicsEngine.createDevice(APP_NAME, APP_VERSION);
}

/*Game Run*/
// Runs the main game loop
void Game::gameRun()
{
    StateManager stateman;
    GameTimer timer;

    // Starting state
    stateman.pushState<MenuState>(stateman, gameContext);

    // Run while state manager is active (has states)
    while(stateman.isAlive())
    {
        timer.updateDelta();

        // Wait for delta accumulator to reach desired framerate
        auto lag = timer.getLag();
        while(lag >= timestep)
        {
            lag -= timestep;
            graphicsEngine.updateTime();
            stateman.update();
        }
        timer.setLag(lag);
    }
}

/*Game Close*/
// Safely closes and deletes game objects or data
void Game::gameClose()
{
    graphicsEngine.dropMainDevice();
}
