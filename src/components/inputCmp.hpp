#pragma once
#include "component.hpp"

struct InputComponent : public Component
{
    // Constructor
    explicit InputComponent(EntityID entID);
    ~InputComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeInput; }

	// Data
	float maxVerticalAngle, rotateSpeed, mouseYDirection;

    bool firstUpdate, noVerticalMovement; // firstInput
};
