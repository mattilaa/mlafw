#include <gtest/gtest.h>
#include "mlafw/mlaattributetuple.h"

using namespace mla::util;

TEST(AttributeTupleTest, ConstructorAndGetter)
{
    AttributeTuple<int, float, std::string> attr(42, 3.14f, "Hello");

    EXPECT_EQ(attr.get<int>(), std::optional<int>(42));
    EXPECT_EQ(attr.get<float>(), std::optional<float>(3.14f));
    EXPECT_EQ(attr.get<std::string>(), std::optional<std::string>("Hello"));
}

TEST(AttributeTupleTest, Setter)
{
    AttributeTuple<int, float, std::string> attr;

    attr.set<int>(100);
    attr.set<float>(2.718f);
    attr.set<std::string>("World");

    EXPECT_EQ(attr.get<int>(), std::optional<int>(100));
    EXPECT_EQ(attr.get<float>(), std::optional<float>(2.718f));
    EXPECT_EQ(attr.get<std::string>(), std::optional<std::string>("World"));
}

TEST(AttributeTupleTest, SetterWithMove)
{
    AttributeTuple<std::string> attr;
    std::string moved_string = "Move me";

    attr.set<std::string>(std::move(moved_string));

    EXPECT_EQ(attr.get<std::string>(), std::optional<std::string>("Move me"));
    // The original string should be empty after move
    EXPECT_TRUE(moved_string.empty());
}

TEST(AttributeTupleTest, ContainsMethod)
{
    using TestTuple = AttributeTuple<int, float, std::string>;

    EXPECT_TRUE(TestTuple::contains<int>());
    EXPECT_TRUE(TestTuple::contains<float>());
    EXPECT_TRUE(TestTuple::contains<std::string>());
    EXPECT_FALSE(TestTuple::contains<double>());
    EXPECT_FALSE(TestTuple::contains<char>());

    AttributeTuple<int, float, std::string> attr;
    if constexpr(attr.contains<double>())
    {
        // Should not ever go here
        ASSERT_TRUE(false);
    }

    // Check with static_assert
    static_assert(attr.contains<int>(), "int should be in the tuple");
    static_assert(attr.contains<float>(), "float should be in the tuple");
    static_assert(attr.contains<std::string>(), "string should be in the tuple");
    static_assert(!attr.contains<double>(), "double should not be in the tuple");
}

TEST(AttributeTupleTest, HasMethod)
{
    AttributeTuple<int, float, std::string> attr;

    EXPECT_FALSE(attr.has<int>());
    EXPECT_FALSE(attr.has<float>());
    EXPECT_FALSE(attr.has<std::string>());

    attr.set<int>(42);
    EXPECT_TRUE(attr.has<int>());
    EXPECT_FALSE(attr.has<float>());
    EXPECT_FALSE(attr.has<std::string>());

    attr.set<std::string>("Hello");
    EXPECT_TRUE(attr.has<int>());
    EXPECT_FALSE(attr.has<float>());
    EXPECT_TRUE(attr.has<std::string>());
}

TEST(AttributeTupleTest, OptionalBehavior)
{
    AttributeTuple<int, float, std::string> attr;

    EXPECT_FALSE(attr.get<int>().has_value());
    EXPECT_FALSE(attr.get<float>().has_value());
    EXPECT_FALSE(attr.get<std::string>().has_value());

    attr.set<int>(42);
    EXPECT_TRUE(attr.get<int>().has_value());
    EXPECT_EQ(attr.get<int>().value(), 42);

    EXPECT_FALSE(attr.get<float>().has_value());
}

TEST(AttributeTupleTest, GetEntireTuple)
{
    AttributeTuple<int, float, std::string> attr(42, 3.14f, "Hello");

    const auto &tuple = attr.get();
    EXPECT_EQ(std::get<0>(tuple), std::optional<int>(42));
    EXPECT_EQ(std::get<1>(tuple), std::optional<float>(3.14f));
    EXPECT_EQ(std::get<2>(tuple), std::optional<std::string>("Hello"));
}

TEST(AttributeTupleTest, SetEntireTuple)
{
    AttributeTuple<int, float, std::string> attr;

    attr.set(100, 2.718f, "World");

    EXPECT_EQ(attr.get<int>(), std::optional<int>(100));
    EXPECT_EQ(attr.get<float>(), std::optional<float>(2.718f));
    EXPECT_EQ(attr.get<std::string>(), std::optional<std::string>("World"));
}

TEST(AttributeTupleTest, PrintTuple)
{
    AttributeTuple<int, float, std::string> attr;

    attr.set(100, 2.718f, "World");
    std::stringstream ss;
    ss << attr;
    EXPECT_EQ(ss.str(), "(100, 2.718, World)");
}
