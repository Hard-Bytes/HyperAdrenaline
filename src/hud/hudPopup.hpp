#pragma once
#include <string>
#include <queue>
#include <unordered_map>

struct HUDPopup
{
    explicit HUDPopup(
        const std::string& p_cat
        , const std::string& p_msg
        , const std::string& p_bg
        , const std::string& p_icon
        , float p_totalTime
    )
        : category        (p_cat)
        , message         (p_msg)
        , bgTextureName   (p_bg)
        , iconTextureName (p_icon)
        , timeLeft        (p_totalTime)
    {}
    ~HUDPopup() = default;

    std::string category;
    std::string message;
    std::string bgTextureName;
    std::string iconTextureName;
    float timeLeft;
};

struct PopupSlot
{
    explicit PopupSlot(const std::string& p_textElement, const std::string& p_bgElement, const std::string& p_iconElement)
        : hudElementTextName(p_textElement)
        , hudElementBgName(p_bgElement)
        , hudElementIconName(p_iconElement)
    {}
    ~PopupSlot() = default;

    bool categoryIsInQueue(const std::string& query);
    void removeCategoryFromList(const std::string& query);
    bool updateQueue(float deltaTime);
    void clearQueue();
    HUDPopup* getNextElement() noexcept;

    template <class... Args>
    bool addToQueue(const std::string& p_cat, Args&&... args)
    {
        bool wasFirst { false };
        if(categoryIsInQueue(p_cat))
        {
            // Replace current popup of this category with the new one
            HUDPopup tmp(p_cat, std::forward<Args>(args)...);
            wasFirst = replacePopupOfSameCategory(p_cat, tmp);
        }
        else
        {
            // Add the new popup at the end of the queue
            popupQueue.emplace(p_cat, std::forward<Args>(args)...);
            popupCategories.push_back(p_cat);
            if(popupQueue.size() == 1) wasFirst = true;
        }

        return wasFirst;
    }

    const std::string& getTextElementName()       const noexcept { return hudElementTextName; }
    const std::string& getBackgroundElementName() const noexcept { return hudElementBgName;   }
    const std::string& getIconElementName()       const noexcept { return hudElementIconName;   }
    int getQueueSize() const noexcept { return popupQueue.size(); }
private:
    std::queue<HUDPopup> popupQueue {};
    std::vector<std::string> popupCategories {};
    std::string hudElementTextName;
    std::string hudElementBgName;
    std::string hudElementIconName;

    // Recursive function
    bool replacePopupOfSameCategory(const std::string& p_cat, HUDPopup& copy);
};
using PopupSlotVector = std::unordered_map<std::string, PopupSlot>;
using HUDSlotPair = std::pair<std::string, PopupSlot>;
