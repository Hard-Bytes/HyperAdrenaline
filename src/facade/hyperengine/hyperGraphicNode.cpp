#include "hyperGraphicNode.hpp"
#include "../../util/macros.hpp"
#include <hyperengine/entities/e_model.hpp>

HyperGraphicNode::~HyperGraphicNode()
{
    //node->remove();
}

/*Set Name*/
// Sets the name of the node
void HyperGraphicNode::setName(std::wstring input)
{
    node->setNameID(std::stoi(input.c_str()));
}
/*Set Name*/
// Sets the name of the node (as a number)
void HyperGraphicNode::setName(int input)
{
    node->setNameID(input);
}
/*Get Name*/
// Gets the name of the node
std::string HyperGraphicNode::getName()
{
    return std::to_string(node->getNameID());
}
/*Get Name As Number*/
// Gets the name of the node (interpreted as a number)
int HyperGraphicNode::getNameAsNumber()
{
    return (int)node->getNameID();
}

/*Set Position*/
// Sets the position of the node
void HyperGraphicNode::setPosition(Vector3f newPos)
{
    node->setTranslation(newPos.makeGLMVector());
}

/*Get Position*/
// Gets the position of the node
Vector3f HyperGraphicNode::getPosition()
{
    auto pos = Vector3f(node->getTranslation());
    return pos;
}

/*Get Absolute Position*/
// Gets the absolute position of the node
Vector3f HyperGraphicNode::getAbsolutePosition()
{
    return this->getPosition();
}

/*Set Rotation*/
// Sets the rotation of the node (in degrees)
void HyperGraphicNode::setRotation(Vector3f rot)
{
    node->setRotation(rot.makeGLMVector());
}

/*Get Rotation*/
// Gets the rotation of the node (in degrees)
Vector3f HyperGraphicNode::getRotation()
{
    auto rot = Vector3f(node->getRotation());
    return rot;
}

/*Rotate*/
// Rotate the node from its current rotation (in degrees)
void HyperGraphicNode::rotate(Vector3f rot)
{
    node->rotate(rot.makeGLMVector());
}

/*Set Scale*/
// Sets the scale factor for the graphic node
void HyperGraphicNode::setScale(Vector3f newScale)
{
    node->setScale(newScale.makeGLMVector());
}

/*Get Scale*/
// Gets the scale factor of the graphic node
Vector3f HyperGraphicNode::getScale()
{
    return Vector3f(node->getScale());
}

/*Set Visible*/
// Sets the visibility of the node (visible or invisible)
void HyperGraphicNode::setVisible(bool visible)
{
    node->setVisible(visible);
}

/*Play Animation*/
// Plays the specified animation
void HyperGraphicNode::playAnimation(const std::string& anim, bool looped)
{
    if(!m_hasAnimation) return;
    auto* model { node->getEntityAsAnimatedModel() };
    if(model) 
    {
        model->setAnimation(anim);
        model->getCurrentAnimation()->setLooped(looped);
    }
}

/*Has Animation*/
// Returns true if the model is animated
bool HyperGraphicNode::hasAnimation()
{
    return m_hasAnimation;
}

/*Set Has Animation*/
// Specifies if the node is animated (and is able to play animations)
void HyperGraphicNode::setHasAnimation(bool hasAnim)
{
    m_hasAnimation = hasAnim;
}

// Camera specific
/*Set Target*/
// Sets the position of the target if the node is a camera
void HyperGraphicNode::setTarget(Vector3f p_target)
{
    node->setCameraTarget(p_target.makeGLMVector());
}

/*Get Target*/
// Gets the position of the target if the node is a camera
Vector3f HyperGraphicNode::getTarget()
{
    return Vector3f(node->getCameraTarget());
}

/*Set Physic Offset*/
// Sets the offset for the physics properties of this node
void HyperGraphicNode::setPhysicOffset(Vector3f newoffset)
{
    node->setPhysicOffset(newoffset.makeGLMVector());
}

void HyperGraphicNode::setSecondaryPhysicOffset(Vector3f newoffset) noexcept
{
    m_secondaryOffset = newoffset;
}

/*Set Physic Rotation*/
// Rotates the internal hitbox
void HyperGraphicNode::setPhysicRotation(Vector3f rot)
{
    rot *= 3.1415 / 180; // Degrees to Radians
    auto* phy       { node->getPhysicProperties() };
    auto collObj    { phy->m_data.body };
    auto& transform { collObj->getWorldTransform() };
    transform.setRotation({rot.y,rot.x,rot.z});
}

/*Override Collision Position*/
// Overrides the internal vinculated collision object
void HyperGraphicNode::overrideCollisionPosition(Vector3f newpos)
{
    auto collObj { m_physicProperties->m_data.collObj };
    auto& world { collObj->getWorldTransform() };
    newpos += m_secondaryOffset;
    world.setOrigin({newpos.x, newpos.y, newpos.z});
}

/*Override Collision (Safe)*/
// Overrides the internal vinculated collision object
void HyperGraphicNode::overrideCollisionPosition_safe(Vector3f newpos)
{
    if(m_physicProperties) this->overrideCollisionPosition(newpos);
}

/*Has Duplicated itbox Data*/
// Returns true if the
bool HyperGraphicNode::hasDuplicatedHitboxData()
{
    return m_physicProperties == node->getPhysicProperties();
}

/*Get Triangles*/
// Returns a vector with the triangles contained in the Triangle selector of node
// We'll use it for navmesh building purposes
std::vector<Triangle> HyperGraphicNode::getTriangles()
{
    std::vector<Triangle> result {};

    // Obtain model entity (container of vertex)
    auto model { node->getEntityAsModel() };
    if(!model) return result;

    // Obtain a float list of vertex positions
    auto& vertexPos { model->getVertexPositions() };
    auto numTriangles { vertexPos.size() / 9 };
    result.reserve(numTriangles);

    // Every three floats are a vertex
    // Every three vertexes (nine floats) are a triangle
    for(unsigned int i=0; i<numTriangles; i++)
    {
        auto base { i*9 };
        result.push_back(
            Triangle(
             {vertexPos[base+0],vertexPos[base+1],vertexPos[base+2]}
            ,{vertexPos[base+3],vertexPos[base+4],vertexPos[base+5]}
            ,{vertexPos[base+6],vertexPos[base+7],vertexPos[base+8]}
            )
        );
    }
    return result;
}

/*Destroy Inner Node*/
// Destroy the node of the graphic engine
// It is recommended to destroy this Graphic Node as well
void HyperGraphicNode::destroyInnerNode()
{
    hyen::Node::deleteBranch(node);
    node = nullptr;
}

/*Append Child*/
//set THIS as the FATHER of CHILD
void HyperGraphicNode::appendChild(GraphicNode* child)
{
    HyperGraphicNode *childHyperNode = static_cast<HyperGraphicNode*>(child);
    this->node->addChild(childHyperNode->node);
}

/*Get Inner Node*/
// Gets the HyperEngine node
hyen::Node* HyperGraphicNode::getInnerNode()
{
    return node;
}
