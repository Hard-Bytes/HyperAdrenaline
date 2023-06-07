#pragma once
#include "../facade/vector3f.hpp"
#include <string>

enum HUDElementType
{
    HUD_TEXT = 0
    ,HUD_IMAGE
    ,HUD_FILLEDRECTANGLE
    ,HUD_BUTTON
    ,HUD_CHECKBOX
    ,HUD_SLIDERINT
    ,HUD_SLIDERFLOAT
    ,HUD_LISTBASE = 10
};

enum HUDElementOrigin
{
    ORIGIN_CENTER
    ,ORIGIN_TOPLEFT
    ,ORIGIN_BOTLEFT
    ,ORIGIN_TOPRIGHT
    ,ORIGIN_BOTRIGHT
    ,ORIGIN_TOPCENTERED
    ,ORIGIN_BOTCENTERED
    ,ORIGIN_LEFTCENTERED
    ,ORIGIN_RIGHTCENTERED
};

struct HUDElement
{
    // Constructor and virtual destructor
    explicit HUDElement(const std::string& name, Vector2f screenRes, Vector2f pos, HUDElementOrigin origin, Vector2f size)
        : m_user_position(pos)
        , m_user_size(size)
        , m_user_origin(origin)
        , m_user_scale(1.f)
        , m_name(name)
    {
        this->recalculateAbsoluteData(screenRes);
    }
    virtual ~HUDElement() = default;

    // Size calculator
    virtual void recalculateAbsoluteData(Vector2f screenResolution);

    constexpr HUDElementType getElementType() const noexcept
        { return m_hudElementType; }
    constexpr const std::string& getName() const noexcept
        { return m_name; }

    Vector2f getAbsolutePosition() const noexcept
        { return m_final_topleftPosition; }
    Vector2f getAbsoluteSize() const noexcept
        { return m_final_absoluteSize; }

    constexpr bool getIsVisible() const noexcept
        { return m_isVisible; }
    constexpr void setIsVisible(bool newVisibility) noexcept
        { m_isVisible = newVisibility; }

    // Returns the position in range [0-1] (0 is top/left of screen, 1 is bottom/right)
    Vector2f getRelativePosition() const noexcept
        { return m_user_position; }
    // Sets the position in range [0-1] (0 is top/left of screen, 1 is bottom/right)
    void setRelativePosition(Vector2f newPos) noexcept
        { m_user_position = newPos; recalculateAbsoluteData(m_screenResolution); }
    // Returns the size in range [0-1] (0 is 0% of screen resolution, 1 is 100%)
    Vector2f getRelativeSize() const noexcept
        { return m_user_size; }
    // Sets the size in range [0-1] (0 is 0% of screen resolution, 1 is 100%)
    void setRelativeSize(Vector2f newSize) noexcept
        { m_user_size = newSize; recalculateAbsoluteData(m_screenResolution); }
    // Returns what the specified position means (if its the top left corner, the center, etc...)
    constexpr HUDElementOrigin getElementOrigin() const noexcept
        { return m_user_origin; }
    // Sets what the specified position means (if its the top left corner, the center, etc...)
    void setElementOrigin(HUDElementOrigin newOrigin) noexcept
        { m_user_origin = newOrigin; recalculateAbsoluteData(m_screenResolution); }
    // Returns what the specified position means (if its the top left corner, the center, etc...)
    constexpr float getElementScale() const noexcept
        { return m_user_scale; }
    // Sets what the specified position means (if its the top left corner, the center, etc...)
    void setElementScale(float newScale) noexcept
        { m_user_scale = newScale; recalculateAbsoluteData(m_screenResolution); }

protected:
    HUDElementType m_hudElementType;
    Vector2f m_screenResolution;
private:
    // Raw data, used for recalculating
    // Both position and size are floats in [0-1]
    Vector2f m_user_position;
    Vector2f m_user_size;
    HUDElementOrigin m_user_origin;
    float m_user_scale;
    std::string m_name;
    bool m_isVisible { true };

    // Calculated data, used for drawing
    // Both position and size are in pixels
    Vector2f m_final_topleftPosition;
    Vector2f m_final_absoluteSize;
};

/*HUD Element Text*/
// HUD Element that shows a text in screen
struct HUDElementText : public HUDElement
{
    explicit HUDElementText(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , float p_fontSize
        , const std::string& p_text
        , const std::string& p_fontName
        , Vector3f p_color
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, {0,0})
        , m_text(p_text)
        , m_font(p_fontName)
        , m_color(p_color)
        , m_fontSize(p_fontSize)
        , m_user_fontSize(p_fontSize)
    {
        m_hudElementType = HUD_TEXT;
        this->recalculateAbsoluteData(p_screenRes);
    }

    void recalculateAbsoluteData(Vector2f screenResolution) final;

    const std::string& getText() const noexcept
        { return m_text; }
    void setText(const std::string& newText) noexcept
        { m_text = newText; this->recalculateAbsoluteData(this->m_screenResolution); }
    Vector3f getColor() const noexcept
        { return m_color; }
    void setColor(const Vector3f& newColor) noexcept
        { m_color = newColor; }
    constexpr float getFontSize() const noexcept
        { return m_fontSize; }
    void setFontSize(int newSize) noexcept
        { m_user_fontSize = newSize; this->recalculateAbsoluteData(this->m_screenResolution); }
    const std::string& getFont() const noexcept
        { return m_font; }
private:
    std::string m_text;
    std::string m_font;
    Vector3f m_color;
    float m_fontSize;

    float m_user_fontSize;
};

/*HUD Element Image*/
// HUD Element that shows an image in screen
struct HUDElementImage : public HUDElement
{
    explicit HUDElementImage(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
        , const std::string& p_filepath
        , const Vector3f& p_colorTint
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
        , m_filepath(p_filepath)
        , m_colorTint(p_colorTint)
    {
        m_hudElementType = HUD_IMAGE;
    }

    const std::string& getFilepath() const noexcept
        { return m_filepath; }
    void setFilepath(const std::string& newpath) noexcept
        { m_filepath = newpath; }
    const Vector3f& getColor() const noexcept
        { return m_colorTint; }
    void setColor(const Vector3f& newcolor) noexcept
        { m_colorTint = newcolor; }
private:
    std::string m_filepath;
    Vector3f m_colorTint;
};

/*HUD Element Filled Rectangle*/
// HUD Element that shows a rectangle of the specified color in screen
struct HUDElementRectangle : public HUDElement
{
    explicit HUDElementRectangle(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
        , Vector3f p_color
        , float p_opacity
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
        , m_colorTint(p_color)
        , m_opacity(p_opacity)
    {
        m_hudElementType = HUD_FILLEDRECTANGLE;
    }

    const Vector3f& getColor() const noexcept
        { return m_colorTint; }
    void setColor(const Vector3f& newcolor) noexcept
        { m_colorTint = newcolor; }
    constexpr float getOpacity() const noexcept
        { return m_opacity; }
    constexpr void setOpacity(float newop) noexcept
        { m_opacity = newop; }
private:
    Vector3f m_colorTint;
    float m_opacity;
};

/*HUD Element Button*/
// HUD Element that shows a clickable button on screen
struct HUDElementButton : public HUDElement
{
    explicit HUDElementButton(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
        , const std::string& p_filepath
        , const Vector3f& p_colorTint
        , float p_fontSize
        , const std::string& p_text
        , const std::string& p_fontName
        , Vector3f p_textcolor        
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
        , m_bgFilepath(p_filepath)
        , m_buttonColor(p_colorTint)
        , m_text(p_text)
        , m_textFont(p_fontName)
        , m_textColor(p_textcolor)
        , m_textFontSize(p_fontSize)
    {
        m_hudElementType = HUD_BUTTON;
    }

    bool& getDataContainer() noexcept
        { return m_dataContainer; }
    constexpr bool isPressed() const noexcept
        { return m_dataContainer; }

    const std::string& getBackgroundFilepath() const noexcept
        { return m_bgFilepath; }
    void setFilepath(const std::string& newpath) noexcept
        { m_bgFilepath = newpath; }
    const Vector3f& getButtonColor() const noexcept
        { return m_buttonColor; }
    void setButtonColor(const Vector3f& newcolor) noexcept
        { m_buttonColor = newcolor; }

    const std::string& getText() const noexcept
        { return m_text; }
    void setText(const std::string& newText) noexcept
        { m_text = newText; this->recalculateAbsoluteData(this->m_screenResolution); }
    Vector3f getTextColor() const noexcept
        { return m_textColor; }
    void setTextColor(const Vector3f& newColor) noexcept
        { m_textColor = newColor; }
    constexpr float getFontSize() const noexcept
        { return m_textFontSize; }
    void setFontSize(int newSize) noexcept
        { m_textFontSize = newSize; this->recalculateAbsoluteData(this->m_screenResolution); }
    const std::string& getFont() const noexcept
        { return m_textFont; }
private:
    bool m_dataContainer { false };

    std::string m_bgFilepath;
    Vector3f m_buttonColor;
    std::string m_text;
    std::string m_textFont;
    Vector3f m_textColor;
    float m_textFontSize;
};

/*HUD Element Checkbox*/
// HUD Element that shows a clickable checkbox on screen
struct HUDElementCheckbox : public HUDElement
{
    explicit HUDElementCheckbox(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
    {
        m_hudElementType = HUD_CHECKBOX;
    }

    bool& getDataContainer() noexcept
        { return m_dataContainer; }
    void setDataContainerValue(bool val) noexcept
        { m_dataContainer = val; }
private:
    bool m_dataContainer { false };
};

/*HUD Element Slider*/
// HUD Element that shows an interactable slider (for integers or floats)
struct HUDElementSlider : public HUDElement
{
    explicit HUDElementSlider(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
        , bool onlyInt
        , float p_minVal
        , float p_maxVal
        , const std::string& p_label
        , const std::string& p_format
        , const std::string& p_font
        , float p_fontsize
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
        , m_minValue(p_minVal)
        , m_maxValue(p_maxVal)
        , m_label(p_label)
        , m_format(p_format)
        , m_textFont(p_font)
        , m_fontSize(p_fontsize)
        , m_user_fontSize(p_fontsize)
    {
        if(onlyInt)
            m_hudElementType = HUD_SLIDERINT;
        else
            m_hudElementType = HUD_SLIDERFLOAT;
        this->recalculateAbsoluteData(p_screenRes);
    }

    void recalculateAbsoluteData(Vector2f screenResolution) final;

    int& getDataContainerInt() noexcept
        { return m_dataContainer_i; }
    float& getDataContainerFloat() noexcept
        { return m_dataContainer_f; }
    void setDataContainerValueInt(int val) noexcept
        { m_dataContainer_i = val; setFormatIndex(val); }
    void setDataContainerValueFloat(float val) noexcept
        { m_dataContainer_f = val; }

    float getMinValue() const noexcept
        { return m_minValue; }
    float getMaxValue() const noexcept
        { return m_maxValue; }
    const std::string& getLabel() const noexcept
        { return m_label; }
    const std::string& getInnerLabel() const noexcept
        { return m_format; }
    const std::string& getFont() const noexcept
        { return m_textFont; }
    float getFontSize() const noexcept
        { return m_fontSize; }

    void reserveFormatVector(int p_size) noexcept
        { m_formatVector.reserve(p_size); }
    void addFormat(const std::string& newformat) noexcept
        { m_formatVector.push_back(newformat); }
    void setFormatIndex(int index) noexcept
    {   
        if(m_formatVector.empty() || index < 0 || index >= (int)m_formatVector.size()) return;
        m_format = m_formatVector[index];
    }
private:
    int   m_dataContainer_i { 0 };
    float m_dataContainer_f { 0.0f };

    float m_minValue;
    float m_maxValue;

    std::string m_label;
    std::string m_format;

    std::string m_textFont;
    float m_fontSize;

    float m_user_fontSize;

    std::vector<std::string> m_formatVector;
};

/*HUD Element List Base*/
// HUD Element that is invisible and acts as a base for making lists
struct HUDElementListBase : public HUDElement
{
    explicit HUDElementListBase(
        const std::string& name
        , Vector2f p_screenRes
        , Vector2f p_pos
        , HUDElementOrigin p_origin
        , Vector2f p_size
        , Vector2f p_spacing
        , float p_fontSize
        , const std::string& p_fontName
        , Vector3f p_textcolor        
    )
        : HUDElement(name, p_screenRes, p_pos, p_origin, p_size)
        , m_spacing(p_spacing)
        , m_textFont(p_fontName)
        , m_textColor(p_textcolor)
        , m_textFontSize(p_fontSize)
    {
        m_hudElementType = HUD_LISTBASE;
    }

    Vector2f getSpacing() const noexcept
        { return m_spacing; }
    Vector3f getTextColor() const noexcept
        { return m_textColor; }
    void setTextColor(const Vector3f& newcolor) noexcept
        { m_textColor = newcolor; }
    constexpr float getFontSize() const noexcept
        { return m_textFontSize; }
    const std::string& getFont() const noexcept
        { return m_textFont; }
private:
    Vector2f m_spacing;
    std::string m_textFont;
    Vector3f m_textColor;
    float m_textFontSize;
};
