#ifndef __FAMILYGENERATOR_H__
#define __FAMILYGENERATOR_H__

#include <cstddef>

namespace aecs
{


class FamilyGenerator
{
public:
    template<typename>
    static size_t index()
    {
        static const size_t idx = get_next();
        return idx;
    }

private:
    static size_t get_next()
    {
        static int i = 0;
        return i++;
    }
};


} // namespace aecs
#endif // __FAMILYGENERATOR_H__