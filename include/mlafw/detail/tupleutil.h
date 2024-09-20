#ifndef __MLA_DETAIL_TUPLEUTIL__
#define __MLA_DETAIL_TUPLEUTIL__

#include <sstream>
#include <tuple>
#include <type_traits>

namespace detail::util
{

// Type trait to check if T is in Ts...
template <typename T, typename... Us>
concept contains_type = (std::same_as<T, Us> || ...);

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

// General case for printing other types
template <typename T>
void print_element(std::ostream& os, const T& t)
{
    os << *t;
}

// Tuple printer implementation
template <typename Tuple, std::size_t... I>
void print_tuple_impl(std::ostream& os, const Tuple& t,
                      std::index_sequence<I...>)
{
    os << "(";
    (..., (os << (I == 0 ? "" : ", "), print_element(os, std::get<I>(t))));
    os << ")";
}

// Main function to print tuple
template <typename... Args>
std::string print_tuple(const std::tuple<Args...>& t)
{
    std::stringstream ss;
    print_tuple_impl(ss, t, std::index_sequence_for<Args...>{});
    return ss.str();
}

} // namespace detail::util

#endif
