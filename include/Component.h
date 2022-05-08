#ifndef __AECS_COMPONENT_H__
#define __AECS_COMPONENT_H__

#include "Entity.h"

namespace aecs
{


class Registry;

struct Component
{
    virtual void onAdd(Registry& reg, Entity ent) {}
    virtual void onRemove(Registry& reg, Entity ent) {}
};


} // namespace aecs
#endif // __AECS_COMPONENT_H__