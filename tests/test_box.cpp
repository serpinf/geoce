#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../src/gisdom/geom/Box.h"

namespace geom
{

class BoxTest : public ::testing::Test
{
protected:
    Box2D box2d_empty;
    Box2D box2d{{0.0, 0.0}, {10.0, 10.0}};
    Box3D box3d{{0.0, 0.0, 0.0}, {10.0, 10.0, 10.0}};
};

// Construction tests
TEST_F(BoxTest, DefaultConstructor)
{
    Box2D box;
    EXPECT_TRUE(box.empty());
}

TEST_F(BoxTest, ConstructorWithCorners2D)
{
    Box2D box({1.0, 2.0}, {5.0, 8.0});
    EXPECT_EQ(box.cmin, glm::dvec2(1.0, 2.0));
    EXPECT_EQ(box.cmax, glm::dvec2(5.0, 8.0));
}

TEST_F(BoxTest, ConstructorWithCorners3D)
{
    Box3D box({1.0, 2.0, 3.0}, {5.0, 8.0, 10.0});
    EXPECT_EQ(box.cmin, glm::dvec3(1.0, 2.0, 3.0));
    EXPECT_EQ(box.cmax, glm::dvec3(5.0, 8.0, 10.0));
}

// init_cen_size tests
TEST_F(BoxTest, InitCenterSize2D)
{
    Box2D box;
    box.init_cen_size({5.0, 5.0}, {2.5, 2.5});
    EXPECT_EQ(box.cmin, glm::dvec2(2.5, 2.5));
    EXPECT_EQ(box.cmax, glm::dvec2(7.5, 7.5));
}

TEST_F(BoxTest, InitCenterSize3D)
{
    Box3D box;
    box.init_cen_size({5.0, 5.0, 5.0}, {2.5, 2.5, 2.5});
    EXPECT_EQ(box.cmin, glm::dvec3(2.5, 2.5, 2.5));
    EXPECT_EQ(box.cmax, glm::dvec3(7.5, 7.5, 7.5));
}

// expand tests
TEST_F(BoxTest, ExpandToPoint)
{
    Box2D box({5.0, 5.0}, {10.0, 10.0});
    box.expand({2.0, 7.0});
    EXPECT_EQ(box.cmin, glm::dvec2(2.0, 5.0));
    EXPECT_EQ(box.cmax, glm::dvec2(10.0, 10.0));
}

TEST_F(BoxTest, ExpandToBox)
{
    Box2D box1({0.0, 0.0}, {10.0, 10.0});
    Box2D box2({5.0, 5.0}, {15.0, 15.0});
    box1.expand(box2);
    EXPECT_EQ(box1.cmin, glm::dvec2(0.0, 0.0));
    EXPECT_EQ(box1.cmax, glm::dvec2(15.0, 15.0));
}

// scale tests
TEST_F(BoxTest, ScaleRelativeToCenter)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    box.scale({2.0, 2.0});
    EXPECT_EQ(box.cmin, glm::dvec2(-5.0, -5.0));
    EXPECT_EQ(box.cmax, glm::dvec2(15.0, 15.0));
}

TEST_F(BoxTest, ScaleToRelativeToPoint)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    box.scaleTo({2.0, 2.0}, {5.0, 5.0});
    EXPECT_EQ(box.cmin, glm::dvec2(-5.0, -5.0));
    EXPECT_EQ(box.cmax, glm::dvec2(15.0, 15.0));
}

// offset tests
TEST_F(BoxTest, Offset)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    box.offset({3.0, 4.0});
    EXPECT_EQ(box.cmin, glm::dvec2(3.0, 4.0));
    EXPECT_EQ(box.cmax, glm::dvec2(13.0, 14.0));
}

// clamp tests
TEST_F(BoxTest, ClampInsideBox)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    glm::dvec2 result = box.clamp({5.0, 5.0});
    EXPECT_EQ(result, glm::dvec2(5.0, 5.0));
}

TEST_F(BoxTest, ClampOutsideBox)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    glm::dvec2 result = box.clamp({15.0, -5.0});
    EXPECT_EQ(result, glm::dvec2(10.0, 0.0));
}

// contains tests
TEST_F(BoxTest, ContainsPoint)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    EXPECT_TRUE(box.contains({5.0, 5.0}));
    EXPECT_TRUE(box.contains({0.0, 0.0}));
    EXPECT_TRUE(box.contains({10.0, 10.0}));
    EXPECT_FALSE(box.contains({11.0, 5.0}));
    EXPECT_FALSE(box.contains({-1.0, 5.0}));
}

TEST_F(BoxTest, ContainsBox)
{
    Box2D outer({0.0, 0.0}, {10.0, 10.0});
    Box2D inner({2.0, 2.0}, {8.0, 8.0});
    Box2D overlapping({5.0, 5.0}, {15.0, 15.0});

    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(outer.contains(overlapping));
}

// overlaps tests
TEST_F(BoxTest, OverlapsPoint)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    EXPECT_TRUE(box.overlaps({5.0, 5.0}));
    EXPECT_FALSE(box.overlaps({15.0, 15.0}));
}

TEST_F(BoxTest, OverlapsBox)
{
    Box2D box1({0.0, 0.0}, {10.0, 10.0});
    Box2D overlapping({5.0, 5.0}, {15.0, 15.0});
    Box2D disjoint({15.0, 15.0}, {20.0, 20.0});
    Box2D touching({10.0, 0.0}, {15.0, 10.0});

    EXPECT_TRUE(box1.overlaps(overlapping));
    EXPECT_FALSE(box1.overlaps(disjoint));
    EXPECT_TRUE(box1.overlaps(touching));
}

// equals tests
TEST_F(BoxTest, Equals)
{
    Box2D box1({0.0, 0.0}, {10.0, 10.0});
    Box2D box2({0.0, 0.0}, {10.0, 10.0});
    Box2D box3({0.0, 0.0}, {10.0, 11.0});

    EXPECT_TRUE(box1.equals(box2));
    EXPECT_FALSE(box1.equals(box3));
}

// empty tests
TEST_F(BoxTest, Empty)
{
    Box2D empty;
    Box2D valid({0.0, 0.0}, {10.0, 10.0});
    Box2D invalid({10.0, 10.0}, {0.0, 0.0});

    EXPECT_TRUE(empty.empty());
    EXPECT_FALSE(valid.empty());
    EXPECT_TRUE(invalid.empty());
}

// Corner getter tests
TEST_F(BoxTest, GetCorners2D)
{
    Box2D box({2.0, 3.0}, {8.0, 9.0});
    EXPECT_EQ(box.getBottomLeft(), glm::dvec2(2.0, 3.0));
    EXPECT_EQ(box.getBottomRight(), glm::dvec2(8.0, 3.0));
    EXPECT_EQ(box.getTopLeft(), glm::dvec2(2.0, 9.0));
    EXPECT_EQ(box.getTopRight(), glm::dvec2(8.0, 9.0));
}

// GetCenter tests
TEST_F(BoxTest, GetCenter2D)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    EXPECT_EQ(box.GetCenter(), glm::dvec2(5.0, 5.0));
}

TEST_F(BoxTest, GetCenter3D)
{
    Box3D box({0.0, 0.0, 0.0}, {10.0, 10.0, 10.0});
    EXPECT_EQ(box.GetCenter(), glm::dvec3(5.0, 5.0, 5.0));
}

// distance tests
TEST_F(BoxTest, DistanceOverlappingBoxes)
{
    Box2D box1({0.0, 0.0}, {10.0, 10.0});
    Box2D box2({5.0, 5.0}, {15.0, 15.0});
    EXPECT_EQ(box1.distance(box2), 0.0);
}

TEST_F(BoxTest, DistanceDisjointBoxes)
{
    Box2D box1({0.0, 0.0}, {5.0, 5.0});
    Box2D box2({10.0, 10.0}, {15.0, 15.0});
    double dist = box1.distance(box2);
    EXPECT_GT(dist, 0.0);
    EXPECT_DOUBLE_EQ(dist, 5.0 * std::sqrt(2.0));
}

// intersection tests
TEST_F(BoxTest, IntersectionOverlappingBoxes)
{
    Box2D box1({0.0, 0.0}, {10.0, 10.0});
    Box2D box2({5.0, 5.0}, {15.0, 15.0});
    Box2D result = box1.intersection(box2);
    EXPECT_EQ(result.cmin, glm::dvec2(5.0, 5.0));
    EXPECT_EQ(result.cmax, glm::dvec2(10.0, 10.0));
}

TEST_F(BoxTest, IntersectionDisjointBoxes)
{
    Box2D box1({0.0, 0.0}, {5.0, 5.0});
    Box2D box2({10.0, 10.0}, {15.0, 15.0});
    Box2D result = box1.intersection(box2);
    EXPECT_TRUE(result.empty());
}

// Size dimension tests
TEST_F(BoxTest, Width)
{
    Box2D box({1.0, 2.0}, {6.0, 8.0});
    EXPECT_DOUBLE_EQ(box.width(), 5.0);
}

TEST_F(BoxTest, Height)
{
    Box2D box({1.0, 2.0}, {6.0, 8.0});
    EXPECT_DOUBLE_EQ(box.height(), 6.0);
}

TEST_F(BoxTest, Depth)
{
    Box3D box({1.0, 2.0, 3.0}, {6.0, 8.0, 10.0});
    EXPECT_DOUBLE_EQ(box.depth(), 7.0);
}

// size tests
TEST_F(BoxTest, Size2D)
{
    Box2D box({1.0, 2.0}, {6.0, 8.0});
    EXPECT_EQ(box.size(), glm::dvec2(5.0, 6.0));
}

TEST_F(BoxTest, Size3D)
{
    Box3D box({1.0, 2.0, 3.0}, {6.0, 8.0, 10.0});
    EXPECT_EQ(box.size(), glm::dvec3(5.0, 6.0, 7.0));
}

// volume tests
TEST_F(BoxTest, Volume2D)
{
    Box2D box({0.0, 0.0}, {4.0, 5.0});
    EXPECT_DOUBLE_EQ(box.volume(), 20.0);
}

TEST_F(BoxTest, Volume3D)
{
    Box3D box({0.0, 0.0, 0.0}, {4.0, 5.0, 6.0});
    EXPECT_DOUBLE_EQ(box.volume(), 120.0);
}

// getPart2D tests
TEST_F(BoxTest, GetPart2DQuadrant1)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    Box2D part = box.getPart2D(1);
    EXPECT_EQ(part.cmin, glm::dvec2(0.0, 5.0));
    EXPECT_EQ(part.cmax, glm::dvec2(5.0, 10.0));
}

TEST_F(BoxTest, GetPart2DQuadrant2)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    Box2D part = box.getPart2D(2);
    EXPECT_EQ(part.cmin, glm::dvec2(5.0, 5.0));
    EXPECT_EQ(part.cmax, glm::dvec2(10.0, 10.0));
}

TEST_F(BoxTest, GetPart2DQuadrant3)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    Box2D part = box.getPart2D(3);
    EXPECT_EQ(part.cmin, glm::dvec2(0.0, 0.0));
    EXPECT_EQ(part.cmax, glm::dvec2(5.0, 5.0));
}

TEST_F(BoxTest, GetPart2DQuadrant4)
{
    Box2D box({0.0, 0.0}, {10.0, 10.0});
    Box2D part = box.getPart2D(4);
    EXPECT_EQ(part.cmin, glm::dvec2(5.0, 0.0));
    EXPECT_EQ(part.cmax, glm::dvec2(10.0, 5.0));
}
}
