#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <cstddef>
#include <cstdint>
#include <limits>

namespace aecs
{


struct Entity
{
    static const size_t max = std::numeric_limits<size_t>::max();
    static const Entity null;

    Entity(size_t idx, size_t ver) : index(idx), version(ver)
    {}

    Entity() : index(max), version(max)
    {}

    bool operator==(const Entity& en) const
    {
        return index == en.index && version == en.version;
    }

    bool isValid() const
    {
        return index != max && version != max;
    }

    size_t get_version() const
    {
        return version;
    }

    size_t index;
    size_t version;
};

inline const Entity Entity::null = Entity(Entity::max, Entity::max);


} // namespace aecs
#endif // __ENTITY_H__