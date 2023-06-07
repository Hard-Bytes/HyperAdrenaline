#pragma once
#include "../util/typeAliases.hpp"
#include "../components/allComponentsInclude.hpp"
#include "../facade/vector3f.hpp"
#include <vector>

struct ComponentVectorBase {
    virtual ~ComponentVectorBase() = default;
};

template<typename Component_t>
struct ComponentVector : public ComponentVectorBase {
    //~ComponentVector() { componentVector.clear(); }
    Vector<Component_t> componentVector;
};

class ComponentManager
{
public:
    // Constructor and destructor
    explicit ComponentManager(int reservedComponents);
    ~ComponentManager();

    // Create component
    /*Creates a component of the given type (by template)*/
    template<typename Component_t, typename... Args>
    Component_t& createComponent_t(Args&&... args) { 
        auto& comvec = this->getComponentVector_t<Component_t>();
        comvec.emplace_back(std::forward<Args>(args)...);
        return comvec.back();
    }

    // Get Component By ID
    /*Gets a component given its type (by template) and the ID of its entity*/
    template<typename Component_t>
    Component_t* getComponentByID_t(EntityID entID) {
        auto& comvec = this->getComponentVector_t<Component_t>();
        auto iter = std::find_if(begin(comvec), end(comvec),
            [&entID](Component_t& cmp){
                    return cmp.getEntityID() == entID;
                }
            );

        // If element was found, return it
        if(iter != end(comvec))
            return &(*iter);

        return nullptr;
    }

    /*Clear the given component of a given entity*/
    template<typename Component_t>
    void clearComponentByID_t(EntityID entID)
    {
        auto& comvec = this->getComponentVector_t<Component_t>();
        auto iter = std::find_if(begin(comvec), end(comvec),
            [&entID](Component_t& cmp){
                    return cmp.getEntityID() == entID;
                }
            );

        // If element was found, erase it
        if(iter != end(comvec))
            comvec.erase(iter);
    }

    // Get Component Vector
    /*Gets the component vector of the specified type. If it didn't exist, it is created*/
    template<typename Component_t>
    Vector<Component_t>& getComponentVector_t() {
        Vector<Component_t>* comvec { nullptr };

        auto type = Component_t::getComponentTypeID();
        auto iter = m_componentVectors.find(type);

        if(iter != m_componentVectors.end())
        {
            auto* v = static_cast<ComponentVector<Component_t>*>(iter->second.get());
            comvec = &v->componentVector;
        }
        else
        {
            comvec = &createComponentVector_t<Component_t>();
        }

        return *comvec;
    }

    // Create Component Vector
    /*Creates a vector of the specified component type*/
    template<typename Component_t>
    Vector<Component_t>& createComponentVector_t() {
        auto type = Component_t::getComponentTypeID();
        auto cmpvec = std::make_unique<ComponentVector<Component_t>>();
        cmpvec->componentVector.reserve(maxComponents);
        auto* vecptr = cmpvec.get();
        m_componentVectors[type] = std::move(cmpvec);

        return vecptr->componentVector;
    }

    // Delete Component Vector
    /*Gets the component vector of the specified type and destroys it*/
    template<typename Component_t>
    void deleteComponentVector_t() {
        auto type = Component_t::getComponentTypeID();
        auto iter = m_componentVectors.find(type);

        if(iter != m_componentVectors.end())
        {
            this->getComponentVector_t<Component_t>().clear();
            iter->second.reset();
            m_componentVectors.erase(iter);
        }
    }

    // Destroyers
    void clearAllComponentsOfEntity(EntityID entID);
    void clearAllComponents();
private:
    int maxComponents;
    HashTable<ComponentID, UPointer<ComponentVectorBase>> m_componentVectors;
};
