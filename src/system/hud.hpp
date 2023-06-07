#pragma once
#include "../hud/hudElementDrawing.hpp"
#include <vector>
#include <unordered_map>

typedef void (*HUDElementDrawFunc)(HUDElement*, GameContext&);
using HUDDrawMap = std::unordered_map< HUDElementType, HUDElementDrawFunc >;

class HUDSystem
{
public:
    explicit HUDSystem(GameContext& gc);
    ~HUDSystem();

    void updateAll() noexcept;

    void renderAll(bool updateGameData = true) const noexcept;

    void loadFromFile(const std::string& filepath) noexcept;
    void updateAbsoluteData(Vector2f newRes) noexcept;
    void clearAllElements() noexcept;
    void setHUDScale(float newScale) noexcept;
    constexpr float getHUDScale() const noexcept
        { return m_currentHUDScale; }

    HUDElement* getHUDElement(const std::string& name) const noexcept;
    HUDElementText*      getHUDElementText(const std::string& name) noexcept;
    HUDElementImage*     getHUDElementImage(const std::string& name) noexcept;
    HUDElementRectangle* getHUDElementRectangle(const std::string& name) noexcept;
    HUDElementButton*    getHUDElementButton(const std::string& name) noexcept;
    HUDElementCheckbox*  getHUDElementCheckbox(const std::string& name) noexcept;
    HUDElementSlider*    getHUDElementSlider(const std::string& name) noexcept;
    Vector3f getColorFromPalette(const std::string& name) const noexcept;
    std::string getTextureFromList(const std::string& name) const noexcept;

    // Popup actions
    void initPopupSlot(const std::string& slotName, const std::string& textHudElement, const std::string& bgHudElement, const std::string& iconHudElement);
    void updatePopupTimes();
    void clearPopupSlot(const std::string& slotName);
    template <class... Args>
    void addPopup(const std::string& slot, Args&&... args)
    {
        auto& slotData = m_popupSlots.at(slot);
        bool updatedFirst = slotData.addToQueue(std::forward<Args>(args)...);

        if(updatedFirst)
            gctx.hudManager.setPopupData(
                slotData.getNextElement()
                ,slotData.getTextElementName()
                ,slotData.getBackgroundElementName()
                ,slotData.getIconElementName()
            );
    }

    // HUD Element adding
    template <class... Args>
    HUDElement* addNewTextElement(Args&&... args)
    {
        m_hudElementsText.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsText.back() );
        auto& elemText { m_hudElementsText.back() };
        gctx.graphicsEngine.loadFont(elemText.getFont(), elemText.getFontSize());
        return m_hudElements.back();
    }
    template <class... Args>
    HUDElement* addNewImageElement(Args&&... args)
    {
        m_hudElementsImages.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsImages.back() );
        return m_hudElements.back();
    }
    template <class... Args>
    HUDElement* addNewRectangleElement(Args&&... args)
    {
        m_hudElementsRectangles.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsRectangles.back() );
        return m_hudElements.back();
    }
    template <class... Args>
    HUDElement* addNewButtonElement(Args&&... args)
    {
        m_hudElementsButtons.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsButtons.back() );
        return m_hudElements.back();
    }
    template <class... Args>
    HUDElement* addNewCheckboxElement(Args&&... args)
    {
        m_hudElementsCheckbox.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsCheckbox.back() );
        return m_hudElements.back();
    }
    template <class... Args>
    HUDElement* addNewSliderElement(Args&&... args)
    {
        m_hudElementsSliders.emplace_back(std::forward<Args>(args)...);
        m_hudElements.push_back( &m_hudElementsSliders.back() );
        auto& elemText { m_hudElementsSliders.back() };
        gctx.graphicsEngine.loadFont(elemText.getFont(), elemText.getFontSize());
        return m_hudElements.back();
    }
    /*
    void removeTextElement(const std::string& elementName)
    {
        auto it1 = std::find_if(m_hudElements.begin(), m_hudElements.end(),
            [&elementName](HUDElement* tmp){return tmp->getName() == elementName;}
        );
        if(it1 != m_hudElements.end())
            m_hudElements.erase(it1);
        auto& parentList = m_hudElementsText;
        auto it2 = std::find_if(parentList.begin(), parentList.end(),
            [&elementName](HUDElement* tmp){return tmp->getName() == elementName;}
        );
        if(it2 != parentList.end())
            parentList.erase(it2);
    }
    */
private:
    GameContext& gctx;

    // Element drawing function map
    HUDDrawMap m_drawingFunctionMap
    {
         { HUD_TEXT             , drawText            }
        ,{ HUD_IMAGE            , drawImage           }
        ,{ HUD_FILLEDRECTANGLE  , drawFilledRectangle }
        ,{ HUD_BUTTON           , drawButton          }
        ,{ HUD_CHECKBOX         , drawCheckbox        }
        ,{ HUD_SLIDERINT        , drawSliderInt       }
        ,{ HUD_SLIDERFLOAT      , drawSliderFloat     }
        ,{ HUD_LISTBASE         , drawListBase        }
    };

    std::list<HUDElementText>       m_hudElementsText;
    std::list<HUDElementImage>      m_hudElementsImages;
    std::list<HUDElementRectangle>  m_hudElementsRectangles;
    std::list<HUDElementButton>     m_hudElementsButtons;
    std::list<HUDElementCheckbox>   m_hudElementsCheckbox;
    std::list<HUDElementSlider>     m_hudElementsSliders;
    std::list<HUDElementListBase>   m_hudElementsListbases;
    std::list<HUDElement*> m_hudElements;

    std::unordered_map<std::string, Vector3f>    m_colorPalette;
    std::unordered_map<std::string, std::string> m_textures;
    PopupSlotVector m_popupSlots {};

    float m_currentHUDScale {1.f};
};


