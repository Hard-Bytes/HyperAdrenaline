#pragma once
#include "pauseState.hpp"
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/allSystemsInclude.hpp"

struct GameState : public State
{
    explicit GameState(StateManager&, GameContext&, int desiredLevel, bool wantsOnline); // Initialize game
    ~GameState();
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    void endGameChecks() noexcept;

    StateManager& stateman;
    GameContext& gctx;

    HUDSystem           hudSys            {gctx};
    AISystem            aiSys             {gctx};
    BehaviourSystem     behaviourSys      {gctx};
    ParticleSystem      particleSys       {gctx};
    RenderSystem        renderSys         {gctx, hudSys, &particleSys};
    RespawnSystem       respawnSys        {gctx};
    RaidSystem          raidSys           {gctx};
    WeaponSystem        weaponSys         {gctx};
    CollisionSystem     collisionSys      {gctx};
    InputSystem         inputSys          {gctx};
    HealthSystem        healthSys         {gctx};
    StatusSystem        statusSys         {gctx};

    int m_startingLevelIndex;
    bool m_wantsOnline;

    int lastFPS {-1};

    bool m_usePhysicsOcclusion { false };

    bool m_backFromEndGameDecision { false };
    bool m_endGameDecisionBackToMenu { false };
    bool m_paused { false };
    bool m_alive { true };
};