#include "pch.h"
/// @file test_linesegment.cpp
/// @brief Comprehensive test suite for the geom::line_segment class
/// @details Tests all line_segment functionality including constructors, geometric operations,
/// intersections, and utility functions with full template specialization coverage.

#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <numbers>
#include "../src/gisdom/geom/LineSegment.h"

namespace geom
{

// PI constant for C++20
constexpr double PI = std::numbers::pi;

// ========== line_segment Constructor Tests ==========
/**
 * @class LineSegmentConstructorTest
 * @brief Test suite for line_segment constructors
 * @details Tests default, copy, coordinate-based, and angle-based constructors
 * for all template specializations (2D, 3D, with/without measure)
 */
class LineSegmentConstructorTest : public ::testing::Test
{
protected:
    Coordinate c0{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate c1{glm::dvec3(1.0, 1.0, 1.0), 1.0};
    Coordinate c2{glm::dvec3(3.0, 4.0, 0.0), 0.0};
    Coordinate c3{glm::dvec3(0.0, 0.0, 5.0), 0.0};
};

TEST_F(LineSegmentConstructorTest, DefaultConstructor)
{
    LineSegment seg;
    // Default constructed segments should have default-constructed Coordinates
    EXPECT_EQ(seg.A.pos.x, 0.0);
    EXPECT_EQ(seg.B.pos.x, 0.0);
}

TEST_F(LineSegmentConstructorTest, CopyConstructorConversion)
{
    CoordinateXY a({0.0, 0.0});
    CoordinateXY b({1.0, 1.0});
    LineSegmentXY seg_xy(a, b);
    LineSegment seg(seg_xy);

    EXPECT_EQ(seg.A.pos.x, 0.0);
    EXPECT_EQ(seg.A.pos.y, 0.0);
    EXPECT_EQ(seg.B.pos.x, 1.0);
    EXPECT_EQ(seg.B.pos.y, 1.0);
}

// ========== line_segment Length Tests ==========
/**
 * @class LineSegmentLengthTest
 * @brief Test suite for line_segment length calculations
 * @details Tests 2D length computation for various segment configurations
 */
class LineSegmentLengthTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentLengthTest, Length2dUnitSegment)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.length2d(), 1.0, 1e-10);
}

TEST_F(LineSegmentLengthTest, Length2d345Triangle)
{
    Coordinate end{glm::dvec3(3.0, 4.0, 100.0), 0.0};  // Z coordinate should be ignored
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.length2d(), 5.0, 1e-10);
}

TEST_F(LineSegmentLengthTest, Length2dZeroLength)
{
    LineSegment seg(origin, origin);
    EXPECT_NEAR(seg.length2d(), 0.0, 1e-10);
}

TEST_F(LineSegmentLengthTest, Length2dNegativeCoordinates)
{
    Coordinate end{glm::dvec3(-3.0, -4.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.length2d(), 5.0, 1e-10);
}

// ========== LineSegment Angle Tests ==========
/**
 * @class LineSegmentAngleTest
 * @brief Test suite for line_segment angle calculations
 * @details Tests angle computation relative to X/Y axes and between segments
 */
class LineSegmentAngleTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentAngleTest, AngleToXAxis0Degrees)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.angle(), 0.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleToXAxis90Degrees)
{
    Coordinate end{glm::dvec3(0.0, 1.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.angle(), PI / 2.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleToXAxis45Degrees)
{
    Coordinate end{glm::dvec3(1.0, 1.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.angle(), PI / 4.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleToXAxisNegative90Degrees)
{
    Coordinate end{glm::dvec3(0.0, -1.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(std::abs(seg.angle()), PI / 2.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleYAxis)
{
    Coordinate end{glm::dvec3(0.0, 1.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(seg.angleY(), 0.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleYAxisPerpendicularToX)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_NEAR(std::abs(seg.angleY()), PI / 2.0, 1e-10);
}

TEST_F(LineSegmentAngleTest, AngleBetweenSegments)
{
    Coordinate end1{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(0.0, 1.0, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(origin, end2);

    double angle = seg1.angle(seg2);
    EXPECT_NEAR(std::abs(angle), PI / 2.0, 1e-10);
}

// ========== LineSegment Orientation Tests ==========
/**
 * @class LineSegmentOrientationTest
 * @brief Test suite for line_segment orientation detection
 * @details Tests horizontal and vertical segment detection in 2D space
 * ignoring Z coordinate values
 */
class LineSegmentOrientationTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentOrientationTest, IsHorizontalTrue)
{
    Coordinate end{glm::dvec3(5.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_TRUE(seg.isHorizontal());
}

TEST_F(LineSegmentOrientationTest, IsHorizontalFalse)
{
    Coordinate end{glm::dvec3(5.0, 1.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_FALSE(seg.isHorizontal());
}

TEST_F(LineSegmentOrientationTest, IsVerticalTrue)
{
    Coordinate end{glm::dvec3(0.0, 5.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_TRUE(seg.isVertical());
}

TEST_F(LineSegmentOrientationTest, IsVerticalFalse)
{
    Coordinate end{glm::dvec3(1.0, 5.0, 0.0), 0.0};
    LineSegment seg(origin, end);
    EXPECT_FALSE(seg.isVertical());
}

TEST_F(LineSegmentOrientationTest, IsHorizontalWithZDifference)
{
    Coordinate start{glm::dvec3(0.0, 5.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(10.0, 5.0, 100.0), 0.0};
    LineSegment seg(start, end);
    EXPECT_TRUE(seg.isHorizontal());
}

TEST_F(LineSegmentOrientationTest, IsVerticalWithZDifference)
{
    Coordinate start{glm::dvec3(5.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(5.0, 10.0, 100.0), 0.0};
    LineSegment seg(start, end);
    EXPECT_TRUE(seg.isVertical());
}

// ========== LineSegment Parallel Tests ==========
/**
 * @class LineSegmentParallelTest
 * @brief Test suite for parallel segment detection
 * @details Tests isParallel() method for various segment configurations
 */
class LineSegmentParallelTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentParallelTest, ParallelHorizontalSegments)
{
    Coordinate end1{glm::dvec3(5.0, 0.0, 0.0), 0.0};
    Coordinate start2{glm::dvec3(0.0, 10.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(5.0, 10.0, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(start2, end2);

    EXPECT_TRUE(seg1.isParallel(seg2));
}

TEST_F(LineSegmentParallelTest, ParallelVerticalSegments)
{
    Coordinate end1{glm::dvec3(0.0, 5.0, 0.0), 0.0};
    Coordinate start2{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(10.0, 5.0, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(start2, end2);

    EXPECT_TRUE(seg1.isParallel(seg2));
}

TEST_F(LineSegmentParallelTest, NotParallelPerpendicular)
{
    Coordinate end1{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(0.0, 1.0, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(origin, end2);

    EXPECT_FALSE(seg1.isParallel(seg2));
}

TEST_F(LineSegmentParallelTest, SameSegmentIsParallel)
{
    Coordinate end{glm::dvec3(5.0, 5.0, 0.0), 0.0};
    LineSegment seg1(origin, end);
    LineSegment seg2(origin, end);

    EXPECT_TRUE(seg1.isParallel(seg2));
}

// ========== LineSegment Lerp Tests ==========
/**
 * @class LineSegmentLerpTest
 * @brief Test suite for linear interpolation along segments
 * @details Tests lerp() method for various parameter values [0, 1]
 */
class LineSegmentLerpTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(10.0, 20.0, 30.0), 0.0};
    LineSegment seg{origin, end};
};

TEST_F(LineSegmentLerpTest, LerpAtStart)
{
    Coordinate result = seg.lerp(0.0);
    EXPECT_NEAR(result.pos.x, 0.0, 1e-10);
    EXPECT_NEAR(result.pos.y, 0.0, 1e-10);
    EXPECT_NEAR(result.pos.z, 0.0, 1e-10);
}

TEST_F(LineSegmentLerpTest, LerpAtEnd)
{
    Coordinate result = seg.lerp(1.0);
    EXPECT_NEAR(result.pos.x, 10.0, 1e-10);
    EXPECT_NEAR(result.pos.y, 20.0, 1e-10);
    EXPECT_NEAR(result.pos.z, 30.0, 1e-10);
}

TEST_F(LineSegmentLerpTest, LerpAtMidpoint)
{
    Coordinate result = seg.lerp(0.5);
    EXPECT_NEAR(result.pos.x, 5.0, 1e-10);
    EXPECT_NEAR(result.pos.y, 10.0, 1e-10);
    EXPECT_NEAR(result.pos.z, 15.0, 1e-10);
}

TEST_F(LineSegmentLerpTest, LerpAt25Percent)
{
    Coordinate result = seg.lerp(0.25);
    EXPECT_NEAR(result.pos.x, 2.5, 1e-10);
    EXPECT_NEAR(result.pos.y, 5.0, 1e-10);
    EXPECT_NEAR(result.pos.z, 7.5, 1e-10);
}

// ========== LineSegment Projection Tests ==========
/**
 * @class LineSegmentProjectionTest
 * @brief Test suite for point projection onto segments
 * @details Tests projectionFactor() method for points inside, outside, and on segments
 */
class LineSegmentProjectionTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    LineSegment seg{origin, end};
};

TEST_F(LineSegmentProjectionTest, ProjectionFactorAtStart)
{
    Coordinate point{glm::dvec3(0.0, 5.0, 0.0), 0.0};
    double factor = seg.projectionFactor(point);
    EXPECT_NEAR(factor, 0.0, 1e-10);
}

TEST_F(LineSegmentProjectionTest, ProjectionFactorAtEnd)
{
    Coordinate point{glm::dvec3(10.0, 5.0, 0.0), 0.0};
    double factor = seg.projectionFactor(point);
    EXPECT_NEAR(factor, 1.0, 1e-10);
}

TEST_F(LineSegmentProjectionTest, ProjectionFactorAtMidpoint)
{
    Coordinate point{glm::dvec3(5.0, 5.0, 0.0), 0.0};
    double factor = seg.projectionFactor(point);
    EXPECT_NEAR(factor, 0.5, 1e-10);
}

TEST_F(LineSegmentProjectionTest, ProjectionFactorBeyondEnd)
{
    Coordinate point{glm::dvec3(15.0, 5.0, 0.0), 0.0};
    double factor = seg.projectionFactor(point);
    EXPECT_GT(factor, 1.0);
}

TEST_F(LineSegmentProjectionTest, ProjectionFactorBeforeStart)
{
    Coordinate point{glm::dvec3(-5.0, 5.0, 0.0), 0.0};
    double factor = seg.projectionFactor(point);
    EXPECT_LT(factor, 0.0);
}

// ========== LineSegment Perpendicular Tests ==========
/**
 * @class LineSegmentPerpendicularTest
 * @brief Test suite for perpendicular segment generation
 * @details Tests perpSegment() method for creating perpendicular segments
 */
class LineSegmentPerpendicularTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentPerpendicularTest, PerpSegmentIsPerp)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);

    LineSegment perp = seg.perpSegment();

    EXPECT_FALSE(seg.isParallel(perp));
}

TEST_F(LineSegmentPerpendicularTest, PerpSegmentStartsAtA)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);

    LineSegment perp = seg.perpSegment();

    EXPECT_EQ(perp.A.pos.x, seg.A.pos.x);
    EXPECT_EQ(perp.A.pos.y, seg.A.pos.y);
}

// ========== LineSegment Closest Point Tests ==========
/**
 * @class LineSegmentClosestPointTest
 * @brief Test suite for finding closest point on segments
 * @details Tests closestPoint() method for interior and boundary cases
 */
class LineSegmentClosestPointTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    LineSegment seg{origin, end};
};

TEST_F(LineSegmentClosestPointTest, ClosestPointOnSegment)
{
    Coordinate point{glm::dvec3(5.0, 10.0, 0.0), 0.0};
    Coordinate closest = seg.closestPoint(point);

    EXPECT_NEAR(closest.pos.x, 5.0, 1e-10);
    EXPECT_NEAR(closest.pos.y, 0.0, 1e-10);
}

TEST_F(LineSegmentClosestPointTest, ClosestPointToEndA)
{
    Coordinate point{glm::dvec3(-5.0, 0.0, 0.0), 0.0};
    Coordinate closest = seg.closestPoint(point);

    EXPECT_NEAR(closest.pos.x, origin.pos.x, 1e-10);
    EXPECT_NEAR(closest.pos.y, origin.pos.y, 1e-10);
}

TEST_F(LineSegmentClosestPointTest, ClosestPointToEndB)
{
    Coordinate point{glm::dvec3(15.0, 0.0, 0.0), 0.0};
    Coordinate closest = seg.closestPoint(point);

    EXPECT_NEAR(closest.pos.x, end.pos.x, 1e-10);
    EXPECT_NEAR(closest.pos.y, end.pos.y, 1e-10);
}

TEST_F(LineSegmentClosestPointTest, ClosestPointToSegmentExact)
{
    Coordinate point{glm::dvec3(3.0, 0.0, 0.0), 0.0};
    Coordinate closest = seg.closestPoint(point);

    EXPECT_NEAR(closest.pos.x, 3.0, 1e-10);
    EXPECT_NEAR(closest.pos.y, 0.0, 1e-10);
}

// ========== LineSegment Intersection Tests ==========
/**
 * @class LineSegmentIntersectionTest
 * @brief Test suite for segment-to-segment intersection
 * @details Tests intersection2d() method for various segment configurations
 */
class LineSegmentIntersectionTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentIntersectionTest, IntersectionPerpendicularSegments)
{
    Coordinate h_end{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    Coordinate v_start{glm::dvec3(5.0, -5.0, 0.0), 0.0};
    Coordinate v_end{glm::dvec3(5.0, 5.0, 0.0), 0.0};

    LineSegment h_seg(origin, h_end);
    LineSegment v_seg(v_start, v_end);

    glm::dvec2 result;
    int intersections = h_seg.intersection2d(result, v_seg);

    EXPECT_EQ(intersections, 1);
    EXPECT_NEAR(result.x, 5.0, 1e-10);
    EXPECT_NEAR(result.y, 0.0, 1e-10);
}

TEST_F(LineSegmentIntersectionTest, NoIntersectionParallel)
{
    Coordinate h_end{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    Coordinate start2{glm::dvec3(0.0, 5.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(10.0, 5.0, 0.0), 0.0};

    LineSegment seg1(origin, h_end);
    LineSegment seg2(start2, end2);

    glm::dvec2 result;
    int intersections = seg1.intersection2d(result, seg2);

    EXPECT_EQ(intersections, 0);
}

TEST_F(LineSegmentIntersectionTest, CollinearSegmentsInfiniteIntersections)
{
    Coordinate end1{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    Coordinate start2{glm::dvec3(2.0, 0.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(8.0, 0.0, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(start2, end2);

    glm::dvec2 result;
    int intersections = seg1.intersection2d(result, seg2);

    EXPECT_EQ(intersections, -1);
}

TEST_F(LineSegmentIntersectionTest, IntersectionXShapedSegments)
{
    Coordinate s1_end{glm::dvec3(10.0, 10.0, 0.0), 0.0};
    Coordinate s2_start{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    Coordinate s2_end{glm::dvec3(0.0, 10.0, 0.0), 0.0};

    LineSegment seg1(origin, s1_end);
    LineSegment seg2(s2_start, s2_end);

    glm::dvec2 result;
    int intersections = seg1.intersection2d(result, seg2);

    EXPECT_EQ(intersections, 1);
    EXPECT_NEAR(result.x, 5.0, 1e-10);
    EXPECT_NEAR(result.y, 5.0, 1e-10);
}

// ========== LineSegment Circle Intersection Tests ==========
/**
 * @class LineSegmentCircleIntersectionTest
 * @brief Test suite for segment-to-circle intersection
 * @details Tests intersectCircle() method for various circle positions and radii
 */
class LineSegmentCircleIntersectionTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(10.0, 0.0, 0.0), 0.0};
    LineSegment seg{origin, end};
    glm::dvec2 center{5.0, 0.0};
    glm::dvec2 center2{5.0, 5.0};
};

TEST_F(LineSegmentCircleIntersectionTest, IntersectCircleTwoPoints)
{
    Coordinate res[2];
    int count = seg.intersectCircle(res, center, 3.0);

    EXPECT_EQ(count, 2);
    // Two intersection points along the segment
    EXPECT_GT(res[0].pos.x, 0.0);
    EXPECT_LT(res[0].pos.x, 10.0);
    EXPECT_GT(res[1].pos.x, 0.0);
    EXPECT_LT(res[1].pos.x, 10.0);
}

TEST_F(LineSegmentCircleIntersectionTest, IntersectCircleTwoPointsAtEnds)
{
    Coordinate res[2];
    int count = seg.intersectCircle(res, center, 5.0);

    EXPECT_EQ(count, 2);
    // Two intersection points along the segment
    EXPECT_NEAR(res[0].pos.x, 0.0, 1e-10);
    EXPECT_NEAR(res[0].pos.y, 0.0, 1e-10);
    EXPECT_NEAR(res[1].pos.x, 10.0, 1e-10);
    EXPECT_NEAR(res[1].pos.y, 0.0, 1e-10);
}

TEST_F(LineSegmentCircleIntersectionTest, IntersectCircleOnePoint)
{
    Coordinate res[2];
    int count = seg.intersectCircle(res, center2, 5.0);

    EXPECT_EQ(count, 1);
    EXPECT_NEAR(res[0].pos.x, 5.0, 1e-10);
    EXPECT_NEAR(res[0].pos.y, 0.0, 1e-10);
}

TEST_F(LineSegmentCircleIntersectionTest, IntersectCircleNoIntersection)
{
    Coordinate res[2];
    int count = seg.intersectCircle(res, center2, 1.0);

    EXPECT_EQ(count, 0);
}

TEST_F(LineSegmentCircleIntersectionTest, IntersectCircleCenterOnSegment)
{
    Coordinate res[2];
    int count = seg.intersectCircle(res, glm::dvec2(2.0, 0.0), 2.0);

    EXPECT_EQ(count, 2);
}

// ========== LineSegment Translate Tests ==========
/**
 * @class LineSegmentTranslateTest
 * @brief Test suite for segment translation
 * @details Tests translateSegment() method for repositioning segments
 */
class LineSegmentTranslateTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(5.0, 5.0, 0.0), 0.0};
    LineSegment seg{origin, end};
};

TEST_F(LineSegmentTranslateTest, TranslateSegment)
{
    Coordinate new_start{glm::dvec3(10.0, 10.0, 0.0), 0.0};
    LineSegment translated = seg.translateSegment(new_start);

    EXPECT_NEAR(translated.A.pos.x, 10.0, 1e-10);
    EXPECT_NEAR(translated.A.pos.y, 10.0, 1e-10);
    EXPECT_NEAR(translated.B.pos.x, 15.0, 1e-10);
    EXPECT_NEAR(translated.B.pos.y, 15.0, 1e-10);
}

TEST_F(LineSegmentTranslateTest, TranslateSegmentPreservesLength)
{
    Coordinate new_start{glm::dvec3(100.0, 100.0, 0.0), 0.0};
    LineSegment translated = seg.translateSegment(new_start);

    EXPECT_NEAR(seg.length2d(), translated.length2d(), 1e-10);
}

TEST_F(LineSegmentTranslateTest, TranslateSegmentPreservesDirection)
{
    Coordinate new_start{glm::dvec3(50.0, 50.0, 0.0), 0.0};
    LineSegment translated = seg.translateSegment(new_start);

    EXPECT_NEAR(seg.angle(), translated.angle(), 1e-10);
}

// ========== LineSegment Equality Tests ==========
/**
 * @class LineSegmentEqualityTest
 * @brief Test suite for segment equality comparison
 * @details Tests operator== for identical and different segments
 */
class LineSegmentEqualityTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
    Coordinate end{glm::dvec3(5.0, 5.0, 0.0), 0.0};
};

TEST_F(LineSegmentEqualityTest, EqualSegments)
{
    LineSegment seg1(origin, end);
    LineSegment seg2(origin, end);

    EXPECT_TRUE(seg1 == seg2);
}

TEST_F(LineSegmentEqualityTest, UnequalSegmentsDifferentEnd)
{
    Coordinate other_end{glm::dvec3(10.0, 10.0, 0.0), 0.0};
    LineSegment seg1(origin, end);
    LineSegment seg2(origin, other_end);

    EXPECT_FALSE(seg1 == seg2);
}

TEST_F(LineSegmentEqualityTest, UnequalSegmentsDifferentStart)
{
    Coordinate other_start{glm::dvec3(1.0, 1.0, 0.0), 0.0};
    LineSegment seg1(origin, end);
    LineSegment seg2(other_start, end);

    EXPECT_FALSE(seg1 == seg2);
}

// ========== LineSegment Template Specialization Tests ==========
/**
 * @class LineSegmentTemplateTest
 * @brief Test suite for different line_segment template specializations
 * @details Tests 2D, 3D, with/without measure variants
 */
class LineSegmentTemplateTest : public ::testing::Test
{
protected:
};

TEST_F(LineSegmentTemplateTest, LineSegmentXY_Construction)
{
    CoordinateXY a({1.0, 2.0});
    CoordinateXY b({3.0, 4.0});
    LineSegmentXY seg(a, b);

    EXPECT_EQ(seg.A.pos.x, 1.0);
    EXPECT_EQ(seg.A.pos.y, 2.0);
    EXPECT_EQ(seg.B.pos.x, 3.0);
    EXPECT_EQ(seg.B.pos.y, 4.0);
}

TEST_F(LineSegmentTemplateTest, LineSegmentXYZ_Construction)
{
    CoordinateXYZ a({1.0, 2.0, 3.0});
    CoordinateXYZ b({4.0, 5.0, 6.0});
    LineSegmentXYZ seg(a, b);

    EXPECT_EQ(seg.A.pos.z, 3.0);
    EXPECT_EQ(seg.B.pos.z, 6.0);
}

TEST_F(LineSegmentTemplateTest, LineSegmentXYM_Construction)
{
    CoordinateXYM a({1.0, 2.0}, 10.0);
    CoordinateXYM b({3.0, 4.0}, 20.0);
    LineSegmentXYM seg(a, b);

    EXPECT_EQ(seg.A.m, 10.0);
    EXPECT_EQ(seg.B.m, 20.0);
}

TEST_F(LineSegmentTemplateTest, LineSegmentFull_Construction)
{
    Coordinate a(glm::dvec3(1.0, 2.0, 3.0), 10.0);
    Coordinate b(glm::dvec3(4.0, 5.0, 6.0), 20.0);
    LineSegment seg(a, b);

    EXPECT_EQ(seg.A.pos.z, 3.0);
    EXPECT_EQ(seg.A.m, 10.0);
    EXPECT_EQ(seg.B.pos.z, 6.0);
    EXPECT_EQ(seg.B.m, 20.0);
}

TEST_F(LineSegmentTemplateTest, LineSegmentXY_Length)
{
    CoordinateXY a({0.0, 0.0});
    CoordinateXY b({3.0, 4.0});
    LineSegmentXY seg(a, b);

    EXPECT_NEAR(seg.length2d(), 5.0, 1e-10);
}

TEST_F(LineSegmentTemplateTest, LineSegmentXYZ_Length)
{
    CoordinateXYZ a({0.0, 0.0, 0.0});
    CoordinateXYZ b({3.0, 4.0, 0.0});
    LineSegmentXYZ seg(a, b);

    EXPECT_NEAR(seg.length2d(), 5.0, 1e-10);
}

// ========== LineSegment Edge Cases Tests ==========
/**
 * @class LineSegmentEdgeCasesTest
 * @brief Test suite for edge cases and boundary conditions
 * @details Tests segments with very small values, special angles, etc.
 */
class LineSegmentEdgeCasesTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentEdgeCasesTest, VerySmallSegment)
{
    Coordinate end{glm::dvec3(1e-15, 1e-15, 0.0), 0.0};
    LineSegment seg(origin, end);

    EXPECT_LT(seg.length2d(), 1e-10);
}

TEST_F(LineSegmentEdgeCasesTest, VeryLongSegment)
{
    Coordinate end{glm::dvec3(1e10, 1e10, 0.0), 0.0};
    LineSegment seg(origin, end);

    EXPECT_GT(seg.length2d(), 1e10);
}

TEST_F(LineSegmentEdgeCasesTest, AngleAlmostHorizontal)
{
    Coordinate end{glm::dvec3(1.0, 1e-10, 0.0), 0.0};
    LineSegment seg(origin, end);

    EXPECT_LT(std::abs(seg.angle()), 0.01);
}

TEST_F(LineSegmentEdgeCasesTest, AngleAlmostVertical)
{
    Coordinate end{glm::dvec3(1e-10, 1.0, 0.0), 0.0};
    LineSegment seg(origin, end);

    EXPECT_NEAR(std::abs(seg.angle()), PI / 2.0, 0.01);
}

TEST_F(LineSegmentEdgeCasesTest, ProjectionFactorExtremes)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);

    Coordinate far_left{glm::dvec3(-1000.0, 0.0, 0.0), 0.0};
    Coordinate far_right{glm::dvec3(1000.0, 0.0, 0.0), 0.0};

    EXPECT_LT(seg.projectionFactor(far_left), 0.0);
    EXPECT_GT(seg.projectionFactor(far_right), 1.0);
}

TEST_F(LineSegmentEdgeCasesTest, ClosestPointWithLargeDistance)
{
    Coordinate end{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    LineSegment seg(origin, end);

    Coordinate far_point{glm::dvec3(0.5, 1e10, 0.0), 0.0};
    Coordinate closest = seg.closestPoint(far_point);

    EXPECT_NEAR(closest.pos.x, 0.5, 1e-10);
    EXPECT_NEAR(closest.pos.y, 0.0, 1e-10);
}

// ========== LineSegment Numerical Stability Tests ==========
/**
 * @class LineSegmentNumericalTest
 * @brief Test suite for numerical stability and precision
 * @details Tests operations with near-parallel and near-identical segments
 */
class LineSegmentNumericalTest : public ::testing::Test
{
protected:
    Coordinate origin{glm::dvec3(0.0, 0.0, 0.0), 0.0};
};

TEST_F(LineSegmentNumericalTest, AlmostParallelSegments)
{
    Coordinate end1{glm::dvec3(1.0, 0.0, 0.0), 0.0};
    Coordinate end2{glm::dvec3(1.0, 1e-8, 0.0), 0.0};

    LineSegment seg1(origin, end1);
    LineSegment seg2(origin, end2);

    // Should not crash with numerical errors
    double angle = seg1.angle(seg2);
    EXPECT_LT(std::abs(angle), 1e-6);
}

TEST_F(LineSegmentNumericalTest, IdenticalSegments)
{
    Coordinate end{glm::dvec3(5.0, 5.0, 0.0), 0.0};
    LineSegment seg1(origin, end);
    LineSegment seg2(origin, end);

    EXPECT_TRUE(seg1 == seg2);
    EXPECT_TRUE(seg1.isParallel(seg2));
}

TEST_F(LineSegmentNumericalTest, OppositeDirectionParallel)
{
    Coordinate end{glm::dvec3(5.0, 5.0, 0.0), 0.0};
    LineSegment seg1(origin, end);
    LineSegment seg2(end, origin);  // Same line, opposite direction

    EXPECT_TRUE(seg1.isParallel(seg2));
}

}; // namespace geom
