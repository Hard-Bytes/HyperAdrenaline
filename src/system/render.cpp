#include "render.hpp"

// Constructor
RenderSystem::RenderSystem(GameContext& gc, HUDSystem& hs, ParticleSystem* ps)
    : gctx(gc), hud(hs), particles(ps)
{}

// Destructor
RenderSystem::~RenderSystem()
{}

void RenderSystem::update(bool updateGameData, bool drawUI)
{
    gctx.graphicsEngine.renderAll();
    if(drawUI) hud.renderAll(updateGameData);
}

void 
RenderSystem::updateAll(bool updateGameData, bool drawUI)
{
    gctx.graphicsEngine.beginRender();

    // Render all 3D nodes and HUD elements
    gctx.graphicsEngine.renderAll();
    if(drawUI) hud.renderAll(updateGameData);

    // Partículas
    if(particles)
    {
        particles->renderAll();
    }

    gctx.graphicsEngine.endRender();
}

void 
RenderSystem::renderLevelSelection(
    bool isOnline
    , const std::vector<std::string>& playerNames )
{
    gctx.graphicsEngine.beginRender();

    // Render all 3D nodes and HUD elements
    gctx.graphicsEngine.renderAll();
    hud.renderAll(false);

    if(isOnline)
    {
        float height = 50;
        float offset = 50;
        for(auto& name : playerNames)
        {
            gctx.graphicsEngine.draw2DString(
                name
                , {900, height}
                , {0, 0, 0}
                , 40
            );
            height += offset;
        }
    }

    gctx.graphicsEngine.endRender();
}

