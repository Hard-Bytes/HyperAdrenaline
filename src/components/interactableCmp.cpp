#include "interactableCmp.hpp"

InteractableComponent::InteractableComponent(
    EntityID entID,
    InteractionFunction action,
    int p_keyId,
    int p_idToInteract,
    int p_idToBeInteracted,
    float p_data
)
    : Component(entID),
    interaction(action),
    sharedKeyId(p_keyId),
    idRemoteToInteractWith(p_idToInteract),
    idRemoteToBeInteracted(p_idToBeInteracted),
    interactionData(p_data)
{}

InteractableComponent::~InteractableComponent()
{
}
