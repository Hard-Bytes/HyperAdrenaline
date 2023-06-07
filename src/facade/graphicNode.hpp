#pragma once
#include <string>
#include <vector>
#include "vector3f.hpp"
#include "../pathfinding/triangle.hpp"

class GraphicNode
{
public:
    // Constructor
    explicit GraphicNode(){}
    virtual ~GraphicNode(){}

    //Naming
    virtual void setName(std::wstring input) = 0;
    virtual void setName(int input) = 0;
    virtual std::string getName() = 0;
    virtual int getNameAsNumber() = 0;

    //Position
    virtual void setPosition(Vector3f newPos) = 0;
    virtual Vector3f getPosition() = 0;
    virtual Vector3f getAbsolutePosition() = 0;

    // Rotation
    virtual void setRotation(Vector3f) = 0;
    virtual Vector3f getRotation() = 0;
    virtual void rotate(Vector3f) = 0;

    // Scale
    virtual void setScale(Vector3f) = 0;
    virtual Vector3f getScale() = 0;

    // Visual properties
    virtual void setVisible(bool) = 0;

    // Animations
    virtual void playAnimation(const std::string& anim, bool looped) = 0;
    virtual bool hasAnimation() = 0;
    virtual void setHasAnimation(bool hasAnim) = 0;

    // Camera specific
    virtual void setTarget(Vector3f p_target) = 0;
    virtual Vector3f getTarget() = 0;

    // Physics
    virtual void setPhysicOffset(Vector3f newoffset) = 0;
    virtual void setSecondaryPhysicOffset(Vector3f newoffset) noexcept = 0;
    virtual void setPhysicRotation(Vector3f rot) = 0;
    virtual void overrideCollisionPosition(Vector3f newpos) = 0;
    virtual void overrideCollisionPosition_safe(Vector3f newpos) = 0;
    virtual bool hasDuplicatedHitboxData() = 0;

    // Character Controller
    // TODO:: Facade
    virtual void setCharacterController(btKinematicCharacterController* con) = 0;
    virtual btKinematicCharacterController* getCharacterController() const = 0;

    // Navmesh building (for now at least)
    virtual std::vector<Triangle> getTriangles() = 0;

    // Facade
    virtual void destroyInnerNode() = 0;
    //virtual irr::scene::ISceneNode* getIrrNode() = 0;

    //hierarchy
    virtual void appendChild(GraphicNode * child) = 0;
protected:
    bool m_hasAnimation { false };
};
