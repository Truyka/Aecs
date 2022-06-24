#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include "SparseSet.h"
#include "FamilyGenerator.h"
#include "BasicView.h"
#include "TupleUtility.h"
#include "Component.h"

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>
#include <tuple>

/* AECS VERSION: 1.1
*/

namespace aecs
{



class Registry
{
public:
    template<typename T>
    using storage_ptr = std::unique_ptr<SparseSet<T>>;

    using storage_base_ptr = std::unique_ptr<SparseSetBase>;

    using entity_storage = std::vector<Entity>;
    
public:
    Registry() : destroyed_(SIZE_MAX)
    {

    }

    /**
     * @brief Get the pool containing the specified component, create
     * and initialize a pool if it doesn't already exist
     * 
     * @return pointer to a SparseSet with the specified components
    */
    template<typename Component>
    SparseSet<Component>* get_pool()
    {
        const size_t index = FamilyGenerator::index<Component>();
        if(index >= pools_.size())
        {
            pools_.resize(index + 1);
        }

        if(!pools_[index])
        {
            pools_[index] = std::make_unique<SparseSet<Component>>(this);
        }

        return get_pool_at<Component>(index);
    }

private:
    /**
     * @brief Gets and converts a base pool into its accessible form and 
     * 
     * @param index index of the pool you want to get, default value is set to
     * the right index of the component's pool
     * 
     * @return converted pointer to the right pool
    */
    template<typename Component>
    SparseSet<Component>* get_pool_at(size_t index = FamilyGenerator::index<Component>())
    {
        SparseSet<Component>* ptr = static_cast<SparseSet<Component>*>(pools_[index].get());
        return ptr;
    }

public:
    /**
     * @brief Constructs a component from given args using list initialization
     * and adds it to an entity. Does nothing if an entity already has that component
     * 
     * @param ent entity
     * @param args arguments you will initialize the component with
     * 
     * @return Component& a reference to the added component or to an
     * already existing one
    */
    template<typename Component, typename... Args>
    std::enable_if_t<std::is_default_constructible<Component>::value, Component&>
    add(Entity ent, Args&&... args)
    {
        auto pool = get_pool<Component>();
        Component c{args...};
        return pool->insert(std::move(c), ent);
    }

    /**
     * @brief Constructs a component from given args using its constructor
     * and adds it to an entity. Does nothing if an entity already has that component
     * 
     * @param ent entity
     * @param args arguments you will initialize the component with
     * 
     * @return Component& a reference to the added component or to an
     * already existing one
    */
    template<typename Component, typename... Args>
    std::enable_if_t<!std::is_default_constructible<Component>::value, Component&>
    add(Entity ent, Args&&... args)
    {
        auto pool = get_pool<Component>();
        Component c(std::forward<Args>(args)...);
        return pool->insert(std::move(c), ent);
    }

    /**
     * @brief Gets a component from an entity
     * 
     * @warning can cause undefined behaviour if your entity
     * doesn't have the specified component
     * 
     * @param ent an entity you're getting the component from
     * 
     * @return Component& a reference to this component
    */
    template<typename Component>
    Component& get(Entity ent)
    {
        auto pool = get_pool_at<Component>();
        return pool->get(ent);
    }

    /**
     * @brief Gets a pointer to a component from an enttiy
     * 
     * @param ent an entity you're getting the component from
     * 
     * @return Component* a pointer to this component or nullptr
     * if your entitiy doesn't have it
    */
    template<typename Component>
    Component* try_get(Entity ent)
    {
        auto pool = get_pool<Component>();
        return pool->try_get(ent);
    }

    /**
     * @brief Checks if an entity has every given component
     * 
     * @tparam Component... all components to match against
     * @param ent 
    */
    template<typename... Component>
    bool has(Entity ent)
    {
        return (get_pool<Component>()->contains(ent) && ...);
    }

    /**
     * @brief Removes a component from an entity
     * 
     * @param ent an entity you're removing from
    */
    template<typename Component>
    void remove(Entity ent)
    {
        auto pool = get_pool<Component>();
        pool->remove(ent);
    }

    /** 
     * @brief Removes every component from an entity
     * and adds it to the removed linked list
     * 
     * @param ent entity
    */
    void remove(Entity ent)
    {
        for(const auto& pool : pools_)
        {
            pool->remove(ent);
        }

        if(ent.index < entities_.size())
        {
            // Update the linked list so it points to the next
            // destroyed entity
            entities_[ent.index].index = destroyed_;
            entities_[ent.index].version++;

            destroyed_ = ent.index;
        }
    }

    /**
     * @brief Creates a new entity. If any entities have 
     * been destroyed it reuses them (their version is
     * incremented by one when removing)
     * 
     * @return Entity 
    */
    Entity create()
    {
        // If there aren't any free/destroyed entities
        if(destroyed_ == SIZE_MAX)
        {
            Entity new_ent(entities_.size(), 0);
            entities_.push_back(new_ent);
            return new_ent;
        }
        else
        {
            const size_t free_index = destroyed_;

            // Update the linked list to point to the next destroyed
            destroyed_ = entities_[free_index].index;

            // Take the most recent destroyed entity's index
            entities_[free_index].index = free_index;
            return entities_[free_index];
        }
    }


    /**
     * @brief Makes a single component view of given component
     * It can be used in range based loops or by using its each() method
     * 
     * @return SingleView<Component> 
    */
    template<typename Component>
    SingleView<Component> view()
    {
        auto pool = get_pool<Component>();
        return SingleView<Component>(pool->get_entities(), this);
    }

    /**
     * @brief Makes a multi component view of given components
     * It can be used in range based loops or by using its each() method
     * 
     * @return MultiView<Comps...> 
    */
    template<typename... Comps, 
             typename std::enable_if_t<(sizeof...(Comps) >= 2), bool> = true>
    MultiView<Comps...> view()
    {
        // Get all the needed pools into a tuple
        std::tuple pools( get_pool<Comps>()... );

        // We assume the smallest one is the first one
        size_t smallest_index = 0;
        const entity_storage* smallest = &std::get<0>(pools)->get_entities();

        // A lambda which we'll use on every tuple element to find the
        // one with smallest number of entities
        auto set_smallest = [&](auto&& poolptr, size_t& i)
        {
            if(poolptr->entities_count() < smallest->size())
            {
                smallest = &poolptr->get_entities();
                smallest_index = i;
            }
            i++;
        };

        // Find smallest pool using 'set_smallest' lambda
        std::apply([&](auto&&... poolptr)
        {
            size_t i = 0;
            (set_smallest(poolptr, i), ...);
        }, pools);

        // This will check if every given pool contains this entity
        // and push it into the 'entities' vector
        std::vector<Entity> entities;
        entities.reserve(smallest->size());
        for(const Entity& entity : *smallest)
        {
            if(!entity.isValid()) continue;

            bool contains = true;
            tplu::apply_without(smallest_index, pools, [&](auto&& poolptr)
            {
                if(!poolptr->contains(entity))
                    contains = false;
            });

            if(contains) entities.push_back(entity);
        }
        entities.shrink_to_fit();
        return MultiView<Comps...>(std::move(entities), this);
    }

private:
    size_t destroyed_;
    entity_storage entities_;
    std::vector<storage_base_ptr> pools_;
};



template<typename C>
template<typename L>
void SingleView<C>::each(L lambda)
{
    auto p = registry_->get_pool<C>();
    auto& comps = p->get_components();
    auto& ents  = p->get_entities();

    for(size_t i = 0; i < ents.size(); i++)
    {
        if(ents[i].isValid())
        {
            lambda(comps[i]);
        }
    }
}

template<typename C1, typename C2, typename... CN>
template<typename L>
void MultiView<C1, C2, CN...>::each(L lambda)
{
    for(const auto& entity : entities_)
    {
        lambda(registry_->get<C1>(entity), 
               registry_->get<C2>(entity), 
               registry_->get<CN>(entity)...);
    }
}



} // namespace aecs
#endif // __REGISTRY_H__