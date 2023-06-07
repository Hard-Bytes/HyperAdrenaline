#pragma once
#include "../graphicsEngine.hpp"
#include <hyperengine/engine/hyperengine.hpp>
#include <map>
#include <list>

class HyperEngine : public GraphicsEngine
{
public:
    // Constructor
    HyperEngine();
    ~HyperEngine();

    // Device functions
    int createDevice(const std::string& appName, const std::string& appVersion) final;
    void dropMainDevice() final;
    void closeMainDevice() final;

    void beginRender(bool clearBackBuffer = true) final;
    void endRender() final;
    void renderAll() final;

    void freeResource(const std::string& name) final;
    void freeAllResources() final;
    void updatePhysics() final;

    void setFieldOfView(float newFov) final;
    void setMaxRenderDistance(float newDist) final;
    void setScreenResolution(float newx, float newy) final;

    // Device queries
    bool isDeviceRunning() final;

    // Utils
    void hideCursor(bool isHidden) final;
    int getFPSCount() const noexcept final;

    // Map creation
    void createMap(
        std::string filePath
    ) final;

    // Node creation
    GraphicNode* createCameraNode(Vector3f pos, Vector3f initLookAt={0.5f,0,0.5f}) final;
    GraphicNode* createNode(
        Vector3f pos,
        Vector3f scale,
        Vector3f color,
        bool isVisible = true
    ) final;

    GraphicNode* createNode(
        Vector3f pos,
        Vector3f scale,
        std::string meshFilePath,
        bool isVisible = true
    ) final;
    GraphicNode* createAnimatedNode(
        Vector3f pos,
        Vector3f scale,
        std::string meshFilePath,
        float timeBetweenFrames,
        bool isVisible = true
    ) final;
    void createSkybox() final;

    int createLightNode(
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
    ) final;
    void setLightVisible(int lightID, bool active) final;

    void* createParticleGenerator(void*) final;

    void* getActiveCameraHyperengine(void) final;

    // Node clearing
    void clearScene() final;

    // Collision creation
    void createNodeCollisions(
        GraphicNode* node,
        int mask,
        bool isSolidToEntities = false,
        bool isAreaEffect = false,
        bool isEventTrigger = false,
        CollisionArea collisionData = CollisionArea(),
        bool useComplexSelector = false
    ) final;
    void createKinematicController(
        GraphicNode* node
        ,int collisionFilterMask
        ,float capsuleRadius
        ,float capsuleHeight
        ,float jumpHeight
        ,float stepHeight
        ,float gravityForce
        ,Vector3f gravityDirection = {0,-1,0}
    ) final;
    void moveNodeCollisionsToSecondarySlot(GraphicNode* node) final;

    // Collision calculation
    bool checkAABBCollision(GraphicNode* node1, GraphicNode* node2) final;
    bool checkIsCollidingWithMap(GraphicNode* node) final;
    int checkEventTriggerCollision(GraphicNode* node) final;
    int checkAreaEffectCollision(GraphicNode* node) final;
    
    bool checkRayCastCollision(Vector3f origin,Vector3f direction,float distance, int mask=0, bool draw=false) final;
    int checkRayCastCollisionWithNode(Vector3f origin,Vector3f direction,float distance, int mask=0, bool draw=false) final;
    int checkRayCastCollisionWithNodeAndResult(Vector3f origin,Vector3f direction,float distance,Vector3f& hitpoint, int mask=0, bool draw=false) final;

    void enablePhysicsDebugDraw(bool enable) final;

    // Visual debug
    void createVisualDebugBullet(Vector3f start, Vector3f end, Vector3f color={1,0,0}, float radius=0.08f, float timeAliveMs=200) final;

    // Animations
    void unregisterFromAnimationList(GraphicNode* node) final;
    void updateAllAnimations() final;

    // Disable collisions
    void disableAllCollisionsOf(GraphicNode*) final;
    void disableMapCollisionOf(GraphicNode*) final;
    void disableEventTriggerCollisionOf(GraphicNode*) final;
    void disableAreaEffectCollisionOf(GraphicNode*) final;

    //Mouse Control
    void updateMouseControllerVariables() final;
    void softRepositionMouseCenter() final;
    void repositionMouseCenter() final;
    Vector3f calculateCameraRotation(Vector3f p_relativerotation, float p_rspeed, float p_maxvertangle) final;
    void mouseOutsideForcedReposition() final;
    int getMouseWheelStatus() final;

    //Camera
    void calculateCameraMovementDirection(bool p_noverticalmove, Vector3f p_relativerotation, Vector3f& p_movedir, Vector3f& p_target) final;

    // Timer creation
    float getTimeStampInSeconds() final;
    float getTime() final;
    float getLastTime() final;
    float getTimeDiff() final;
    float getTimeDiffInSeconds() final;
    void resetTimeDIff() final;
    void updateTime() final;

    // Event receiver facade
    bool getKeyContinuousPress(Key key) final;    
    bool getKeySinglePress(Key key) final;
    bool getKeyKeyboardPress(Key key) final;
    bool getClickBehaviour(Key key, bool behaviour) final; //  True es single, false es continuo
    inline void updateKeyReleaseTrigger() final { 
        hyperengine->resetKeyStates();
        hyperengine->resetMouseKeyStates();
        hyperengine->resetMouseWheelStatus();
    };

    // HUD
    void draw2DString(const std::string& text, Vector2f position, Vector3f color, int fontSize, std::string fontToUse = "audiowide") final;
    void draw2DImage(const std::string& filepath, Vector2f position, Vector2f dimensions, Vector3f colorTint, float opacity) final;
    void draw2Drectangle(const Vector2f& position,const Vector2f& size, Vector3f color, float opacity) final;
    void draw2DButton(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f dimensions, const std::string& text, const std::string& filepath, Vector3f colorTint) final;
    void draw2DCheckbox(bool& dataContainer, const std::string& internalLabel, Vector2f position, Vector2f size) final;
    void draw2DSliderInt(int& dataContainer, const std::string& internalLabel, int minval, int maxval, Vector2f position, Vector2f size, std::string label, std::string format, int fontSize, std::string fontToUse) final;
    void draw2DSliderFloat(float& dataContainer, const std::string& internalLabel, float minval, float maxval, Vector2f position, Vector2f size, std::string label, std::string format, int fontSize, std::string fontToUse) final;

    void loadFont(const std::string& fontName, int fontSize) final;
    int  getTextSize(const std::string& text, const std::string& fontName) final;

    // Screen data
    Vector2f getScreenResolution() final;
private:
    // Main device
    hyen::HyperEngine* hyperengine {nullptr};
    bool m_hasActiveCamera { false };

    // Device data
    Vector2f m_resolution;
    float m_fov { 69.0f };
    float m_maxRenderDistance { 300 };

    // Collision data
    using CollisionNodeList = std::list<hyen::Node*>;
    CollisionNodeList m_collisionNodes;
    CollisionNodeList m_eventTriggerNodes;
    CollisionNodeList m_areaOfEffectNodes;

    // Animation data
    using AnimatedModeList = std::list<hyen::EAnimatedModel*>;
    AnimatedModeList m_animatedModels;

    // Fonts
    std::map<std::string, ImFont*> m_fontMap;

    // Mouse control
    Vector2f m_mousePosition { 0.5f, 0.5f };
    Vector2f m_mouseCenter { 0.5f, 0.5f };

    // Values
    static inline double degtorad { 3.14159265359f / 180.0f };

    // Helpers
    inline void updateScreenResolution();
    inline void unregisterAnimatedModel(hyen::Node* node);
    inline void disableCollisionInList(hyen::Node* node, CollisionNodeList& list);
    inline void openGUIWindow(const std::string& name) const;
    inline void setRotationDegrees( glm::mat4& mat, glm::vec3 rotation) const;
    inline void transformVect( glm::mat4& mat, glm::vec3& vect) const;
};
