#include "componentManager.hpp"
#include "../util/macros.hpp"
#include <algorithm>

ComponentManager::ComponentManager(int reservedComponents)
    : maxComponents(reservedComponents)
{}

ComponentManager::~ComponentManager()
{
    this->deleteComponentVector_t<NodeComponent>();
    this->deleteComponentVector_t<CollisionComponent>();
    this->deleteComponentVector_t<HealthComponent>();
    this->deleteComponentVector_t<StatusComponent>();
    this->deleteComponentVector_t<InputComponent>();
    this->deleteComponentVector_t<AIComponent>();
    this->deleteComponentVector_t<InventoryComponent>();
    this->deleteComponentVector_t<WeaponComponent>();
    this->deleteComponentVector_t<InteractableComponent>();
    this->deleteComponentVector_t<TypeComponent>();
    this->deleteComponentVector_t<BehaviourComponent>();

    this->deleteComponentVector_t<ParticleComponent>();
}

/*Destroy All Components Of Entity*/
// Clear all components of a given entity
void ComponentManager::clearAllComponentsOfEntity(EntityID entID)
{
    this->clearComponentByID_t<NodeComponent>(entID);
    this->clearComponentByID_t<CollisionComponent>(entID);
    this->clearComponentByID_t<HealthComponent>(entID);
    this->clearComponentByID_t<StatusComponent>(entID);
    this->clearComponentByID_t<InputComponent>(entID);
    this->clearComponentByID_t<AIComponent>(entID);
    this->clearComponentByID_t<InventoryComponent>(entID);
    this->clearComponentByID_t<WeaponComponent>(entID);
    this->clearComponentByID_t<InteractableComponent>(entID);
    this->clearComponentByID_t<TypeComponent>(entID);
    this->clearComponentByID_t<BehaviourComponent>(entID);

    this->clearComponentByID_t<ParticleComponent>(entID);

}

/*Clear All Components*/
// Clears all components of all entities
void ComponentManager::clearAllComponents()
{
    this->getComponentVector_t<NodeComponent>().clear();
    this->getComponentVector_t<CollisionComponent>().clear();
    this->getComponentVector_t<HealthComponent>().clear();
    this->getComponentVector_t<StatusComponent>().clear();
    this->getComponentVector_t<InputComponent>().clear();
    this->getComponentVector_t<AIComponent>().clear();
    this->getComponentVector_t<InventoryComponent>().clear();
    this->getComponentVector_t<WeaponComponent>().clear();
    this->getComponentVector_t<InteractableComponent>().clear();
    this->getComponentVector_t<TypeComponent>().clear();
    this->getComponentVector_t<BehaviourComponent>().clear();

    this->getComponentVector_t<ParticleComponent>().clear();
}
