#ifndef __MLA_ATTRIBUTETUPLE__
#define __MLA_ATTRIBUTETUPLE__

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mla::util
{

template <typename... Ts>
class AttributeTuple
{
public:
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
        data = std::tuple<std::optional<Ts>...>(
            std::make_optional<Ts>(std::forward<Args>(args))...);
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
        static_assert(contains_type<T, Ts...>::value,
                      "Type not found in tuple");
        constexpr std::size_t index =
            find_type_index<T, std::tuple<std::optional<Ts>...>>::value;
        return std::get<index>(data);
    }

    // Setter for a specific element type
    template <typename T>
    void set(const T& value)
    {
        static_assert(contains_type<T, Ts...>::value,
                      "Type not found in tuple");
        constexpr std::size_t index =
            find_type_index<T, std::tuple<std::optional<Ts>...>>::value;
        std::get<index>(data) = std::make_optional(value);
    }

    // Setter for a specific element type (move version)
    template <typename T>
    void set(T&& value)
    {
        static_assert(contains_type<T, Ts...>::value,
                      "Type not found in tuple");
        constexpr std::size_t index =
            find_type_index<T, std::tuple<std::optional<Ts>...>>::value;
        std::get<index>(data) = std::make_optional(std::forward<T>(value));
    }

    // Check if a specific type exists in the tuple
    template <typename T>
    static constexpr bool contains()
    {
        return contains_type<T, Ts...>::value;
    }

    // Check if a specific type has a value set in the tuple
    template <typename T>
    bool has() const
    {
        if constexpr(contains_type<T, Ts...>::value)
        {
            constexpr std::size_t index =
                find_type_index<T, std::tuple<std::optional<Ts>...>>::value;
            return std::get<index>(data).has_value();
        }
        return false;
    }

    // Getter for the stored tuple type
    using tuple_type = std::tuple<std::optional<Ts>...>;
private:
    std::tuple<std::optional<Ts>...> data;

    // Type trait to check if T is in Ts...
    template <typename T, typename... Us>
    struct contains_type : std::disjunction<std::is_same<T, Us>...>
    {
    };

    // Helper to find the index of a type in the tuple
    template <typename T, typename Tuple, std::size_t I = 0>
    struct find_type_index : std::conditional_t<
              I == std::tuple_size_v<Tuple>,
              std::integral_constant<std::size_t, std::tuple_size_v<Tuple>>,
              std::conditional_t<
                  std::is_same_v<
                      T, typename std::tuple_element_t<I, Tuple>::value_type>,
                  std::integral_constant<std::size_t, I>,
                  find_type_index<T, Tuple, I + 1>>>
    {
    };
};

} // namespace mla::util

#endif // __MLA_ATTRIBUTETUPLE__
