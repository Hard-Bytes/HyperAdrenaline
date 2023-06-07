#include "hyperEngine.hpp"
#include "hyperGraphicNode.hpp"
#include <hyperengine/resources/r_shader.hpp>
#include "../../util/macros.hpp"
#include "../../util/paths.hpp"
#include <hyperengine/util/functions.hpp>

// Constructor
HyperEngine::HyperEngine()
: GraphicsEngine()
{
}

// Destructor
HyperEngine::~HyperEngine()
{
    // Deleting a hyperengine safely closes the scene
    delete hyperengine;

    m_collisionNodes.clear();
    m_eventTriggerNodes.clear();
    m_areaOfEffectNodes.clear();
    m_animatedModels.clear();

    // Delete fonts
    // Apparently they don't need to be deleted, just clear the map
    m_fontMap.clear();
}

/*Create Device*/
// Creates an Irrlicht device and gets a fast access to its video driver and scene manager
// Returns -1 if device creation failed or 0 if everything went ok
int HyperEngine::createDevice(const std::string& appName, const std::string& appVersion)
{
    // If device already exists, return error;
    if(hyperengine) return -2;

    // Create device
    hyperengine = new hyen::HyperEngine(true);
    if(!this->hyperengine) return -1;

    // Set window header
    std::string windowTitle = appName+" - "+appVersion;
    hyperengine->setWindowTitle(windowTitle);
    hyperengine->setWindowIcon(ASSET_MENU_FAVICON);

    hyperengine->setWindowClearColor(0.39f,0.39f,0.39f,1);

    // Save resolution
    updateScreenResolution();

    return 0;
}

/*Drop Device*/
// Drops the main device, thus deleting and freeing memory of everything inside and the device itself
void HyperEngine::dropMainDevice()
{
    hyperengine->setWindowActive(false);
}

/*Close Device*/
// Close the main device, safely close the main device, is better than the rought drop, oh drop you are up here
void HyperEngine::closeMainDevice()
{
    hyperengine->setWindowActive(false);
}

/*Begin Render*/
// Begins the render
void HyperEngine::beginRender(bool clearBackBuffer)
{
    hyperengine->beginRender();
    this->openGUIWindow("Ventana temporal");
}

/*End Render*/
// Presents the rendered image to the screen
void HyperEngine::endRender()
{
    hyen::gui::End();
    hyen::gui::PopStyleVar(2);
    hyperengine->endRender();
    updateScreenResolution();
}

/*Render All*/
// Render all renderable nodes
void HyperEngine::renderAll()
{
    hyperengine->drawScene();
}

/*Free Resource*/
// Frees the specified resource, stored in the graphic engine
void HyperEngine::freeResource(const std::string& name)
{
    hyen::ResourceManager::freeResource(name);
}

/*Free All Resources*/
// Frees all resources stored in the graphic engine
void HyperEngine::freeAllResources()
{
    hyen::ResourceManager::freeAllResources();
}

/*Update Physics*/
// Updates the physics of the world
void HyperEngine::updatePhysics()
{
    hyperengine->updatePhysics(this->getTimeDiffInSeconds());
}

/*Set Field of View (FOV)*/
// Sets the field of view angle (in degrees)
void HyperEngine::setFieldOfView(float newFov)
{
    this->m_fov = newFov;
    if(!m_hasActiveCamera) return;
    auto* cam { hyperengine->getActiveCamera()->getEntityAsCamera() };
    if(cam) cam->setPerspective(m_fov, m_resolution.x/m_resolution.y, 0.1f, m_maxRenderDistance);
}

/*Set Max Render Distance*/
// Sets the max render distance
void HyperEngine::setMaxRenderDistance(float newDist)
{
    this->m_maxRenderDistance = newDist;
    if(!m_hasActiveCamera) return;
    auto* cam { hyperengine->getActiveCamera()->getEntityAsCamera() };
    if(cam) cam->setPerspective(m_fov, m_resolution.x/m_resolution.y, 0.1f, m_maxRenderDistance);
}

/*Set Screen Resolution*/
// Changes the screen resolution and the aspect ratio acordingly
void HyperEngine::setScreenResolution(float newx, float newy)
{
    this->m_resolution = { newx, newy };
    if(!m_hasActiveCamera) return;
    auto* cam { hyperengine->getActiveCamera()->getEntityAsCamera() };
    if(cam) 
    {
        hyperengine->setWindowSize((int)newx, (int)newy);
        cam->setPerspective(m_fov, m_resolution.x/m_resolution.y, 0.1f, m_maxRenderDistance);
    }
}

/*Is Device Running*/
// Returns if the device is still running
bool HyperEngine::isDeviceRunning()
{
    return hyperengine->isWindowActive();
}

/*Hide Cursor*/
// Tells the device to hide the mouse cursor and locks it to the window
void HyperEngine::hideCursor(bool isHidden)
{
    hyperengine->setCursorVisibility(!isHidden);
}

/*Show FPS Count*/
// Return the framerate at which the engine is going
int HyperEngine::getFPSCount() const noexcept
{
    // TODO::
    return 0;
}

/*Create Map*/
// Clears the scene and then create the mesh of the map
void HyperEngine::createMap(std::string filePath)
{
    // Delete previous nodes
    this->clearScene();

    // Add map node to scene
    auto* node { this->createNode(
        Vector3f()
        ,Vector3f(1,1,1)
        ,filePath
    ) };
    this->createNodeCollisions(
        node
        ,MASK MASK_MAP
        ,true, false, false
        , CollisionArea()
        , true
    );
    delete node;

    // Create skybox
    this->createSkybox();
}

/*Create Camera Node*/
// Creates a camera and returns its node
GraphicNode* HyperEngine::createCameraNode(Vector3f pos, Vector3f initLookAt)
{
    // Create camera
    hyen::Node* node = hyperengine->createCamera(
        nullptr
        , pos.makeGLMVector()
    );
    m_hasActiveCamera = true;
    auto* cam { node->getEntityAsCamera() };
    cam->setPerspective(m_fov, m_resolution.x/m_resolution.y, 0.1f, m_maxRenderDistance);

    // Rotate camera
    auto* facadenode = new HyperGraphicNode(node);
    facadenode->setTarget(pos+initLookAt);

    return facadenode;
}

/*Create Node*/
// Creates a node in the scene and returns the node
GraphicNode* HyperEngine::createNode(
    Vector3f pos,
    Vector3f scale,
    Vector3f color,
    bool isVisible)
{
    auto node = this->createNode(
        pos
        ,scale
        ,MODEL_DEFAULT_CUBE
        ,isVisible
    );

    return node;
}


/*Create Node (Mesh)*/
// Creates a node in the scene and returns the node
GraphicNode* HyperEngine::createNode(
    Vector3f pos,
    Vector3f scale,
    std::string meshFilePath,
    bool isVisible)
{
    // Create node with specified mesh
    hyen::Node* node = hyperengine->createModel(
        nullptr
        , pos.makeGLMVector()
        , {0,0,0}
        , scale.makeGLMVector()
        , meshFilePath
    );

    node->setVisible(isVisible);
    auto* facadenode { new HyperGraphicNode(node) };

    return facadenode;
}

/*Create Animated Node (Mesh)*/
// Creates an animated node in the scene and returns the node
GraphicNode* HyperEngine::createAnimatedNode(
    Vector3f pos,
    Vector3f scale,
    std::string meshFilePath,
    float timeBetweenFrames,
    bool isVisible)
{
    // If animations are not enabled
    if(!enableAnimations)
    {
        return createNode(
            pos,
            scale,
            meshFilePath + MODEL_DEFAULT_NOANIM,
            isVisible
        );
    }

    // Create node with specified mesh
    hyen::Node* node = hyperengine->createAnimatedModel(
        nullptr
        , pos.makeGLMVector()
        , {0,0,0}
        , scale.makeGLMVector()
        , meshFilePath
        , timeBetweenFrames
    );

    node->setVisible(isVisible);
    auto* facadenode { new HyperGraphicNode(node) };
    facadenode->setHasAnimation(true);

    m_animatedModels.push_back(node->getEntityAsAnimatedModel());

    return facadenode;
}

void HyperEngine::createSkybox()
{
    // TODO:: Clean previous skyboxes resources(?)
    hyperengine->deleteSkybox();

    // Choose skybox
    std::string base { ASSET_SKYBOX_BLUE_FOLDER };
    switch(m_skyboxQuality)
    {
        default:
        case 0: 
            base += ASSET_SKYBOX_LOW_FOLDER;
            break;
        case 1: 
            base += ASSET_SKYBOX_MID_FOLDER;
            break;
        case 2: 
            base += ASSET_SKYBOX_HIGH_FOLDER;
            break;
        case 3: 
            base += ASSET_SKYBOX_ULTRAHIGH_FOLDER;
            break;
    }
    
    hyperengine->createSkybox(
        default_createnode_params
        , SkyboxNamelist 
        { 
            base+ASSET_SKYBOX_TOP
        ,   base+ASSET_SKYBOX_BOT
        ,   base+ASSET_SKYBOX_LEFT
        ,   base+ASSET_SKYBOX_RIGHT
        ,   base+ASSET_SKYBOX_FRONT
        ,   base+ASSET_SKYBOX_BACK 
        }
    );
}

int HyperEngine::createLightNode(
    Vector3f pos,
    int type,
    float radius,
    Vector3f intensityAmbient,
    Vector3f intensityDiffuse,
    Vector3f intensitySpecular,
    float attenuationConstant,
    float attenuationLinear,
    float attenuationQuadratic,
    Vector3f direction,
    float innerCutoff,
    float outerCutoff)
{
    // Create params
    hyen::LightIntensity   intensityData {
        intensityAmbient.makeGLMVector(),   //.ambient {0.02,0.02,0.02}
        intensityDiffuse.makeGLMVector(),   //.diffuse {0.6,0.6,0.6}
        intensitySpecular.makeGLMVector()   //.specular {0.5,0.5,0.5}
    };
    hyen::LightAttenuation attenuationData {
        attenuationConstant,    //.constant {1.0f}
        attenuationLinear,      //.linear {0.009f}
        attenuationQuadratic    //.quadratic {0.00032f}
    };
    hyen::LightAperture    apertureData {
        innerCutoff,    //.innerCutoff {0.0f}
        outerCutoff     //.outerCutoff {0.0f}
    };
    hyen::LightDirection   directionData {
        direction.x, direction.y, direction.z // 0, 0, 0
    };

    // Create light node
    hyen::Node* node = hyperengine->createLight(
        nullptr
        , pos.makeGLMVector()
        , {0,0,0}
        , {0,0,0}
        , (hyen::LightType)type
        , intensityData
        , attenuationData
        , apertureData
        , directionData
    );
    auto* lightEntity { node->getEntityAsLight() };
    return (int)lightEntity->getID();
}

/*Set Light Visible*/
// Enables the specified light, or disables it
void HyperEngine::setLightVisible(int lightID, bool active)
{
    hyperengine->setActiveLight(lightID, active);
}

/*createParticleGenerator*/
// Creates a hyperengine generator, sucks that it has to return a void* to then cast it
void* 
HyperEngine::createParticleGenerator(void* cInfo)
{
    // ParticleGenerator::CInfo const& cInfo

    // hyen::ParticleGenerator::CInfo cInfo;
    // cInfo.maxParticles =        {100};
    // cInfo.texturePath =         {"assets/particles/flare2.png"};
    // // cInfo.texturePath =         {"assets/particles/particle.DDS"};
    // cInfo.origin =              {0,0,0};
    // cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    // // cInfo.gravity =             {0.0f, -9.81f, 0.0f};
    // cInfo.mainDir =             {0.0f,  0.0f, 0.0f};
    // cInfo.particlesPerSecond =  {100.0f};
    // cInfo.spreadFactor =        {0.5f};
    // cInfo.lifeSpan =            {1.0f};
    // cInfo.minParticleSize =     {0.15f};
    // cInfo.maxParticleSize =     {0.5f};
    // cInfo.shapeRadius =         {3.f};
    // cInfo.funcColor =           {&hyen::PGF::generateParticleColorsStandard};
    // cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};
    // cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionSoftInfluence};
    // // cInfo.funcPos =             {&hyen::PGF::generatePositionCameraTarget};
    // cInfo.funcPos =             {&hyen::PGF::generatePositionStatic};
    // cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};
    // // cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionCameraTarget};

    return (void*)this->hyperengine->createParticleGenerator( *static_cast<hyen::ParticleGenerator::CInfo*>(cInfo) );
}


void* // hyen::Node*
HyperEngine::getActiveCameraHyperengine(void)
{
    return (void*)hyperengine->getActiveCamera();
}


/*Clear Scene*/
// Clears all scene's 3D nodes
void HyperEngine::clearScene()
{
    // Delete scene nodes
    hyperengine->clearTree();
    hyperengine->deleteAllWorldPhysics();
    hyperengine->updatePhysics(this->getTimeDiffInSeconds());

    m_hasActiveCamera = false;

    m_animatedModels.clear();
    // Delete collision registers
    m_collisionNodes.clear();
    m_eventTriggerNodes.clear();
    m_areaOfEffectNodes.clear();
}

/*Create Node Collisions*/
// Create colliders for this node
void HyperEngine::createNodeCollisions(
    GraphicNode* facadenode,
    int mask,
    bool isSolidToEntities,
    bool isAreaEffect,
    bool isEventTrigger,
    CollisionArea collisionData,
    bool useComplexSelector
)
{
    auto* hypernode { static_cast<HyperGraphicNode*>(facadenode) };
    auto *node { hypernode->getInnerNode() };

    // Create hitbox
    if(useComplexSelector)
    {
        // For everything else
        hyperengine->createPhysicPropertiesTriangleMeshShape(
            node, 0
            , mask, -1
        );
    }
    else
    {
        btCollisionShape* shape { nullptr };
        auto dims { collisionData.dimensions };
        if(collisionData.shape == CollisionShape::Sphere)
            shape = new btSphereShape(collisionData.radius);
        else if(collisionData.shape == CollisionShape::Box)
            shape = new btBoxShape({dims.x, dims.y, dims.z});
        // For pickups (keys, weapons...)
        hyperengine->createPhysicPropertiesCollisionObject(
                node
            ,   shape
            , mask, -1
        );
    }

    // Register in collisions
    if(isSolidToEntities)
        m_collisionNodes.push_back(node);
    if(isAreaEffect)
        m_areaOfEffectNodes.push_back(node);
    if(isEventTrigger)
        m_eventTriggerNodes.push_back(node);
}

/*Create Kinematic Controller*/
// Creates a kinematic controller for the given node
void HyperEngine::createKinematicController(
    GraphicNode* facadenode
    ,int collisionFilterMask
    ,float capsuleRadius
    ,float capsuleHeight
    ,float jumpHeight
    ,float stepHeight
    ,float gravityForce
    , Vector3f gravityDirection)
{
    // Save other physic properties before using this controller
    this->moveNodeCollisionsToSecondarySlot(facadenode);

    // Get inner nodes
    auto *hypernode { static_cast<HyperGraphicNode*>(facadenode) };
    auto *node      { hypernode->getInnerNode() };

    // Add controller
    hyperengine->createPhysicPropertiesKinematicCharacterController(
        node
        ,capsuleRadius, capsuleHeight, jumpHeight, stepHeight
        ,collisionFilterMask
    );
    // Configure gravity
    auto* controller { node->getPhysicProperties()->charCon };
    gravityDirection = gravityDirection.normalize();
    gravityDirection *= gravityForce;
    controller->setJumpSpeed(jumpHeight);
    controller->setGravity({gravityDirection.x, gravityDirection.y, gravityDirection.z});
    // Vinculate to node
    facadenode->setCharacterController(controller);
}

void HyperEngine::moveNodeCollisionsToSecondarySlot(GraphicNode* facadenode)
{
    auto *hypernode { static_cast<HyperGraphicNode*>(facadenode) };
    auto *node      { hypernode->getInnerNode() };
    auto* phy { node->getPhysicProperties() };
    if(phy)
        hypernode->setPhysicObject(phy);
}

bool HyperEngine::checkAABBCollision(GraphicNode* node1, GraphicNode* node2)
{
    return hyperengine->getAABBCollisionBetweenNodes(
         static_cast<HyperGraphicNode*>(node1)->getInnerNode()
        ,static_cast<HyperGraphicNode*>(node2)->getInnerNode()
    );
}

/*Check Node Colliding With Map*/
// Checks if the given node is colliding with the map
bool HyperEngine::checkIsCollidingWithMap(GraphicNode* node)
{ 
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    CollisionPairResult tmp;
    for(auto &query : m_collisionNodes)
    {
        if(hyperengine->getCollisionBetweenNodes(childHyperNode, query, tmp))
        {
            return true;
        }
    }
    return false;
}

/*Check Event Trigger Collision*/
// Checks if a given node is inside some event trigger box
// It returns the ID of the collided event entity, or -1 if no event box was found
int HyperEngine::checkEventTriggerCollision(GraphicNode* node)
{
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    CollisionPairResult tmp;
    for(auto &query : m_eventTriggerNodes)
    {
        if(hyperengine->getCollisionBetweenNodes(childHyperNode, query, tmp))
        {
            return (int)query->getNameID();
        }
    }
    return -1;
}

/*Check Area Effect Collision*/
// Checks if a given node is inside some area effect box
// It returns the ID of the collided area effect entity, or -1 if no event box was found
int HyperEngine::checkAreaEffectCollision(GraphicNode* node)
{
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    CollisionPairResult tmp;
    for(auto &query : m_areaOfEffectNodes)
    {
        if(hyperengine->getCollisionBetweenNodes(childHyperNode, query, tmp))
        {
            return (int)query->getNameID();
        }
    }
    return -1;
}

/*Check Raycast Collision*/
// Checks if a ray thrown from the given position in the given
// direction collides with a node that matches the mask
bool HyperEngine::checkRayCastCollision(
    Vector3f origin,
    Vector3f direction,
    float distance,
    int mask,
    bool draw)
{
    hyen::RayResult result;
    bool hit = this->hyperengine->throwRaycast(
        hyen::util::glmVec3TobtVec3(origin.makeGLMVector())
        ,hyen::util::glmVec3TobtVec3((origin+(direction.normalize() * distance)).makeGLMVector())
        ,result
        ,-1,mask
    );
    return hit;
}

/*Check Raycast Collision (Node)*/
// Checks if a ray thrown from the given position in the given
// direction collides with a node that matches the mask
// Returns the node that collided
int HyperEngine::checkRayCastCollisionWithNode(
    Vector3f origin,
    Vector3f direction,
    float distance,
    int mask,
    bool draw)
{
    hyen::RayResult result;
    bool hit = this->hyperengine->throwRaycast(
        hyen::util::glmVec3TobtVec3(origin.makeGLMVector())
        ,hyen::util::glmVec3TobtVec3((origin+(direction.normalize() * distance)).makeGLMVector())
        ,result
        ,-1,mask
    );
    auto* node = result.node;
    if(!hit || !node) return 0;

    auto nodeId = (int)node->getNameID();
    return nodeId >= 0 ? nodeId : 0;
}

int 
HyperEngine::checkRayCastCollisionWithNodeAndResult(
    Vector3f origin
,   Vector3f direction
,   float distance
,   Vector3f& hitpoint
,   int mask
,   bool draw
)
{
    hyen::RayResult result;
    bool hit = this->hyperengine->throwRaycast(
        hyen::util::glmVec3TobtVec3(origin.makeGLMVector())
        ,hyen::util::glmVec3TobtVec3((origin+(direction.normalize() * distance)).makeGLMVector())
        , result
        ,-1,mask
    );
    auto* node = result.node;
    if(!hit || !node) return 0;

    auto& pointHit {result.hitPoint};
    hitpoint = Vector3f(pointHit.x(), pointHit.y(), pointHit.z());
    auto nodeId = (int)node->getNameID();
    return nodeId >= 0 ? nodeId : 0;
}

/*Enable Physics Debug Draw*/
// Enables or disables debug draw of physics
void HyperEngine::enablePhysicsDebugDraw(bool enable)
{
    if(enable)
        hyperengine->enableDebugDraw();
    else
        hyperengine->disableDebugDraw();
}

/*Create Visual Debug Bullet*/
// Creates a sphere with an animator to go forward and die automatically
void HyperEngine::createVisualDebugBullet(Vector3f start, Vector3f end, Vector3f color, float radius, float timeAliveMs)
{ 
    // TODO::
}

/*Unregister From Animation List*/
// Removes the node from the animated model list
void HyperEngine::unregisterFromAnimationList(GraphicNode* node)
{
    auto *innerNode { static_cast<HyperGraphicNode*>(node)->getInnerNode() };
    this->unregisterAnimatedModel(innerNode);
}

/*Update All Animations*/
// Updates all registered animated models
void HyperEngine::updateAllAnimations()
{
    auto delta { this->getTimeDiffInSeconds() };
    for(auto* anim : m_animatedModels)
        anim->updateAnimation(delta);
}

/*Disable All Collisions Of Node*/
// Removes the triangle selector of the node from any possible selectors it can be in
void HyperEngine::disableAllCollisionsOf(GraphicNode* node)
{
    auto *childHyperNode { static_cast<HyperGraphicNode*>(node) };
    auto *innerNode      { childHyperNode->getInnerNode() };
    bool secondaryHitboxIsPrimary { node->hasDuplicatedHitboxData() };
    if(innerNode->getPhysicProperties()) hyperengine->deletePhysicProperties(innerNode);
    this->disableCollisionInList(innerNode, m_collisionNodes);
    this->disableCollisionInList(innerNode, m_areaOfEffectNodes);
    this->disableCollisionInList(innerNode, m_eventTriggerNodes);

    auto* secondProp = childHyperNode->getPhysicObject();
    if(!secondaryHitboxIsPrimary && secondProp)
    {
        innerNode->setPhysicProperties(secondProp);
        hyperengine->deletePhysicProperties(innerNode);
    }
    childHyperNode->setPhysicObject(nullptr);
}

/*Disable Map Collision of Node*/
// Removes the triangle selector of the node from the map collision selectors
void HyperEngine::disableMapCollisionOf(GraphicNode* node)
{
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    this->disableCollisionInList(childHyperNode, m_collisionNodes);
}

/*Disable Event Trigger of Node*/
// Removes the triangle selector of the node from the event trigger collision selectors
void HyperEngine::disableEventTriggerCollisionOf(GraphicNode* node)
{
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    this->disableCollisionInList(childHyperNode, m_eventTriggerNodes);
}

/*Disable Area Effect of Node*/
// Removes the triangle selector of the node from the area effect collision selectors
void HyperEngine::disableAreaEffectCollisionOf(GraphicNode* node)
{
    auto *childHyperNode = static_cast<HyperGraphicNode*>(node)->getInnerNode();
    this->disableCollisionInList(childHyperNode, m_areaOfEffectNodes);
}

void HyperEngine::updateMouseControllerVariables()
{
    auto absolutePos { hyperengine->getMousePositionAbsolute() };
    auto mousePosX { absolutePos.x / this->m_resolution.x };
    auto mousePosY { absolutePos.y / this->m_resolution.y };
    m_mousePosition = { (float)mousePosX, (float)mousePosY };
}

void HyperEngine::softRepositionMouseCenter()
{
    hyperengine->setMousePositionToCenter();
    m_mouseCenter = { 0.5f, 0.5f };
}

void HyperEngine::repositionMouseCenter()
{
    hyperengine->setMousePositionToCenter();
    m_mousePosition = m_mouseCenter = { 0.5f, 0.5f };
}

Vector3f HyperEngine::calculateCameraRotation(Vector3f p_relativerotation, float p_rspeed, float p_maxvertangle)
{
    if(m_mousePosition != m_mouseCenter)
    {
        p_relativerotation.y += (0.5f - m_mousePosition.x) * p_rspeed;
        p_relativerotation.x -= (0.5f - m_mousePosition.y) * p_rspeed; // * p_mouseydir (is set to 1 in the inputCmp)

        // If you go above or below your rotation sphere
        if (p_relativerotation.x > p_maxvertangle*2 && p_relativerotation.x < 360.0f-p_maxvertangle)
        {
            p_relativerotation.x = 360.0f - p_maxvertangle;
        }
        else 
        {
            if (p_relativerotation.x > p_maxvertangle && p_relativerotation.x < 360.0f-p_maxvertangle)
            {
                p_relativerotation.x = p_maxvertangle;
            }
        }

        this->repositionMouseCenter();
    }

    return p_relativerotation;
}

void HyperEngine::mouseOutsideForcedReposition()
{
    this->repositionMouseCenter();
}

int HyperEngine::getMouseWheelStatus()
{
    auto wheelStatus = hyperengine->getMouseWheelStatus();
    return (int)wheelStatus.offsetY;
}

void HyperEngine::calculateCameraMovementDirection(bool p_noverticalmove, Vector3f p_relativerotation, Vector3f& p_movedir, Vector3f& p_target)
{
    glm::mat4 mat { 1.0f };
    glm::vec3 inner_target = p_target.makeGLMVector();
	this->setRotationDegrees(mat, glm::vec3(p_relativerotation.x, p_relativerotation.y, 0));
	this->transformVect(mat, inner_target);
    p_target.makeVector3f(inner_target);

	if (p_noverticalmove)
	{
        glm::vec3 inner_movedir = p_movedir.makeGLMVector();
		this->setRotationDegrees(mat, glm::vec3(0, p_relativerotation.y, 0));
		this->transformVect(mat, inner_movedir);
        p_movedir.makeVector3f(inner_movedir);
	}
	else
	{
		p_movedir = p_target;
	}

	p_movedir.normalize();
}

float HyperEngine::getTimeStampInSeconds()
{
    return 0.0f;
}

/*Time passed*/
//We use it to get the new time for update
float HyperEngine::getTime()
{
    return 0.0f;
}

float HyperEngine::getLastTime()
{
    return 0.0f;
}

float HyperEngine::getTimeDiff()
{
    return (1000.f/60.f);
}

float HyperEngine::getTimeDiffInSeconds()
{
    return (1.f/60.f);
}

void HyperEngine::resetTimeDIff()
{
}

void HyperEngine::updateTime()
{
}

/*Get key Continuous Press*/
// Returns true if the specified key is currently pressed (down)
bool HyperEngine::getKeyContinuousPress(Key key)
{
    return hyperengine->getKeyContinuousPress((int)key);
}

/*Get key Single Press*/
// Returns true when the key is pressed, but false afterwards even if
// the key keeps pressed and until the key is released and pressed again
bool HyperEngine::getKeySinglePress(Key key)
{
    return hyperengine->getKeySinglePress((int)key);
}

/*Get key Keyboard Press*/
// Similar to getKeySinglePress but with the keyboard effect
// (After holding a key a few seconds, it will start being like getKeyContinuousPress)
bool HyperEngine::getKeyKeyboardPress(Key key)
{
    return hyperengine->getKeyKeyboardPress((int)key);
}

/*Click behaviour*/
//Get pressing of key as singlePress if true, continousPress if false
bool HyperEngine::getClickBehaviour(Key key, bool behaviour)
{
    if(key == Key::LMOUSE)
    {
        // TODO:: Adapt for any key
        if(behaviour)
            return hyperengine->getMouseKeySinglePress((int)key);
        return hyperengine->getMouseKeyContinuousPress((int)key);
    }
    if(behaviour)
        return this->getKeySinglePress(key);

    return this->getKeyContinuousPress(key);
}

/*Draw 2D String*/
// Draws a 2D string
void HyperEngine::draw2DString(
    const std::string& text
    , Vector2f position, Vector3f color
    , int fontSize, std::string fontToUse)
{
    // Get font
    std::string fontName {fontToUse + "-"+std::to_string(fontSize)};

    // Draw text with font
    hyen::gui::SetCursorPos({position.x, position.y});
    hyen::gui::PushFont(this->m_fontMap[fontName]);
    hyen::gui::TextColored({color.x,color.y,color.z, 1}, text.c_str());
    hyen::gui::PopFont();
}

/*Draw 2D Image*/
// Draws a 2D image
void HyperEngine::draw2DImage(const std::string& filepath, Vector2f position, Vector2f dimensions, Vector3f colorTint, float opacity)
{
    // Get image
    auto* resource = hyen::ResourceManager::getResource_t<hyen::RTexture>(filepath);
    resource->initialize();
    GLuint imageId = (GLuint)resource->getProgramID();

    // Draw the image
    hyen::gui::SetCursorPos({position.x, position.y});
    hyen::gui::Image(
        (void*)(intptr_t)imageId
        ,{dimensions.x, dimensions.y}
        ,{0, 0}
        ,{1, 1}
        ,{colorTint.x, colorTint.y, colorTint.z, opacity}
    );
}

/*Draw 2D Rectangle*/
// Draws a 2D rectangle
void HyperEngine::draw2Drectangle(const Vector2f& position,const Vector2f& size, Vector3f color, float opacity)
{
    this->draw2DImage(
        ASSET_HUD_DEFAULT_RECT
        ,position
        ,size
        ,color
        ,opacity
    );
}

/*Draw 2D Button*/
// Draws a 2D button
void HyperEngine::draw2DButton(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f dimensions, const std::string& text, const std::string& filepath, Vector3f colorTint)
{
    // Get image
    auto* resource = hyen::ResourceManager::getResource_t<hyen::RTexture>(filepath);
    resource->initialize();
    GLuint imageId = (GLuint)resource->getProgramID();

    // Draw the image
    hyen::gui::SetCursorPos({position.x, position.y});
    // TODO:: Text(?)
    hyen::gui::PushID(internalLabel.c_str());
    dataContainer = hyen::gui::ImageButton(
        (void*)(intptr_t)imageId
        ,{dimensions.x, dimensions.y}
        ,{0, 0}
        ,{1, 1}
        ,0, {0,0,0,0} //frame_padding and bg_color
        ,{colorTint.x, colorTint.y, colorTint.z, 1}
    );
    hyen::gui::PopID();
}

void HyperEngine::draw2DCheckbox(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f size)
{
    // TODO:: Size
    hyen::gui::SetCursorPos({position.x, position.y});
    hyen::gui::Checkbox(("##" + internalLabel).c_str(), &dataContainer);
}

void HyperEngine::draw2DSliderInt(
    int& dataContainer
    , const std::string& internalLabel
    , int minval, int maxval
    , Vector2f position, Vector2f size
    , std::string label, std::string format
    , int fontSize, std::string fontToUse)
{
    // Get font
    std::string fontName {fontToUse + "-"+std::to_string(fontSize)};

    hyen::gui::SetCursorPos({position.x, position.y});
    hyen::gui::PushItemWidth(size.x);
    hyen::gui::PushFont(this->m_fontMap[fontName]);
    hyen::gui::SliderInt((label+"##" + internalLabel).c_str(), &dataContainer, minval, maxval, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
    hyen::gui::PopFont();
    hyen::gui::PopItemWidth();
}

void HyperEngine::draw2DSliderFloat(
    float& dataContainer
    , const std::string& internalLabel
    , float minval, float maxval
    , Vector2f position, Vector2f size
    , std::string label, std::string format
    , int fontSize, std::string fontToUse)
{
    // Get font
    std::string fontName {fontToUse + "-"+std::to_string(fontSize)};

    hyen::gui::SetCursorPos({position.x, position.y});
    hyen::gui::PushItemWidth(size.x);
    hyen::gui::PushFont(this->m_fontMap[fontName]);
    hyen::gui::SliderFloat((label+"##" + internalLabel).c_str(), &dataContainer, minval, maxval, format.c_str(), ImGuiSliderFlags_AlwaysClamp);
    hyen::gui::PopFont();
    hyen::gui::PopItemWidth();
}


/*Load Font*/
// Loads the font in the system
void HyperEngine::loadFont(const std::string& fontToAdd, int fontSize)
{
    std::string fontName {fontToAdd + "-"+std::to_string(fontSize)};
    auto it { this->m_fontMap.find(fontName) };
    if(it == this->m_fontMap.end())
    {
        auto& fonts { hyen::gui::GetIO().Fonts };
        this->m_fontMap[fontName] = fonts->AddFontFromFileTTF("assets/fonts/Audiowide-Regular.ttf", fontSize);
        fonts->Build();
        ImGui_ImplOpenGL3_CreateDeviceObjects();
    }
}

/*Get Text Size*/
// Gets the text size in pixels given the specified font
int HyperEngine::getTextSize(const std::string& text, const std::string& fontName)
{
    // TODO:::
    return 0;
}

Vector2f HyperEngine::getScreenResolution()
{
    return this->m_resolution;
}

/*Update Screen Resolution*/
// Updates the resolution variable
inline void HyperEngine::updateScreenResolution()
{
    auto winsize = hyperengine->getWindowSize();
    this->m_resolution = {(float)winsize.x, (float)winsize.y};
}

/*Unregister Animated Model*/
// Unregister the node in the animated model list
inline void HyperEngine::unregisterAnimatedModel(hyen::Node* node)
{
    auto* model { node->getEntityAsAnimatedModel() };
    auto it = std::find(m_animatedModels.begin(), m_animatedModels.end(), model);
    if(it == m_animatedModels.end()) return;
    m_animatedModels.erase(it);
}

/*Disable Collision in List*/
// Disables collision of the given list type of the given node
inline void HyperEngine::disableCollisionInList(hyen::Node* node, CollisionNodeList& list)
{
    auto it = std::find(list.begin(), list.end(), node);
    if(it == list.end()) return;
    list.erase(it);
}

/*Open GUI Window*/
// Opens a window with the specified name in the specified position
inline void HyperEngine::openGUIWindow(const std::string& name) const
{
    hyen::gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    hyen::gui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

    hyen::gui::SetNextWindowPos({0, 0});
    hyen::gui::SetNextWindowSize({this->m_resolution.x, this->m_resolution.y});
    hyen::gui::Begin(name.c_str(), 0, 
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
    );
}

/*Set Rotation Radians*/
// Helper function that does math stuff
inline void HyperEngine::setRotationDegrees( glm::mat4& mat, glm::vec3 rotation) const
{
    rotation *= this->degtorad;

    const double cr = cos( rotation.x );
    const double sr = sin( rotation.x );
    const double cp = cos( rotation.y );
    const double sp = sin( rotation.y );
    const double cy = cos( rotation.z );
    const double sy = sin( rotation.z );

    mat[0][0] = (double)( cp*cy );
    mat[0][1] = (double)( cp*sy );
    mat[0][2] = (double)( -sp );

    const double srsp = sr*sp;
    const double crsp = cr*sp;

    mat[1][0] = (double)( srsp*cy-cr*sy );
    mat[1][1] = (double)( srsp*sy+cr*cy );
    mat[1][2] = (double)( sr*cp );

    mat[2][0] = (double)( crsp*cy+sr*sy );
    mat[2][1] = (double)( crsp*sy-sr*cy );
    mat[2][2] = (double)( cr*cp );
}

inline void HyperEngine::transformVect( glm::mat4& mat, glm::vec3& vect) const
{
   float vector[3];

    vector[0] = vect.x*mat[0][0] + vect.y*mat[1][0] + vect.z*mat[2][0] + mat[3][0];
    vector[1] = vect.x*mat[0][1] + vect.y*mat[1][1] + vect.z*mat[2][1] + mat[3][1];
    vector[2] = vect.x*mat[0][2] + vect.y*mat[1][2] + vect.z*mat[2][2] + mat[3][2];

    vect.x = vector[0];
    vect.y = vector[1];
    vect.z = vector[2];
}
