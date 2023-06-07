#pragma once
#include <vector>
#include "../manager/settingsManager.hpp"
#include "../components/entity.hpp"
#include "componentManager.hpp"
#include "../sharedTypes/serverEnums.hpp"
#include "../util/enums.hpp"

#define INVALID_ENTITY -1
#define UNREACHABLE_ENTITY -2

class GameManager;

class EntityManager
{
public:
    // Constructor and destructor
    explicit EntityManager();
    ~EntityManager();

    EntityID createEntity();

    void updateDeadEntities();
    void markAsDead(EntityID entID);
    void clearAll();

    // Get an entity by ID
    Entity* getEntityByID(EntityID entID);

    // Get a component by ID
    template<typename Component_t>
    Component_t* getComponentByID(EntityID entID) { return cmpManager.getComponentByID_t<Component_t>(entID); }

    // Get the entity vector
    const std::vector<Entity>& getEntities() { return entities; }

    // Get a component vector
    template<typename Component_t>
    std::vector<Component_t>& getComponentVector() {return cmpManager.getComponentVector_t<Component_t>(); }

    // Create a component for an entity
    template<typename Component_t, typename... Args>
    Component_t& createComponent(EntityID entID, Args&&... args) {return cmpManager.createComponent_t<Component_t>(entID, std::forward<Args>(args)...); }
private:
    void killEntity(EntityID entID);

    static const int reservedEntities { 100 };
    std::vector<Entity> entities;

    ComponentManager cmpManager{reservedEntities};
};
