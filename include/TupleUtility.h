#include <tuple>
#include <cstddef>

namespace tplu
{


/**
 * @brief Applies a given function to given objects in a
 * tuple by passing the current object as an argument 
 * 
 * @tparam I the amount you're offseting 'indices' by
 * @param tpl tuple
 * @param idxs indexes you want to visit
 * @param lambda your function
*/
template<size_t I = 0, typename F, typename... Ts, size_t... Indices>
void index_apply(std::tuple<Ts...>& tpl, 
                 std::index_sequence<Indices...> idxs, 
                 F&& lambda)
{
    (lambda(std::get<Indices + I>(tpl)), ...);
}



/**
 * @brief Applies a given function to every object in a
 * tuple without the object with the index 'I' by passing the 
 * current object as an argument 
 * 
 * @tparam I index you want to skip
 * @param tpl tuple
 * @param lambda your function
*/
template<size_t I, typename F, typename... Ts>
void apply_without(std::tuple<Ts...>& tpl, F&& lambda)
{
    constexpr size_t tuple_size = sizeof...(Ts);
    static_assert((I < tuple_size), "Removed index is out of bounds!");

    auto head = std::make_index_sequence<I>();
    auto tail = std::make_index_sequence<tuple_size - I - 1>();

    index_apply(tpl, head, lambda);
    index_apply<I+1>(tpl, tail, lambda);
}



/**
 * @brief Makes 'apply_without' callable with a runtime index 
 * by using compile-time recursion
*/
template<size_t I>
struct apply_without_runtime_impl
{
    template<typename F, typename... Ts>
    static void apply(size_t i, std::tuple<Ts...>& tpl, F&& lambda)
    {
        if(I == i) apply_without<I>(tpl, lambda);
        else apply_without_runtime_impl<I-1>::apply(i, tpl, lambda);
    }
};

/**
 * @brief Partial specialization to stop the recursion
*/
template<>
struct apply_without_runtime_impl<0>
{
    template<typename F, typename... Ts>
    static void apply(size_t i, std::tuple<Ts...>& tpl, F&& lambda)
    {
        apply_without<0>(tpl, lambda);
    }
};

/**
 * @brief A nice wrapper which calls a function upon
 * every object in a tuple except the one at a specified
 * index
 * 
 * @param i the index you want to skip
 * @param tpl tuple
 * @param lambda the function
*/
template<typename F, typename... Ts>
void apply_without(size_t i, std::tuple<Ts...>& tpl, F&& lambda)
{
    apply_without_runtime_impl<sizeof...(Ts)-1>::apply(i, tpl, lambda);
}


} // namespace tplu