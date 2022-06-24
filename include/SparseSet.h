#ifndef __SPARSESET_H__
#define __SPARSESET_H__

#include <vector>
#include <array>
#include <limits>
#include <algorithm>
#include <memory>
#include <cassert>
#include <type_traits>
#include <utility>

#include "Entity.h"
#include "Component.h"
#include "PagedVector.h"

#define PAGE_SIZE 128

namespace aecs
{


class Registry;

class SparseSetBase
{
public:
    virtual ~SparseSetBase() {}

    virtual bool contains(Entity ent) = 0;
    virtual void remove(Entity ent) = 0;
};

template<typename T>
class SparseSet : public SparseSetBase
{
public:
    using Page = std::array<size_t, PAGE_SIZE>;

public:
    SparseSet(Registry* reg) : registry_(reg), entities_(0), destroyed_(SIZE_MAX)
    {
        sparse_.resize(8);
    }

    size_t& sparse_at(const size_t n) const
    {
        return sparse_[n / PAGE_SIZE]->operator[](n % PAGE_SIZE);
    }

    T& insert(T&& elem, Entity ent)
    {
        if(contains(ent))
            return denseComponents_[sparse_at(ent.index)]; 

        const size_t pageNo = ent.index / PAGE_SIZE;

        if(pageNo >= sparse_.size())
        {
            sparse_.resize(pageNo + 1);
        }

        if(!sparse_[pageNo])
        {
            sparse_[pageNo] = std::make_unique<Page>();
            sparse_[pageNo]-> fill(SIZE_MAX);
        }

        size_t index = 0;
        if(destroyed_ == SIZE_MAX)
        {
            index = denseComponents_.size();
            denseEntities_.push_back(ent);
            denseComponents_.push_back(std::forward<T>(elem));
        }
        else
        {
            index = destroyed_;
            destroyed_ = denseEntities_[index].index;

            denseEntities_[index] = ent;
            denseComponents_[index] = std::forward<T>(elem);
        }

        sparse_at(ent.index) = index;

        if constexpr(std::is_base_of_v<Component, T>)
        {
            denseComponents_[index].onAdd(*registry_, ent);
        }

        entities_++;
        return denseComponents_[index];
    }

    bool contains(Entity ent) override
    {
        const size_t pageNo = ent.index / PAGE_SIZE;
        if(pageNo >= sparse_.size())
            return false;

        if(!sparse_[pageNo])
            return false;

        const size_t index = sparse_at(ent.index);
        if(index >= denseEntities_.size())
            return false;

        return denseEntities_[index] == ent;
    }

    T& get(Entity ent)
    {
        size_t index = sparse_at(ent.index);
        return denseComponents_[index];
    }

    T* try_get(Entity ent)
    {
        return contains(ent) ? &get(ent) : nullptr;
    }

    void remove(Entity ent) override
    {
        if(!contains(ent))
        {
            return;
        }

        if constexpr(std::is_base_of_v<Component, T>)
        {
            denseComponents_[sparse_at(ent.index)].onRemove(*registry_, ent);
        }

        // Index of the dense array's element which will be deleted
        const size_t dIndex = sparse_at(ent.index);

        denseEntities_[dIndex] = Entity(destroyed_, Entity::max);
        destroyed_ = dIndex;

        sparse_at(ent.index) = SIZE_MAX;

        entities_--;
    }

    /**
     * @brief Get the entities array. There may be invalid
     * entities inside of it so use entity.isValid() to check it
     * 
     * @return const std::vector<Entity>& 
    */
    const std::vector<Entity>& get_entities()
    {
        return denseEntities_;
    }

    /**
     * @brief Get the components array. It may contain
     * removed components
     * 
     * @return const std::vector<Entity>& 
    */
    PagedVector<T, PAGE_SIZE>& get_components()
    {
        return denseComponents_;
    }

    size_t entities_count()
    {
        return entities_;
    }

    size_t count_allocated_pages() const
    {
        size_t counter = 0;
        for(size_t i = 0; i < sparse_.size(); i++)
        {
            if(sparse_[i]) counter++;
        }
        return counter;
    }

private:
    size_t destroyed_;
    size_t entities_;

    PagedVector<T, PAGE_SIZE> denseComponents_;
    std::vector<Entity> denseEntities_;
    std::vector<std::unique_ptr<Page>> sparse_;

    Registry* registry_;
};



} // namespace aecs

#endif // __SPARSESET_H__