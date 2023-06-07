#pragma once
#include "keycodes.hpp"
#include "graphicNode.hpp"
#include "vector2f.hpp"
#include "vector3f.hpp"

namespace hyen{
    struct ParticleGenerator;
    // struct ParticleGenerator::CInfo;
    // struct CInfo;
}

#define MASK 1<<
// Max masks = 32 or 64, we don't know
enum CollisionMask
{
    MASK_USELESS = 0,
    MASK_PLAYER,
    MASK_ENEMY,
    MASK_MAP,
    MASK_INTERACTABLE
};

enum class CollisionShape {
    Sphere, Box
};
struct CollisionArea
{
    CollisionShape shape    {CollisionShape::Sphere};
    Vector3f dimensions     {0.5,1,0.5};
    float radius            {0.25f};
};

class GraphicsEngine
{
public:
    // Constructor
    GraphicsEngine(){}
    virtual ~GraphicsEngine(){}

    // Device functions
    virtual int createDevice(const std::string& appName, const std::string& appVersion) = 0;
    virtual void dropMainDevice() = 0;
    virtual void closeMainDevice() = 0;

    virtual void beginRender(bool clearBackBuffer = true) = 0;
    virtual void endRender() = 0;
    virtual void renderAll() = 0;

    virtual void freeResource(const std::string& name) = 0;
    virtual void freeAllResources() = 0;
    virtual void updatePhysics() = 0;

    virtual void setFieldOfView(float newFov) = 0;
    virtual void setMaxRenderDistance(float newDist) = 0;
    virtual void setScreenResolution(float newx, float newy) = 0;

    // Device queries
    virtual bool isDeviceRunning() = 0;

    // Utils
    virtual void hideCursor(bool isHidden) = 0;
    virtual int getFPSCount() const noexcept = 0;

    // Map creation
    virtual void createMap(
        std::string filePath
    ) = 0;

    // Node creation
    virtual GraphicNode* createCameraNode(Vector3f pos, Vector3f initLookAt={0.5f,0,0.5f}) = 0;
    virtual GraphicNode* createNode(
        Vector3f pos,
        Vector3f scale,
        Vector3f color,
        bool isVisible = true
    ) = 0;

    virtual GraphicNode* createNode(
        Vector3f pos,
        Vector3f scale,
        std::string meshFilePath,
        bool isVisible = true
    ) = 0;
    virtual GraphicNode* createAnimatedNode(
        Vector3f pos,
        Vector3f scale,
        std::string meshFilePath,
        float timeBetweenFrames,
        bool isVisible = true
    ) = 0;
    virtual void createSkybox() = 0;
    virtual int createLightNode(
        Vector3f pos,
        int type,
        float radius,
        Vector3f intensityAmbient,
        Vector3f intensityDiffuse,
        Vector3f intensitySpecular,
        float attenuationConstant,
        float attenuationLinear,
        float attenuationQuadratic,
        Vector3f direction = {0,0,0},
        float innerCutoff = 0,
        float outerCutoff = 0
    ) = 0;
    virtual void setLightVisible(int lightID, bool active) = 0;

    virtual void* createParticleGenerator(void*) = 0;

    virtual void* getActiveCameraHyperengine(void) = 0;

    // Node clearing
    virtual void clearScene() = 0;

    // Collision creation
    virtual void createNodeCollisions(
        GraphicNode* node,
        int mask,
        bool isSolidToEntities = false,
        bool isAreaEffect = false,
        bool isEventTrigger = false,
        CollisionArea collisionData = CollisionArea(),
        bool useComplexSelector = false
    ) = 0;
    virtual void createKinematicController(
        GraphicNode* node
        ,int collisionFilterMask
        ,float capsuleRadius
        ,float capsuleHeight
        ,float jumpHeight
        ,float stepHeight
        ,float gravityForce
        , Vector3f gravityDirection = {0,-1,0}
    ) = 0;
    virtual void moveNodeCollisionsToSecondarySlot(GraphicNode* node) = 0;

    // Collision calculation
    virtual bool checkAABBCollision(GraphicNode* node1, GraphicNode* node2) = 0;
    virtual bool checkIsCollidingWithMap(GraphicNode* node) = 0;
    virtual int checkEventTriggerCollision(GraphicNode* node) = 0;
    virtual int checkAreaEffectCollision(GraphicNode* node) = 0;

    virtual bool checkRayCastCollision(Vector3f origin,Vector3f direction,float distance, int mask=0, bool draw=false) = 0;
    virtual int checkRayCastCollisionWithNode(Vector3f origin,Vector3f direction,float distance, int mask=0, bool draw=false) = 0;
    virtual int checkRayCastCollisionWithNodeAndResult(Vector3f origin,Vector3f direction,float distance,Vector3f& hitpoint, int mask=0, bool draw=false) = 0;
    virtual void enablePhysicsDebugDraw(bool enable) = 0;

    // Visual debug
    virtual void createVisualDebugBullet(Vector3f start, Vector3f end, Vector3f color={1,0,0}, float radius=0.08f, float timeAliveMs=200) = 0;

    // Animations
    virtual void unregisterFromAnimationList(GraphicNode* node) = 0;
    virtual void updateAllAnimations() = 0;

    // Disable collisions
    virtual void disableAllCollisionsOf(GraphicNode*) = 0;
    virtual void disableMapCollisionOf(GraphicNode*) = 0;
    virtual void disableEventTriggerCollisionOf(GraphicNode*) = 0;
    virtual void disableAreaEffectCollisionOf(GraphicNode*) = 0;

    //Mouse Control
    virtual void updateMouseControllerVariables() = 0;
    virtual void softRepositionMouseCenter() = 0;
    virtual void repositionMouseCenter() = 0;
    virtual Vector3f calculateCameraRotation(Vector3f p_relativerotation, float p_rspeed, float p_maxvertangle) = 0;
    virtual void mouseOutsideForcedReposition() = 0;
    virtual int getMouseWheelStatus() = 0;

    //Camera
    virtual void calculateCameraMovementDirection(bool p_noverticalmove, Vector3f p_relativerotation, Vector3f& p_movedir, Vector3f& p_target) = 0;

    // Timer creation
    virtual float getTimeStampInSeconds() = 0;
    virtual float getTime() = 0;
    virtual float getLastTime() = 0;
    virtual float getTimeDiff() = 0;
    virtual float getTimeDiffInSeconds() = 0;
    virtual void resetTimeDIff() = 0;
    virtual void updateTime() = 0;

    // Event receiver facade
    virtual bool getKeyContinuousPress(Key key) = 0;    
    virtual bool getKeySinglePress(Key key) = 0;
    virtual bool getKeyKeyboardPress(Key key) = 0;
    virtual bool getClickBehaviour(Key key, bool behaviour) = 0; //  True es single, false es continuo
    // set lmouse key pressed flag as true
    virtual inline void updateKeyReleaseTrigger() = 0;

    // HUD
    virtual void draw2DString(const std::string& text, Vector2f position, Vector3f color, int fontSize, std::string fontToUse = "audiowide") = 0; // posicion color
    virtual void draw2DImage(const std::string& filepath, Vector2f position, Vector2f dimensions, Vector3f colorTint, float opacity) = 0;
    virtual void draw2Drectangle(const Vector2f& position,const Vector2f& size, Vector3f color, float opacity) = 0;
    virtual void draw2DButton(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f dimensions, const std::string& text, const std::string& filepath, Vector3f colorTint) = 0;
    virtual void draw2DCheckbox(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f size) = 0;
    virtual void draw2DSliderInt(int& dataContainer, const std::string& internalLabel, int minval, int maxval, Vector2f position, Vector2f size, std::string label, std::string format, int fontSize, std::string fontToUse) = 0;
    virtual void draw2DSliderFloat(float& dataContainer, const std::string& internalLabel, float minval, float maxval, Vector2f position, Vector2f size, std::string label, std::string format, int fontSize, std::string fontToUse) = 0;

    virtual void loadFont(const std::string& fontName, int fontSize) = 0;
    virtual int  getTextSize(const std::string& text, const std::string& fontName) = 0;

    // Screen data
    virtual Vector2f getScreenResolution() = 0;

    // Not virtual methods
    constexpr void setDynamicLightning(bool enable)  noexcept { enableDynamicLightning = enable; }
    constexpr bool getDynamicLightning()       const noexcept { return enableDynamicLightning; }
    constexpr void setAnimationsEnabled(bool enable) noexcept { enableAnimations = enable; }
    constexpr bool getAnimationsEnabled()      const noexcept { return enableAnimations; }

    constexpr void setSkyboxQuality(int newval) noexcept { m_skyboxQuality = newval; }
    constexpr int getSkyboxQuality()      const noexcept { return m_skyboxQuality; }
    constexpr void setParticleQuality(int newval) noexcept { m_particleQuality = newval; }
    constexpr int getParticleQuality()      const noexcept { return m_particleQuality; }
protected:
    bool enableDynamicLightning {false};
    bool enableAnimations {true};
    int m_skyboxQuality { 0 };
    int m_particleQuality { 0 };
};
