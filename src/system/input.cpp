#include "input.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/weaponCmpFunc.hpp"
#include "../components/inventoryCmpFunc.hpp"
#include "../util/raidTimes.hpp"

// Constructor
InputSystem::InputSystem(GameContext& gc)
    : gctx(gc)
{
}

// Destructor
InputSystem::~InputSystem()
{
}

// Update One
void InputSystem::updateOne(InputComponent& cmp) const noexcept
{
    // Sacar la camara de la entidad
	NodeComponent *nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
	CollisionComponent *collComp = gctx.entityManager.getComponentByID<CollisionComponent>(cmp.getEntityID());
	WeaponComponent *weaponCmp = gctx.entityManager.getComponentByID<WeaponComponent>(cmp.getEntityID());
	TypeComponent* typeCmp = nullptr; // Will get it later if needed;

	GraphicsEngine& graphicsEngine = gctx.graphicsEngine;

	if(weaponCmp)
	{
		// Get if its firing
		auto& weapon = weaponCmp->weapons[weaponCmp->index];
		weapon.prevIsFiring = weapon.isFiring;
		weapon.isFiring =
			gctx.graphicsEngine.getClickBehaviour(
				Key::LMOUSE,
				weapon.isSemiauto
			);
		
		// Send online
		if(weapon.prevIsFiring != weapon.isFiring)
		{
			if(!typeCmp) typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
			gctx.networkClient.send(
				"PlayerIsShooting" SEP
				+std::to_string(typeCmp->idInServer)
				+SEP+std::to_string(weapon.isFiring)
			);
		}
	}
	
	if(nodeCmp && cmp.firstUpdate)
    {
        // Reposition mouse on center in the first update
		graphicsEngine.repositionMouseCenter();
		cmp.firstUpdate = false;
    }

    Vector3f pos = collComp->previousPosition;
    Vector3f target = nodeCmp->node->getTarget() - pos;
	Vector3f oriTarget = target;	//for camera and gun position purposes
	
	// Rotation
	Vector3f relativeRotation = graphicsEngine.calculateCameraRotation(
		target.getHorizontalAngle(),
		cmp.rotateSpeed,
		cmp.maxVerticalAngle
	);
	
	graphicsEngine.mouseOutsideForcedReposition();

	// Function for the engine, set Target
	target.set(.0f,.0f,.5f);
	Vector3f movedir = target;
	
	// Get movedir
	graphicsEngine.calculateCameraMovementDirection(
		cmp.noVerticalMovement,
		relativeRotation,
		movedir,
		target
	);

    Vector3f speed {0,0,0};
	Vector3f strafevect = movedir;
	strafevect = strafevect.crossProduct(Vector3f(0.0f, 1.0f, 0.0f));
	strafevect.normalize();

	// Movement input	
	if (graphicsEngine.getKeyContinuousPress(Key::W)) 
		speed = movedir;
	else if (graphicsEngine.getKeyContinuousPress( Key::S)) 
		speed = movedir * -0.6;
	
	if (graphicsEngine.getKeyContinuousPress( Key::A)) 
		speed += strafevect * -0.9;
	else if (graphicsEngine.getKeyContinuousPress( Key::D)) 
		speed += strafevect * 0.9;
	speed.normalize();
	speed *= collComp->baseAcceleration.x;

	// Dash input	
	if (graphicsEngine.getKeyContinuousPress(Key::Q) || graphicsEngine.getKeyContinuousPress(Key::LSHIFT) )
		if(collComp->dashCounter <= 0.0f && collComp->dashCooldown <= 0.0f)
		{
			// Play sound
			if(!typeCmp) typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
			gctx.audioManager.playSound(SOUND_SFX_DASH, SOUNDS_SFX_COMBAT);
			gctx.networkClient.send("PlayerDashed" SEP+std::to_string(typeCmp->idInServer));

			collComp->dashCounter = collComp->dashTime;
			if(std::abs(speed.x)<=UMBRAL_SPEED && std::abs(speed.z)<=UMBRAL_SPEED)
			{
				collComp->speed = movedir * collComp->baseAcceleration.x;
			}

			collComp->storedSpeed = collComp->speed;

			if(gctx.gameManager.isRaidZone)
				gctx.gameManager.timeWithoutRaid+=DASH;
		}

	// Weapon change input
	if(graphicsEngine.getKeySinglePress(Key::NUM_1))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 1);
	if(graphicsEngine.getKeySinglePress(Key::NUM_2))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 2);
	if(graphicsEngine.getKeySinglePress(Key::NUM_3))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 3);
	if(graphicsEngine.getKeySinglePress(Key::NUM_4))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 4);
	if(graphicsEngine.getKeySinglePress(Key::NUM_5))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 5);
	if(graphicsEngine.getKeySinglePress(Key::NUM_6))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 6);
	if(graphicsEngine.getKeySinglePress(Key::NUM_7))
		cf::equipWeapon(*weaponCmp, gctx, (WeaponID) 7);

	cf::cycleWeapon(*weaponCmp, gctx, graphicsEngine.getMouseWheelStatus());
	/*End Weapon Selector*/

	// Jump input
	if (graphicsEngine.getKeyContinuousPress( Key::SPACEBAR )) {
		Vector3f dir=Vector3f(0,-1,0);
		Vector3f size=collComp->size;
		Vector3f corner1=nodeCmp->node->getPosition();
			corner1.x+=size.x/2;
			corner1.z+=size.z/2;
		Vector3f corner2=corner1;
			corner2.x-=size.x;
		Vector3f corner3=corner2;
			corner3.z-=size.z;
		Vector3f corner4=corner3;
			corner4.x+=size.x;
		
		float distance= collComp->size.y + 0.9f;
		if(graphicsEngine.checkRayCastCollision(nodeCmp->node->getPosition(),dir, distance, MASK MASK_MAP) || 
			graphicsEngine.checkRayCastCollision(corner1,dir, distance, MASK MASK_MAP) || 
			graphicsEngine.checkRayCastCollision(corner2,dir, distance, MASK MASK_MAP) || 
			graphicsEngine.checkRayCastCollision(corner3,dir, distance, MASK MASK_MAP) || 
			graphicsEngine.checkRayCastCollision(corner4,dir, distance, MASK MASK_MAP) 
			)
		{
			if (collComp->fallingSpeed.y <= 0) 
			{
				auto* controller { nodeCmp->node->getCharacterController() };
				controller->applyImpulse({0, controller->getJumpSpeed(), 0});
			}
		}
	}

	//Interactuable input
	int eventEntityID = graphicsEngine.checkRayCastCollisionWithNode(nodeCmp->node->getPosition(),target.normalize(),5,MASK MASK_INTERACTABLE);
	gctx.hudManager.setInteractableInfo( eventEntityID );
	if (eventEntityID > 0 && (graphicsEngine.getKeySinglePress( Key::F ) || graphicsEngine.getKeySinglePress(Key::E)) )
	{
		InteractableComponent* otherInteractable = gctx.entityManager.getComponentByID<InteractableComponent>(eventEntityID);
		if(otherInteractable)
		{
			otherInteractable->interaction(*otherInteractable, cmp.getEntityID(), gctx); 
			gctx.hudManager.setCrosshairEffect("crosshairInteraction", "crosshairInterac" );
		}
	}

	// Modify speed based on input
	collComp->speed.x = speed.x;
	collComp->speed.z = speed.z;

	// Write right target
	// Target needs to have the calculated pos, when target becomes a parameter of a component, this will change
	target += pos;
	nodeCmp->node->setTarget(target);
	graphicsEngine.softRepositionMouseCenter();
	nodeCmp->node->setRotation(oriTarget.normalize().getHorizontalAngle());	

    //checkDebugInputs(cmp);
}

// Update All
void InputSystem::updateAll() const noexcept
{
    gctx.graphicsEngine.updateMouseControllerVariables();
	
    auto& components = gctx.entityManager.getComponentVector<InputComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
	    
	gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void InputSystem::checkDebugInputs(InputComponent& cmp) const noexcept
{
	// DEBUG INPUTS
	// Force a Raid
	GraphicsEngine& graphicsEngine { gctx.graphicsEngine };
	if(graphicsEngine.getKeySinglePress(Key::L))
	{
		gctx.gameManager.timeWithoutRaid += 10000;
        gctx.hudManager.addPopup("bottomSlot", "cheat", "Cheat: Raid accelerated", "popupBackground", "icon_skull", 1.f);
	}
	// Win free credits
	else if(graphicsEngine.getKeySinglePress(Key::P))
	{
		auto* inventory = gctx.entityManager.getComponentByID<InventoryComponent>(cmp.getEntityID());
		cf::addCredits(*inventory, 1000, gctx);
        gctx.hudManager.addPopup("bottomSlot", "cheat", "Cheat: Won 1000 credits!", "popupBackground", "icon_credits", 1.f);
	}
	//	Get keys from 1 to 10
	else if (graphicsEngine.getKeySinglePress(Key::K))
	{
		auto* inventory = gctx.entityManager.getComponentByID<InventoryComponent>(cmp.getEntityID());
		for(int i=1; i<=10; i++)
			cf::addKey(*inventory, i, gctx, false);
        gctx.hudManager.addPopup("bottomSlot", "cheat", "Cheat: Got keys 1 to 10", "popupBackground", "icon_card1", 1.f);
	}
	// Win free kills to open doors
	else if(graphicsEngine.getKeySinglePress(Key::J))
	{
		gctx.gameManager.killedMapEnemies += 100;
		gctx.hudManager.forceInteractableInfoRecalculation();
		// Send online
		gctx.networkClient.send(
			"EnemiesKilledCountChanged" SEP
			+std::to_string(gctx.gameManager.killedMapEnemies)
		);
        gctx.hudManager.addPopup("bottomSlot", "cheat", "Cheat: Added 100 kills!", "popupBackground", "icon_skull", 1.f);
	}
	//	Make this entity (player) die
	else if (graphicsEngine.getKeySinglePress(Key::U)) 
	{
		HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID());
		if(health) cf::kill(*health, gctx);
	}
	//	Beat the level (not really, but you go to next level)
	else if (graphicsEngine.getKeySinglePress(Key::I))
	{
		gctx.levelManager.flagLevelFinished = true;
	}
	//	All weapons
	else if (graphicsEngine.getKeySinglePress(Key::O))
	{
		WeaponComponent *weaponCmp = gctx.entityManager.getComponentByID<WeaponComponent>(cmp.getEntityID());
		cf::addWeapon(*weaponCmp,W_HANDGUN, &gctx);
		cf::addWeapon(*weaponCmp,W_SHOTGUN, &gctx);
		cf::addWeapon(*weaponCmp,W_SNIPER, &gctx);
		cf::addWeapon(*weaponCmp,W_CARBINE, &gctx);
		cf::addWeapon(*weaponCmp,W_MACHINEGUN, &gctx);
		cf::addWeapon(*weaponCmp,W_SPECIAL, &gctx);
		cf::addWeapon(*weaponCmp,W_EXPLOSIVE, &gctx);
        gctx.hudManager.showWeaponBar(*weaponCmp);
        gctx.hudManager.addPopup("bottomSlot", "cheat", "Cheat: Obtained all weapons", "popupBackground", "icon_skull", 1.f);
	}
	//  Adjust volume
	else if(graphicsEngine.getKeySinglePress(Key::T))
	{
		HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID());
		if(health) cf::doDamage(*health, gctx, 1);
	}
	else if(graphicsEngine.getKeySinglePress(Key::Y))
	{
		HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID());
		if(health) cf::doDamage(*health, gctx, -1);
	}
	else if(graphicsEngine.getKeySinglePress(Key::G))
		gctx.graphicsEngine.enablePhysicsDebugDraw(false);
	else if(graphicsEngine.getKeySinglePress(Key::H))
		gctx.graphicsEngine.enablePhysicsDebugDraw(true);
}
