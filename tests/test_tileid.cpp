#include "pch.h"
#include "../src/gisdom/tileid.h"
#include <unordered_set>

using namespace gce;

class TileidTest : public ::testing::Test
{
protected:
    tileid tile_empty;
    tileid tile_z0{0, 0, 0};
    tileid tile_z1{0, 0, 1};
    tileid tile_z5{10, 15, 5};
};

// Construction tests
TEST_F(TileidTest, DefaultConstructor)
{
    tileid tile;
    EXPECT_TRUE(tile.empty());
    EXPECT_EQ(tile.get_value(), 0u);
    EXPECT_EQ(tile.get_x(), 0u);
    EXPECT_EQ(tile.get_y(), 0u);
    EXPECT_EQ(tile.get_z(), 0u);
}

TEST_F(TileidTest, ConstructorFromValue)
{
    tileid::value_type key = 0x1234567890ABCDEFull;
    tileid tile(key);
    EXPECT_EQ(tile.get_value(), key);
}

TEST_F(TileidTest, ConstructorFromComponents)
{
    tileid tile(5u, 10u, 3u);
    EXPECT_EQ(tile.get_x(), 5u);
    EXPECT_EQ(tile.get_y(), 10u);
    EXPECT_EQ(tile.get_z(), 3u);
}

TEST_F(TileidTest, ConstructorZeroLevel)
{
    tileid tile(0u, 0u, 0u);
    EXPECT_EQ(tile.get_x(), 0u);
    EXPECT_EQ(tile.get_y(), 0u);
    EXPECT_EQ(tile.get_z(), 0u);
    EXPECT_TRUE(tile.empty());
}

TEST_F(TileidTest, ConstructorMaxLevel)
{
    uint8_t max_z = static_cast<uint8_t>(tileid::max_level());
    tileid tile(100u, 200u, max_z);
    EXPECT_EQ(tile.get_z(), max_z);
}

// Component masking tests
TEST_F(TileidTest, ComponentMaskingX)
{
    uint32_t x_value = (1u << tileid::X_BITS) + 5u; // Overflow x
    tileid tile(x_value, 0u, 0u);
    EXPECT_EQ(tile.get_x(), 5u);
}

TEST_F(TileidTest, ComponentMaskingY)
{
    uint32_t y_value = (1u << tileid::Y_BITS) + 10u; // Overflow y
    tileid tile(0u, y_value, 0u);
    EXPECT_EQ(tile.get_y(), 10u);
}

TEST_F(TileidTest, ComponentMaskingZ)
{
    uint8_t z_value = (1u << tileid::Z_BITS) + 5u; // Overflow z
    tileid tile(0u, 0u, z_value);
    EXPECT_EQ(tile.get_z(), 5u);
}

// Accessor tests
TEST_F(TileidTest, GetXYZ)
{
    tileid tile(42u, 99u, 5u);
    EXPECT_EQ(tile.get_x(), 42u);
    EXPECT_EQ(tile.get_y(), 99u);
    EXPECT_EQ(tile.get_z(), 5u);
}

TEST_F(TileidTest, GetValue)
{
    tileid tile(10u, 20u, 3u);
    tileid::value_type val = tile.get_value();
    EXPECT_NE(val, 0u);
    tileid tile2(val);
    EXPECT_EQ(tile2.get_x(), 10u);
    EXPECT_EQ(tile2.get_y(), 20u);
    EXPECT_EQ(tile2.get_z(), 3u);
}

// Empty test
TEST_F(TileidTest, EmptyDefault)
{
    tileid tile;
    EXPECT_TRUE(tile.empty());
}

TEST_F(TileidTest, EmptyNonZeroValue)
{
    tileid tile(1u, 0u, 0u);
    EXPECT_FALSE(tile.empty());
}

// Comparison tests
TEST_F(TileidTest, EqualsIdentical)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    EXPECT_EQ(tile1, tile2);
    EXPECT_TRUE(tile1 == tile2);
}

TEST_F(TileidTest, NotEqualsDifferentX)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(6u, 10u, 3u);
    EXPECT_NE(tile1, tile2);
    EXPECT_FALSE(tile1 == tile2);
}

TEST_F(TileidTest, NotEqualsDifferentY)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 11u, 3u);
    EXPECT_NE(tile1, tile2);
}

TEST_F(TileidTest, NotEqualsDifferentZ)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 4u);
    EXPECT_NE(tile1, tile2);
}

// Three-way comparison tests
TEST_F(TileidTest, ThreeWayComparisonLess)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(6u, 10u, 3u);
    EXPECT_TRUE(tile1 < tile2);
}

TEST_F(TileidTest, ThreeWayComparisonGreater)
{
    tileid tile1(6u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    EXPECT_TRUE(tile1 > tile2);
}

TEST_F(TileidTest, ThreeWayComparisonLessOrEqual)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    EXPECT_TRUE(tile1 <= tile2);
}

TEST_F(TileidTest, ThreeWayComparisonGreaterOrEqual)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    EXPECT_TRUE(tile1 >= tile2);
}

// Parent tests
TEST_F(TileidTest, ParentFromZ1)
{
    tileid tile(5u, 10u, 1u);
    tileid parent = tile.parent();
    EXPECT_EQ(parent.get_x(), 2u); // 5 >> 1 = 2
    EXPECT_EQ(parent.get_y(), 5u); // 10 >> 1 = 5
    EXPECT_EQ(parent.get_z(), 0u);
}

TEST_F(TileidTest, ParentFromZ5)
{
    tileid tile(16u, 24u, 5u);
    tileid parent = tile.parent();
    EXPECT_EQ(parent.get_x(), 8u); // 16 >> 1 = 8
    EXPECT_EQ(parent.get_y(), 12u); // 24 >> 1 = 12
    EXPECT_EQ(parent.get_z(), 4u);
}

TEST_F(TileidTest, ParentFromZ0)
{
    tileid tile(0u, 0u, 0u);
    tileid parent = tile.parent();
    EXPECT_TRUE(parent.empty());
}

TEST_F(TileidTest, ParentChain)
{
    tileid tile(100u, 150u, 5u);
    tileid p1 = tile.parent();
    tileid p2 = p1.parent();
    tileid p3 = p2.parent();
    tileid p4 = p3.parent();
    tileid p5 = p4.parent();
    tileid p6 = p5.parent();
    EXPECT_EQ(p1.get_z(), 4u);
    EXPECT_EQ(p2.get_z(), 3u);
    EXPECT_EQ(p3.get_z(), 2u);
    EXPECT_EQ(p4.get_z(), 1u);
    EXPECT_EQ(p5.get_z(), 0u);
    EXPECT_TRUE(p6.empty());
}

// get_part tests (child quadrants)
TEST_F(TileidTest, GetPartQuadrant1)
{
    tileid tile(5u, 10u, 2u);
    tileid child = tile.get_part(1);
    EXPECT_EQ(child.get_x(), 10u); // 5 * 2 = 10
    EXPECT_EQ(child.get_y(), 21u); // 10 * 2 + 1 = 21
    EXPECT_EQ(child.get_z(), 3u);
}

TEST_F(TileidTest, GetPartQuadrant2)
{
    tileid tile(5u, 10u, 2u);
    tileid child = tile.get_part(2);
    EXPECT_EQ(child.get_x(), 11u); // 5 * 2 + 1 = 11
    EXPECT_EQ(child.get_y(), 21u); // 10 * 2 + 1 = 21
    EXPECT_EQ(child.get_z(), 3u);
}

TEST_F(TileidTest, GetPartQuadrant3)
{
    tileid tile(5u, 10u, 2u);
    tileid child = tile.get_part(3);
    EXPECT_EQ(child.get_x(), 10u); // 5 * 2 = 10
    EXPECT_EQ(child.get_y(), 20u); // 10 * 2 = 20
    EXPECT_EQ(child.get_z(), 3u);
}

TEST_F(TileidTest, GetPartQuadrant4)
{
    tileid tile(5u, 10u, 2u);
    tileid child = tile.get_part(4);
    EXPECT_EQ(child.get_x(), 11u); // 5 * 2 + 1 = 11
    EXPECT_EQ(child.get_y(), 20u); // 10 * 2 = 20
    EXPECT_EQ(child.get_z(), 3u);
}

TEST_F(TileidTest, GetPartInvalidQuadrant)
{
    tileid tile(5u, 10u, 2u);
    tileid child = tile.get_part(5);
    EXPECT_TRUE(child.empty());

    child = tile.get_part(0);
    EXPECT_TRUE(child.empty());

    child = tile.get_part(-1);
    EXPECT_TRUE(child.empty());
}

TEST_F(TileidTest, GetPartAtMaxLevel)
{
    uint8_t max_z = static_cast<uint8_t>(tileid::max_level());
    tileid tile(100u, 200u, max_z);
    tileid child = tile.get_part(1);
    EXPECT_EQ(child.get_value(), tile.get_value());
    EXPECT_EQ(child.get_z(), max_z);
}

// Parent-child relationship tests
TEST_F(TileidTest, ParentChildRelationship)
{
    tileid parent(5u, 10u, 3u);
    tileid child1 = parent.get_part(1);
    tileid child2 = parent.get_part(2);
    tileid child3 = parent.get_part(3);
    tileid child4 = parent.get_part(4);

    EXPECT_EQ(child1.parent(), parent);
    EXPECT_EQ(child2.parent(), parent);
    EXPECT_EQ(child3.parent(), parent);
    EXPECT_EQ(child4.parent(), parent);
}

// to_string tests
TEST_F(TileidTest, ToStringZ0)
{
    tileid tile(0u, 0u, 0u);
    EXPECT_EQ(tile.to_string(), "0/0/0");
}

TEST_F(TileidTest, ToStringZ5)
{
    tileid tile(10u, 20u, 5u);
    EXPECT_EQ(tile.to_string(), "5/10/20");
}

TEST_F(TileidTest, ToStringMaxLevel)
{
    uint8_t max_z = static_cast<uint8_t>(tileid::max_level());
    tileid tile(100u, 200u, max_z);
    std::string str = tile.to_string();
    EXPECT_TRUE(str.find("28/100/200") != std::string::npos);
}

// Hash tests
TEST_F(TileidTest, HashFunction)
{
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    tileid tile3(6u, 10u, 3u);

    std::hash<tileid> hasher;
    EXPECT_EQ(hasher(tile1), hasher(tile2));
    EXPECT_NE(hasher(tile1), hasher(tile3));
}

TEST_F(TileidTest, UnorderedSetInsertion)
{
    std::unordered_set<tileid> tiles;
    tileid tile1(5u, 10u, 3u);
    tileid tile2(5u, 10u, 3u);
    tileid tile3(6u, 10u, 3u);

    tiles.insert(tile1);
    tiles.insert(tile2); // Should not increase size (duplicate)
    tiles.insert(tile3);

    EXPECT_EQ(tiles.size(), 2u);
    EXPECT_TRUE(tiles.count(tile1) > 0);
    EXPECT_TRUE(tiles.count(tile3) > 0);
}

// Bit layout tests
TEST_F(TileidTest, BitShifts)
{
    EXPECT_EQ(tileid::X_SHIFT, 0u);
    EXPECT_EQ(tileid::Y_SHIFT, 28u);
    EXPECT_EQ(tileid::Z_SHIFT, 56u);
}

TEST_F(TileidTest, BitSizes)
{
    EXPECT_EQ(tileid::X_BITS, 28u);
    EXPECT_EQ(tileid::Y_BITS, 28u);
    EXPECT_EQ(tileid::Z_BITS, 5u);
}

TEST_F(TileidTest, MaxLevel)
{
    EXPECT_EQ(tileid::max_level(), 28u);
}

// Edge cases
TEST_F(TileidTest, MaxXAtLevel28)
{
    uint8_t max_z = static_cast<uint8_t>(tileid::max_level());
    uint32_t max_coord = (1u << tileid::X_BITS) - 1u;
    tileid tile(max_coord, 0u, max_z);
    EXPECT_EQ(tile.get_x(), max_coord);
}

TEST_F(TileidTest, MaxYAtLevel28)
{
    uint8_t max_z = static_cast<uint8_t>(tileid::max_level());
    uint32_t max_coord = (1u << tileid::Y_BITS) - 1u;
    tileid tile(0u, max_coord, max_z);
    EXPECT_EQ(tile.get_y(), max_coord);
}

// Constexpr tests (compile-time evaluation)
TEST_F(TileidTest, ConstexprDefaultConstructor)
{
    constexpr tileid tile;
    EXPECT_TRUE(tile.empty());
}

TEST_F(TileidTest, ConstexprConstructorFromComponents)
{
    constexpr tileid tile(5u, 10u, 3u);
    EXPECT_EQ(tile.get_x(), 5u);
    EXPECT_EQ(tile.get_y(), 10u);
    EXPECT_EQ(tile.get_z(), 3u);
}

TEST_F(TileidTest, ConstexprParent)
{
    constexpr tileid tile(8u, 16u, 4u);
    constexpr tileid parent = tile.parent();
    EXPECT_EQ(parent.get_x(), 4u);
    EXPECT_EQ(parent.get_y(), 8u);
    EXPECT_EQ(parent.get_z(), 3u);
}

TEST_F(TileidTest, ConstexprGetPart)
{
    constexpr tileid tile(5u, 10u, 2u);
    constexpr tileid child = tile.get_part(2);
    EXPECT_EQ(child.get_x(), 11u);
    EXPECT_EQ(child.get_y(), 21u);
    EXPECT_EQ(child.get_z(), 3u);
}
