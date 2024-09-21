#include "mlafw/arrayquickmap.h"
#include "mlafw/vectorquickmap.h"
#include <gtest/gtest.h>
#include <string>

// Test fixture for both ArrayQuickMap and VectorQuickMap
template <typename MapType>
class QuickMapTest : public ::testing::Test
{
protected:
    MapType map;
};

// Define the types we want to test
using MapTypes = ::testing::Types<mla::ArrayQuickMap<std::string, int, 16>,
                                  mla::VectorQuickMap<std::string, int>>;
TYPED_TEST_SUITE(QuickMapTest, MapTypes);

TYPED_TEST(QuickMapTest, InsertAndGet)
{
    this->map.insert("one", 1);
    this->map.insert("two", 2);

    EXPECT_EQ(this->map.get("one"), 1);
    EXPECT_EQ(this->map.get("two"), 2);
    EXPECT_EQ(this->map.get("three"), std::nullopt);
}

TYPED_TEST(QuickMapTest, Size)
{
    EXPECT_EQ(this->map.size(), 0);

    this->map.insert("one", 1);
    EXPECT_EQ(this->map.size(), 1);

    this->map.insert("two", 2);
    EXPECT_EQ(this->map.size(), 2);
}

TYPED_TEST(QuickMapTest, Remove)
{
    this->map.insert("one", 1);
    this->map.insert("two", 2);

    this->map.remove("one");
    EXPECT_EQ(this->map.get("one"), std::nullopt);
    EXPECT_EQ(this->map.size(), 1);

    EXPECT_EQ(this->map.get("two"), 2);
}

TYPED_TEST(QuickMapTest, Iterator)
{
    this->map.insert("one", 1);
    this->map.insert("two", 2);
    this->map.insert("three", 3);

    std::vector<std::pair<std::string, int>> expected = {
        {"one", 1}, {"two", 2}, {"three", 3}};
    std::vector<std::pair<std::string, int>> actual;

    for(const auto& pair : this->map)
    {
        actual.push_back({pair.first, pair.second});
    }

    EXPECT_EQ(actual.size(), expected.size());
    for(const auto& pair : expected)
    {
        EXPECT_NE(std::find(actual.begin(), actual.end(), pair), actual.end());
    }
}

TYPED_TEST(QuickMapTest, Find)
{
    this->map.insert("one", 1);
    this->map.insert("two", 2);

    auto it = this->map.find("two");
    EXPECT_NE(it, this->map.end());
    EXPECT_EQ((*it).second, 2);

    // Instead of comparing iterators directly, check if the key exists
    auto not_found = this->map.find("three");
    auto value = this->map.get("three");
    EXPECT_EQ(value, std::nullopt);
}

// Specific test for ArrayQuickMap to check capacity
TEST(ArrayQuickMapTest, Capacity)
{
    mla::ArrayQuickMap<int, int, 32> map;
    EXPECT_EQ(map.capacity(), 32);
}

// Specific test for VectorQuickMap to check rehashing
TEST(VectorQuickMapTest, Rehash)
{
    mla::VectorQuickMap<int, int> map(2); // Start with a small capacity
    for(int i = 0; i < 100; ++i)
    {
        map.insert(i, i);
    }
    EXPECT_EQ(map.size(), 100);
    for(int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(map.get(i), i);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
