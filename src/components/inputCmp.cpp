#include "inputCmp.hpp"

//We initialize the data to the same data, as the example (now commented) FPS camera
InputComponent::InputComponent(EntityID entID)
    : Component(entID),
    maxVerticalAngle(88.f),
    rotateSpeed(100), 
    mouseYDirection(1), 
    firstUpdate(true),
    noVerticalMovement(true)
{
}

InputComponent::~InputComponent()
{
}
