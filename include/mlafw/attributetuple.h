#ifndef __MLA_ATTRIBUTETUPLE__
#define __MLA_ATTRIBUTETUPLE__

#include "detail/tupleutil.h"

#include <optional>
#include <ostream>
#include <tuple>
#include <utility>

namespace mla::util
{

// Concept for primitive types
template <typename T>
concept Primitive = std::is_arithmetic_v<T>;

template<typename T, typename Tag = void>
class Attribute
{
public:
    Attribute() = default;
    Attribute(T value) requires Primitive<T> : val(value) {}
    Attribute(const T& value) : val(value) {}
    Attribute(T&& value) : val(std::move(value)) {}

    friend std::ostream& operator<<(std::ostream& os, const Attribute& attr)
    {
        if constexpr(requires { { Tag::name() } ->
                std::convertible_to<std::string_view>; })
        {
            os << Tag::name() << ":";
        }
        if(attr.val.has_value())
            os << *attr.val;
        return os;
    }

    // For primitive types
    constexpr T get() const
        requires Primitive<T>
    {
        return val.value_or(T{});
    }

    // For non-primitive types
    const T& get() const
        requires(!Primitive<T>)
    {
        static const T defValue{};
        return val.value_or(defValue);
    }

    // Non-const version for non-primitive types
    constexpr T& get()
        requires(!Primitive<T>)
    {
        if(!val.has_value())
            val = T{};
        return val.value();
    }

private:
    std::optional<T> val;
};

template <typename... Ts>
class AttributeTuple
{
public:
    using tuple_type = std::tuple<std::optional<Ts>...>;

    // Default constructor
    AttributeTuple() = default;

    // Constructor taking 1..N elements
    template <typename... Args>
    explicit AttributeTuple(Args&&... args)
        : data(std::make_optional<Ts>(std::forward<Args>(args))...)
    {
    }

    // Setter for the entire tuple
    template <typename... Args>
    void set(Args&&... args)
    {
        data = tuple_type(std::make_optional<Ts>(std::forward<Args>(args))...);
    }

    // Getter for the stored tuple
    const auto& get() const
    {
        return data;
    }

    // Getter for a specific element type
    template <typename T>
    std::optional<T> get() const
    {
        static_assert(detail::util::contains_type<T, Ts...>,
                      "Type not found in AttributeTuple");
        constexpr std::size_t index =
            detail::util::find_type_index_v<T, tuple_type>;
        return std::get<index>(data);
    }

    // Setter for a specific element type
    template <typename T>
    void set(const T& value)
    {
        static_assert(detail::util::contains_type<T, Ts...>,
                      "Type not found in AttributeTuple");
        constexpr std::size_t index =
            detail::util::find_type_index_v<T, tuple_type>;
        std::get<index>(data) = std::make_optional(value);
    }

    // Setter for a specific element type (move version)
    template <typename T>
    void set(T&& value)
    {
        static_assert(detail::util::contains_type<T, Ts...>,
                      "Type not found in AttributeTuple");
        constexpr std::size_t index =
            detail::util::find_type_index_v<T, tuple_type>;
        std::get<index>(data) = std::make_optional(std::forward<T>(value));
    }

    // Check if a specific type exists in the tuple
    template <typename T>
    static constexpr bool contains()
    {
        return detail::util::contains_type<T, Ts...>;
    }

    // Check if a specific type has a value set in the tuple
    template <typename T>
    bool has() const
    {
        if constexpr(detail::util::contains_type<T, Ts...>)
        {
            constexpr std::size_t index =
                detail::util::find_type_index_v<T, tuple_type>;
            return std::get<index>(data).has_value();
        }
        return false;
    }

    // Printer for AttributeTuple
    friend std::ostream& operator<<(std::ostream& os, const AttributeTuple& tuple)
    {
        return os << detail::util::print_tuple(tuple.data);
    }
private:
    tuple_type data;
};

} // namespace mla::util

#endif // __MLA_ATTRIBUTETUPLE__
