#include "entityManager.hpp"
#include "../manager/gameManager.hpp"
#include <algorithm>


EntityManager::EntityManager()
{
    entities.reserve(reservedEntities);
}

EntityManager::~EntityManager()
{}

EntityID EntityManager::createEntity()
{
    entities.emplace_back();
    return entities.back().getID();
}

/*Update Dead Entities*/
// Removes dead entities, but does not update game data (see GameManager for that)
// This should ALWAYS be immediately preceded by GameManagers's updateDeadEntities
void EntityManager::updateDeadEntities()
{
    for(auto it = entities.begin(); it != entities.end(); ++it)
    {
        if(it->isDead)
        {
            this->killEntity(it->getID());
            --it;
        }
    }
}

/*Mark As Dead*/
// Marks an entity as dead to be deleted later
void EntityManager::markAsDead(EntityID entID)
{
    Entity* ent = getEntityByID(entID);
    if(ent) ent->isDead = true;
}

/*Clear All*/
// Clears all entities and components
void EntityManager::clearAll()
{
    cmpManager.clearAllComponents();
    entities.clear();
}

/*Get Entity By ID*/
// Returns an Entity pointer given its ID
Entity* EntityManager::getEntityByID(EntityID entID)
{
    for(unsigned int i=0; i<entities.size(); i++)
    {
        if(entities[i].getID() == entID)
            return &entities[i];
    }

    return nullptr;
}

/*Kill Entity*/
// Kills an entity, removing it from the vector and all of its components
void EntityManager::killEntity(EntityID entID)
{
    // Destroy components
    cmpManager.clearAllComponentsOfEntity(entID);
    entities.erase(
        std::find_if(entities.begin(), entities.end(),
            [&entID](Entity& tmp){return tmp.getID() == entID;}
        )
    );
}
