#ifndef __BASICVIEW_H__
#define __BASICVIEW_H__

#include <memory>
#include <type_traits>
#include "Entity.h"

namespace aecs
{


class Registry;

template<typename Component>
class SingleView
{
public:
    using entity_storage = std::vector<Entity>;

    class iterator
    {
    public:
        using difference_type = size_t;
        using value_type = const Entity;
        using pointer = const Entity*;
        using reference = const Entity&;
        using iterator_category = std::forward_iterator_tag;

        iterator(size_t i, const entity_storage& s) : idx(i), ents(s) {}

        iterator& operator++()
        {
            do { ++idx; } while(idx < ents.size() && !ents[idx].isValid());
            return *this;
        }

        iterator  operator++(int)
        {
            iterator cpy = *this;
            ++(*this);
            return cpy;
        }

        bool operator==(iterator& other) const { return idx == other.idx; }
        bool operator!=(iterator& other) const { return !(*this == other); }
        const Entity& operator*()                         { return ents[idx]; }

    private:
        size_t idx;
        const entity_storage& ents;
    };

public:
    SingleView(const entity_storage& ctnr, Registry* reg) 
                        : entities_(ctnr), registry_(reg)
    {}

    /**
     * @brief Calls the given lambda on every component of type Component
     * 
     * @warning May cause undefined behaviour if you're adding/deleting
     * new components while iterating
     * 
     * @param lambda custom lambda which arguments match view components
    */
    template<typename L>
    void each(L lambda);

    size_t size() const
    {
        return entities_.size();
    }

    const entity_storage& getInnerContainer()
    {
        return entities_;
    }

    Entity front()
    {
        size_t idx = find_begin_idx();
        if(idx < entities_.size())
            return entities_[idx];

        return Entity::null;
    }

    auto begin() { return iterator(find_begin_idx(), entities_); }
    auto end()   { return iterator(entities_.size(), entities_); }

private:
    size_t find_begin_idx()
    {
        size_t idx = 0;
        for(; idx < entities_.size(); idx++)
        {
            if(entities_[idx].isValid())
                return idx;
        }
        return idx;
    }

private:
    const entity_storage& entities_;
    Registry* registry_;
};



template<typename C1, typename C2, typename... CN>
class MultiView
{
public:
    using entity_storage = std::vector<Entity>;

public:
    MultiView(entity_storage&& ctnr, Registry* reg) 
              : entities_(std::move(ctnr)), registry_(reg)
    {}

    /**
     * @brief Calls the given lambda on every component of type Component
     * 
     * @warning May cause undefined behaviour if you're adding/deleting
     * new components while iterating
     * 
     * @param lambda custom lambda which arguments match view components
    */
    template<typename L>
    void each(L lambda);

    size_t size() const
    {
        return entities_.size();
    }

    Entity front()
    {
        if(entities_.size() > 0)
            return entities_[0];

        return Entity::null;
    }
    
    const entity_storage& getInnerContainer()
    {
        return entities_;
    }

    auto begin() { return entities_.begin(); }
    auto end()   { return entities_.end();   }
    
private:
    entity_storage entities_;
    Registry* registry_;
};


} // namespace aecs
#endif // __BASICVIEW_H__