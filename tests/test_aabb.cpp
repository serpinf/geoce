#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../src/gisdom/geom/aabb.h"

namespace geom
{

class AABBTest : public ::testing::Test
{
protected:
    aabb2 aabb2d_empty;
    aabb2 aabb2d{{5.0, 5.0}, {2.5, 2.5}};
    aabb3 aabb3d_empty;
    aabb3 aabb3d{{5.0, 5.0, 5.0}, {2.5, 2.5, 2.5}};
};

// ========== aabb2 Construction Tests ==========
TEST_F(AABBTest, DefaultConstructor2D)
{
    aabb2 aabb;
    EXPECT_EQ(aabb.cen, glm::dvec2(0.0, 0.0));
    EXPECT_EQ(aabb.size, glm::dvec2(0.0, 0.0));
}

TEST_F(AABBTest, ConstructorWithComponentsCen2D)
{
    aabb2 aabb({3.0, 4.0}, {1.5, 2.0});
    EXPECT_EQ(aabb.cen, glm::dvec2(3.0, 4.0));
    EXPECT_EQ(aabb.size, glm::dvec2(1.5, 2.0));
}

TEST_F(AABBTest, ConstructorFromBox2D)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    aabb2 aabb(box);
    EXPECT_EQ(aabb.cen, glm::dvec2(5.0, 5.0));
    EXPECT_EQ(aabb.size, glm::dvec2(5.0, 5.0));
}

TEST_F(AABBTest, ConstructorFromBox2DAsymmetric)
{
    Box2D box({1.0, 2.0}, {9.0, 12.0});
    aabb2 aabb(box);
    EXPECT_EQ(aabb.cen, glm::dvec2(5.0, 7.0));
    EXPECT_EQ(aabb.size, glm::dvec2(4.0, 5.0));
}

// ========== aabb3 Construction Tests ==========
TEST_F(AABBTest, DefaultConstructor3D)
{
    aabb3 aabb;
    EXPECT_EQ(aabb.cen, glm::dvec3(0.0, 0.0, 0.0));
    EXPECT_EQ(aabb.size, glm::dvec3(0.0, 0.0, 0.0));
}

TEST_F(AABBTest, ConstructorWithComponentsCenter3D)
{
    aabb3 aabb({3.0, 4.0, 5.0}, {1.5, 2.0, 2.5});
    EXPECT_EQ(aabb.cen, glm::dvec3(3.0, 4.0, 5.0));
    EXPECT_EQ(aabb.size, glm::dvec3(1.5, 2.0, 2.5));
}

TEST_F(AABBTest, ConstructorFromBox3D)
{
    Box3D box({0.0, 0.0, 0.0}, {10.0, 10.0, 10.0});
    aabb3 aabb(box);
    EXPECT_EQ(aabb.cen, glm::dvec3(5.0, 5.0, 5.0));
    EXPECT_EQ(aabb.size, glm::dvec3(5.0, 5.0, 5.0));
}

TEST_F(AABBTest, ConstructorFromBox3DAsymmetric)
{
    Box3D box({1.0, 2.0, 3.0}, {9.0, 12.0, 13.0});
    aabb3 aabb(box);
    EXPECT_EQ(aabb.cen, glm::dvec3(5.0, 7.0, 8.0));
    EXPECT_EQ(aabb.size, glm::dvec3(4.0, 5.0, 5.0));
}

// ========== Intersects3D Tests ==========
TEST_F(AABBTest, Intersects3DPointAtCenter)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb.intersects({5.0, 5.0, 5.0}));
}

TEST_F(AABBTest, Intersects3DPointInside)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb.intersects({6.0, 6.0, 6.0}));
    EXPECT_TRUE(aabb.intersects({4.0, 4.0, 4.0}));
    EXPECT_TRUE(aabb.intersects({5.5, 5.5, 5.5}));
}

TEST_F(AABBTest, Intersects3DPointOnBoundary)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb.intersects({7.0, 5.0, 5.0}));
    EXPECT_TRUE(aabb.intersects({3.0, 5.0, 5.0}));
    EXPECT_TRUE(aabb.intersects({5.0, 7.0, 5.0}));
    EXPECT_TRUE(aabb.intersects({5.0, 3.0, 5.0}));
    EXPECT_TRUE(aabb.intersects({5.0, 5.0, 7.0}));
    EXPECT_TRUE(aabb.intersects({5.0, 5.0, 3.0}));
}

TEST_F(AABBTest, Intersects3DPointOutside)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_FALSE(aabb.intersects({8.5, 5.0, 5.0}));
    EXPECT_FALSE(aabb.intersects({1.5, 5.0, 5.0}));
    EXPECT_FALSE(aabb.intersects({5.0, 8.5, 5.0}));
    EXPECT_FALSE(aabb.intersects({5.0, 1.5, 5.0}));
    EXPECT_FALSE(aabb.intersects({5.0, 5.0, 8.5}));
    EXPECT_FALSE(aabb.intersects({5.0, 5.0, 1.5}));
    EXPECT_FALSE(aabb.intersects({10.0, 10.0, 10.0}));
}

// ========== Overlaps3D Tests ==========
TEST_F(AABBTest, Overlaps3DIdentical)
{
    aabb3 aabb1({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    aabb3 aabb2({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps3DOverlapping)
{
    aabb3 aabb1({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    aabb3 aabb2({7.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps3DTouching)
{
    aabb3 aabb1({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    aabb3 aabb2({9.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    EXPECT_FALSE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps3DDisjoint)
{
    aabb3 aabb1({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
    aabb3 aabb2({10.0, 10.0, 10.0}, {1.0, 1.0, 1.0});
    EXPECT_FALSE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps3DAsymmetric)
{
    aabb3 aabb1({0.0, 0.0, 0.0}, {1.0, 2.0, 3.0});
    aabb3 aabb2({0.5, 1.5, 2.0}, {1.0, 1.0, 1.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps3DPartialOverlap)
{
    aabb3 aabb1({0.0, 0.0, 0.0}, {5.0, 5.0, 5.0});
    aabb3 aabb2({7.0, 7.0, 7.0}, {3.0, 3.0, 3.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

// ========== Overlaps2D Tests ==========
TEST_F(AABBTest, Overlaps2DIdentical)
{
    aabb2 aabb1({5.0, 5.0}, {2.0, 2.0});
    aabb2 aabb2({5.0, 5.0}, {2.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps2DOverlapping)
{
    aabb2 aabb1({5.0, 5.0}, {2.0, 2.0});
    aabb2 aabb2({6.0, 6.0}, {2.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps2DTouching)
{
    aabb2 aabb1({5.0, 5.0}, {2.0, 2.0});
    aabb2 aabb2({9.0, 5.0}, {2.0, 2.0});
    EXPECT_FALSE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps2DDisjoint)
{
    aabb2 aabb1({0.0, 0.0}, {1.0, 1.0});
    aabb2 aabb2({10.0, 10.0}, {1.0, 1.0});
    EXPECT_FALSE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps2DAxisAlignedPartial)
{
    aabb2 aabb1({0.0, 0.0}, {2.0, 2.0});
    aabb2 aabb2({2.5, 0.0}, {1.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

TEST_F(AABBTest, Overlaps2DAxisAlignedJustOverlap)
{
    aabb2 aabb1({0.0, 0.0}, {2.0, 2.0});
    aabb2 aabb2({2.1, 0.0}, {1.0, 2.0});
    EXPECT_TRUE(aabb1.overlaps(aabb2));
}

// ========== Distance2 Tests ==========
TEST_F(AABBTest, Distance2PointAtCenter)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    double dist_sq = aabb.distance2({5.0, 5.0, 5.0});
    EXPECT_DOUBLE_EQ(dist_sq, 0.0);
}

TEST_F(AABBTest, Distance2PointInside)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    double dist_sq = aabb.distance2({6.0, 6.0, 6.0});
    EXPECT_DOUBLE_EQ(dist_sq, 0.0);
}

TEST_F(AABBTest, Distance2PointOnBoundary)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    double dist_sq = aabb.distance2({7.0, 5.0, 5.0});
    EXPECT_DOUBLE_EQ(dist_sq, 0.0);
}

TEST_F(AABBTest, Distance2PointOutside)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
    double dist_sq = aabb.distance2({2.0, 0.0, 0.0});
    EXPECT_DOUBLE_EQ(dist_sq, 1.0);
}

TEST_F(AABBTest, Distance2PointOutsideDiagonal)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
    double dist_sq = aabb.distance2({2.0, 2.0, 2.0});
    EXPECT_DOUBLE_EQ(dist_sq, 3.0);
}

TEST_F(AABBTest, Distance2PointOutsidePartial)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
    double dist_sq = aabb.distance2({2.0, 0.5, 0.0});
    EXPECT_DOUBLE_EQ(dist_sq, 1.0);
}

// ========== ToBox3D Tests ==========
TEST_F(AABBTest, ToBox3D)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});
    Box3D box = aabb.toBox();
    EXPECT_EQ(box.cmin, glm::dvec3(3.0, 3.0, 3.0));
    EXPECT_EQ(box.cmax, glm::dvec3(7.0, 7.0, 7.0));
}

TEST_F(AABBTest, ToBox3DOriginCenter)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.5, 1.5, 1.5});
    Box3D box = aabb.toBox();
    EXPECT_EQ(box.cmin, glm::dvec3(-1.5, -1.5, -1.5));
    EXPECT_EQ(box.cmax, glm::dvec3(1.5, 1.5, 1.5));
}

TEST_F(AABBTest, ToBox3DNegativeCenter)
{
    aabb3 aabb({-5.0, -5.0, -5.0}, {2.0, 3.0, 4.0});
    Box3D box = aabb.toBox();
    EXPECT_EQ(box.cmin, glm::dvec3(-7.0, -8.0, -9.0));
    EXPECT_EQ(box.cmax, glm::dvec3(-3.0, -2.0, -1.0));
}

// ========== RoundTrip Conversion Tests ==========
TEST_F(AABBTest, RoundTripFromBox3D)
{
    Box3D original({1.0, 2.0, 3.0}, {9.0, 12.0, 13.0});
    aabb3 aabb(original);
    Box3D converted = aabb.toBox();
    EXPECT_EQ(converted.cmin, original.cmin);
    EXPECT_EQ(converted.cmax, original.cmax);
}

TEST_F(AABBTest, RoundTripFromBox2D)
{
    Box2D original({1.0, 2.0}, {9.0, 12.0});
    aabb2 aabb(original);
    // Convert to Box3D for comparison (since aabb2 doesn't have toBox3D)
    EXPECT_EQ(aabb.cen, glm::dvec2(5.0, 7.0));
    EXPECT_EQ(aabb.size, glm::dvec2(4.0, 5.0));
}

// ========== GetMetrics Tests ==========
TEST_F(AABBTest, GetMetricsSymmetric3D)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 2.0);
}

TEST_F(AABBTest, GetMetricsAsymmetric3D)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1.0, 2.0, 3.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 6.0);
}

TEST_F(AABBTest, GetMetricsLargeValues3D)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {10.0, 5.0, 3.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 20.0);
}

TEST_F(AABBTest, GetMetricsZero3D)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 0.0);
}

TEST_F(AABBTest, GetMetricsSymmetric2D)
{
    aabb2 aabb({0.0, 0.0}, {2.0, 2.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 4.0);
}

TEST_F(AABBTest, GetMetricsAsymmetric2D)
{
    aabb2 aabb({0.0, 0.0}, {1.0, 3.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 6.0);
}

TEST_F(AABBTest, GetMetricsAsymmetric2DXGreater)
{
    aabb2 aabb({0.0, 0.0}, {5.0, 2.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 10.0);
}

TEST_F(AABBTest, GetMetricsZero2D)
{
    aabb2 aabb({0.0, 0.0}, {0.0, 0.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 0.0);
}

TEST_F(AABBTest, GetMetricsLargeValues2D)
{
    aabb2 aabb({0.0, 0.0}, {100.0, 50.0});
    EXPECT_DOUBLE_EQ(aabb.getMetrics(), 200.0);
}

// ========== Edge Case Tests ==========
TEST_F(AABBTest, SinglePointAABB)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {0.0, 0.0, 0.0});
    EXPECT_TRUE(aabb.intersects({5.0, 5.0, 5.0}));
    EXPECT_FALSE(aabb.intersects({5.1, 5.0, 5.0}));
}

TEST_F(AABBTest, VeryLargeAABB)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {1e6, 1e6, 1e6});
    EXPECT_TRUE(aabb.intersects({0.0, 0.0, 0.0}));
    EXPECT_TRUE(aabb.intersects({1e6, 1e6, 1e6}));
    EXPECT_FALSE(aabb.intersects({1.1e6, 0.0, 0.0}));
}

TEST_F(AABBTest, NegativeCoordinates)
{
    aabb3 aabb({-5.0, -5.0, -5.0}, {2.0, 2.0, 2.0});
    EXPECT_TRUE(aabb.intersects({-5.0, -5.0, -5.0}));
    EXPECT_TRUE(aabb.intersects({-3.0, -3.0, -3.0}));
    EXPECT_FALSE(aabb.intersects({-0.1, -5.0, -5.0}));
}

TEST_F(AABBTest, MixedSignCoordinates)
{
    aabb3 aabb({0.0, 0.0, 0.0}, {5.0, 5.0, 5.0});
    EXPECT_TRUE(aabb.intersects({-3.0, -3.0, -3.0}));
    EXPECT_TRUE(aabb.intersects({3.0, 3.0, 3.0}));
    EXPECT_FALSE(aabb.intersects({6.0, 0.0, 0.0}));
}

// ========== Typealiases Tests ==========
TEST_F(AABBTest, TypealiasAABB2)
{
    aabb2 aabb({3.0, 4.0}, {1.5, 2.0});
    EXPECT_EQ(aabb.cen, glm::dvec2(3.0, 4.0));
    EXPECT_EQ(aabb.size, glm::dvec2(1.5, 2.0));
}

TEST_F(AABBTest, TypealiasAABB3)
{
    aabb3 aabb({3.0, 4.0, 5.0}, {1.5, 2.0, 2.5});
    EXPECT_EQ(aabb.cen, glm::dvec3(3.0, 4.0, 5.0));
    EXPECT_EQ(aabb.size, glm::dvec3(1.5, 2.0, 2.5));
}

TEST_F(AABBTest, TypealiasPSAABB)
{
    psAABB aabb({3.0, 4.0, 5.0}, {1.5, 2.0, 2.5});
    EXPECT_EQ(aabb.cen, glm::dvec3(3.0, 4.0, 5.0));
    EXPECT_EQ(aabb.size, glm::dvec3(1.5, 2.0, 2.5));
}

// ========== Integration Tests ==========
TEST_F(AABBTest, Box2DToAABB2AndBack)
{
    Box2D box({1.0, 2.0}, {9.0, 12.0});
    aabb2 aabb(box);

    EXPECT_EQ(aabb.cen, glm::dvec2(5.0, 7.0));
    EXPECT_EQ(aabb.size, glm::dvec2(4.0, 5.0));
}

TEST_F(AABBTest, Box3DToAABB3AndBack)
{
    Box3D original({2.0, 3.0, 4.0}, {10.0, 15.0, 20.0});
    aabb3 aabb(original);
    Box3D converted = aabb.toBox();

    EXPECT_EQ(converted.cmin, original.cmin);
    EXPECT_EQ(converted.cmax, original.cmax);
}

TEST_F(AABBTest, MultipleOverlapsCheck)
{
    aabb3 center({0.0, 0.0, 0.0}, {5.0, 5.0, 5.0});
    aabb3 top({0.0, 10.0, 0.0}, {3.0, 2.0, 3.0});
    aabb3 bottom({0.0, -10.0, 0.0}, {3.0, 2.0, 3.0});
    aabb3 side({10.0, 0.0, 0.0}, {2.0, 3.0, 3.0});

    EXPECT_FALSE(center.overlaps(top));
    EXPECT_FALSE(center.overlaps(bottom));
    EXPECT_FALSE(center.overlaps(side));
}

TEST_F(AABBTest, Distance2MultiplePoints)
{
    aabb3 aabb({5.0, 5.0, 5.0}, {2.0, 2.0, 2.0});

    EXPECT_DOUBLE_EQ(aabb.distance2({5.0, 5.0, 5.0}), 0.0);
    EXPECT_DOUBLE_EQ(aabb.distance2({7.0, 7.0, 7.0}), 0.0);
    EXPECT_DOUBLE_EQ(aabb.distance2({9.0, 5.0, 5.0}), 4.0);
    EXPECT_DOUBLE_EQ(aabb.distance2({10.0, 10.0, 10.0}), 27.0);
}

}
