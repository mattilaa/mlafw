#ifndef __MLA_DETAIL_TUPLEUTIL__
#define __MLA_DETAIL_TUPLEUTIL__

#include <optional>
#include <tuple>
#include <type_traits>

namespace detail::util
{

template <typename... Ts>
using tuple_type = std::tuple<std::optional<Ts>...>;
// Type trait to check if T is in Ts...
template <typename T, typename... Us>
struct contains_type : std::disjunction<std::is_same<T, Us>...>
{
};

// Helper to find the index of a type in the tuple
template <typename T, typename Tuple, std::size_t I = 0>
struct find_type_index
    : std::conditional_t<
          I == std::tuple_size_v<Tuple>,
          std::integral_constant<std::size_t, std::tuple_size_v<Tuple>>,
          std::conditional_t<std::is_same_v<T, typename std::tuple_element_t<
                                                   I, Tuple>::value_type>,
                             std::integral_constant<std::size_t, I>,
                             find_type_index<T, Tuple, I + 1>>>
{
};

} // namespace detail::util

#endif
