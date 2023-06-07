#pragma once
#include <stack>
#include <memory>
// #include <type_traits>
#include "../states/state.hpp"

using StateStack = std::stack<State*>;

struct StateManager
{
    explicit StateManager() = default;
    ~StateManager() {};

    template <typename State_t, typename... Args>
    void pushState(Args&&... args) {
        // maybe do a static assert between type child and new one to check if son
        //m_states.push( std::make_unique<State_t>( std::forward< decltype(args) >( args )... ) );
        State_t* state = new State_t(std::forward< decltype(args) >( args )...);
        m_states.push( state );
        m_states.top()->init();
        m_statePushedThisUpdate = true;
    }

    void update() noexcept;
    bool isAlive() noexcept;
private:
    StateStack m_states;
    bool m_statePushedThisUpdate { false };

    void updateTopState() noexcept;
};
