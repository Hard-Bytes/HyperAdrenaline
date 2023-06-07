#include "stateManager.hpp"
#include <iostream>
#include "../util/macros.hpp"

void 
StateManager::update() noexcept {
    if( !isAlive() ) return;
        
    auto& state = m_states.top();

    if( state && state->isAlive() )
    {
        updateTopState();
    }
    else 
    {
        if(state) state->close();
        delete state;
        m_states.pop();

        if(!m_states.empty())
            m_states.top()->resume();
    }
    m_statePushedThisUpdate = false;
}

bool 
StateManager::isAlive() noexcept {
    return !m_states.empty();
}

void
StateManager::updateTopState() noexcept
{
    auto& state = m_states.top();
    state->update();

    // A transparent state will update the previous state too
    if(!m_statePushedThisUpdate && state->isTransparent)
    {
        m_states.pop();

        updateTopState();

        m_states.push(state);
    }
}
