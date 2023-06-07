#include "hudPopup.hpp"
#include "../util/macros.hpp"
#include <iostream>
#include <algorithm>

bool PopupSlot::categoryIsInQueue(const std::string& query)
{
    for(auto& category : popupCategories)
        if(query == category)
            return true;
    return false;
}

void PopupSlot::removeCategoryFromList(const std::string& query)
{  
    popupCategories.erase(
        std::find(popupCategories.begin(), popupCategories.end(), query)
    ); 
}
bool PopupSlot::updateQueue(float deltaTime)
{
    if(popupQueue.empty()) return false;
    auto& next = popupQueue.front();
    next.timeLeft -= deltaTime;
    if(next.timeLeft <= 0.f)
    {
        removeCategoryFromList(next.category);
        popupQueue.pop();
        return true;
    }
    return false;
}

void PopupSlot::clearQueue()
{
    while(!popupQueue.empty())
        popupQueue.pop();
    popupCategories.clear();
}

HUDPopup* PopupSlot::getNextElement() noexcept 
{
    if(popupQueue.empty()) return nullptr;
    return &popupQueue.front(); 
}

bool PopupSlot::replacePopupOfSameCategory(const std::string& p_cat, HUDPopup& copy)
{
    bool wasFirst { false };
    auto queueSize { popupQueue.size() };
    for(unsigned int i=0; i<queueSize; i++)
    {
        auto nextCopy = popupQueue.front();
        popupQueue.pop();

        if(nextCopy.category == p_cat)
        {
            if(i==0) wasFirst = true;
            popupQueue.push(copy);   
        }
        else
        {
            popupQueue.push(nextCopy);   
        }
    }
    return wasFirst;
}
