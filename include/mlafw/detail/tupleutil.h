#ifndef __MLA_DETAIL_TUPLEUTIL__
#define __MLA_DETAIL_TUPLEUTIL__

#include <sstream>
#include <tuple>
#include <type_traits>

namespace detail::util
{

template<typename T>
concept TupleType = requires { typename std::tuple_size<T>::type; };

template <typename T, TupleType Tuple>
class find_type_index
{
    template <std::size_t I>
    static constexpr bool is_same_as_element()
    {
        if constexpr(I < std::tuple_size_v<Tuple>)
        {
            return std::is_same_v<
                T, typename std::tuple_element_t<I, Tuple>::value_type>;
        }
        return false;
    }

    template <std::size_t... Is>
    static constexpr std::size_t find_index(std::index_sequence<Is...>)
    {
        return ((is_same_as_element<Is>() ? Is : 0) + ...);
    }

public:
    static constexpr std::size_t value =
        find_index(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
};

template<typename T, TupleType Tuple>
inline constexpr std::size_t find_type_index_v = find_type_index<T, Tuple>::value;

// Type trait to check if T is in Ts...
template <typename T, typename... Us>
concept contains_type = (std::same_as<T, Us> || ...);

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
