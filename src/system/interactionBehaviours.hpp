#pragma once

// Interaction Behaviours
#include "../components/allComponentsInclude.hpp"

#define declareInteractionFunction(func) void func(InteractableComponent&, int, GameContext&)

declareInteractionFunction(interactionInKillingArea);
declareInteractionFunction(interactionInDamagingArea);
declareInteractionFunction(interactionExplosionArea);
declareInteractionFunction(interactionLevelFinished);
declareInteractionFunction(interactionKamikazeSelfDestruct);
declareInteractionFunction(interactionInExplosionArea);

//Remote interaction
declareInteractionFunction(interactionOpenDoor);
declareInteractionFunction(interactionPlatformControl);
declareInteractionFunction(interactionRemoteEntityInteract);
declareInteractionFunction(interactionRemoteInteract);
declareInteractionFunction(interactionRemoteWithCardInteract);

//Pickup interaction
declareInteractionFunction(interactionHealKit);
declareInteractionFunction(interactionHealPad);
declareInteractionFunction(interactionAmmoKit);
declareInteractionFunction(interactionAmmoPad);
declareInteractionFunction(interactionReceiveCard);
declareInteractionFunction(interactionOpenDoorWithCard);
declareInteractionFunction(interactionReceiveWeapon);

//Points interaction
declareInteractionFunction(interactionVendingMachine);
declareInteractionFunction(interactionOpenDoorWithPoints);

//EnemyKillsInteraction
declareInteractionFunction(interactionOpenEnemyKillsDoor);

void helperGenericKitInteraction(InteractableComponent&, int, GameContext&);
void helperGenericPadInteraction(InteractableComponent&, GameContext&);
void sendOnlineMessage(std::string msg, InteractableComponent& interacted, int interactorID, GameContext& gctx);
