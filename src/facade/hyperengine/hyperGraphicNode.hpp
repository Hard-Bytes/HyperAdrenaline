#pragma once
#include "../graphicNode.hpp"
#include <hyperengine/tree/node.hpp>

class HyperGraphicNode : public GraphicNode
{
public:
    // Constructor
    explicit HyperGraphicNode(hyen::Node* p_node)
        : GraphicNode(), node(p_node)
    {}
    ~HyperGraphicNode();

    //Naming
    void setName(std::wstring input) final;
    void setName(int input) final;
    std::string getName() final;
    int getNameAsNumber() final;

    //Position
    void setPosition(Vector3f newPos) final;
    Vector3f getPosition() final;
    Vector3f getAbsolutePosition() final;

    // Rotation
    void setRotation(Vector3f) final;
    Vector3f getRotation() final;
    void rotate(Vector3f) final;

    // Scale
    void setScale(Vector3f) final;
    Vector3f getScale() final;

    // Visual properties
    void setVisible(bool) final;

    // Animations
    void playAnimation(const std::string& anim, bool looped) final;
    bool hasAnimation() final;
    void setHasAnimation(bool hasAnim) final;

    // Camera specific
    void setTarget(Vector3f p_target) final;
    Vector3f getTarget() final;

    // Physics
    void setPhysicOffset(Vector3f newoffset) final;
    void setSecondaryPhysicOffset(Vector3f newoffset) noexcept final;
    void setPhysicRotation(Vector3f rot) final;
    void overrideCollisionPosition(Vector3f newpos) final;
    void overrideCollisionPosition_safe(Vector3f newpos) final;
    bool hasDuplicatedHitboxData() final;

    // Character Controller
    void setCharacterController(btKinematicCharacterController* con) noexcept final
        { m_controller = con; }
    btKinematicCharacterController* getCharacterController() const final
        { return m_controller; }

    // Navmesh building (for now at least)
    std::vector<Triangle> getTriangles() final;

    // Facade
    void destroyInnerNode() final;
    hyen::Node* getInnerNode();

    //hierarchy
    void appendChild(GraphicNode * child) final;

    // Physic Properties for HyperEngine
    PhysicProperties* getPhysicObject() noexcept { return m_physicProperties; }
    void setPhysicObject(PhysicProperties* newProps) noexcept
        { m_physicProperties = newProps; }
private:
    hyen::Node* node { nullptr };
    btKinematicCharacterController* m_controller { nullptr };
    PhysicProperties* m_physicProperties { nullptr };
    Vector3f m_secondaryOffset;
};
