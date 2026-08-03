#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../src/gisdom/geom/Coordinate.h"

namespace geom
{

// ========== CoordinateType Tests ==========
class CoordinateTypeTest : public ::testing::Test {};

TEST_F(CoordinateTypeTest, HasZForXY)
{
    EXPECT_FALSE(hasZ(CoordinateType::XY));
}

TEST_F(CoordinateTypeTest, HasZForXYZ)
{
    EXPECT_TRUE(hasZ(CoordinateType::XYZ));
}

TEST_F(CoordinateTypeTest, HasZForXYM)
{
    EXPECT_FALSE(hasZ(CoordinateType::XYM));
}

TEST_F(CoordinateTypeTest, HasZForXYZM)
{
    EXPECT_TRUE(hasZ(CoordinateType::XYZM));
}

TEST_F(CoordinateTypeTest, HasMForXY)
{
    EXPECT_FALSE(hasM(CoordinateType::XY));
}

TEST_F(CoordinateTypeTest, HasMForXYZ)
{
    EXPECT_FALSE(hasM(CoordinateType::XYZ));
}

TEST_F(CoordinateTypeTest, HasMForXYM)
{
    EXPECT_TRUE(hasM(CoordinateType::XYM));
}

TEST_F(CoordinateTypeTest, HasMForXYZM)
{
    EXPECT_TRUE(hasM(CoordinateType::XYZM));
}

TEST_F(CoordinateTypeTest, DimensionsXY)
{
    EXPECT_EQ(dimensions(CoordinateType::XY), 2);
}

TEST_F(CoordinateTypeTest, DimensionsXYZ)
{
    EXPECT_EQ(dimensions(CoordinateType::XYZ), 3);
}

TEST_F(CoordinateTypeTest, DimensionsXYM)
{
    EXPECT_EQ(dimensions(CoordinateType::XYM), 3);
}

TEST_F(CoordinateTypeTest, DimensionsXYZM)
{
    EXPECT_EQ(dimensions(CoordinateType::XYZM), 4);
}

// ========== CoordinateXY (2D) Tests ==========
class CoordinateXYTest : public ::testing::Test
{
protected:
    CoordinateXY c_zero{{0.0, 0.0}};
    CoordinateXY c_one{{1.0, 1.0}};
    CoordinateXY c_custom{{3.5, 7.2}};
};

TEST_F(CoordinateXYTest, DefaultConstructor)
{
    CoordinateXY c;
    EXPECT_EQ(c.pos.x, 0.0);
    EXPECT_EQ(c.pos.y, 0.0);
}

TEST_F(CoordinateXYTest, ConstructorFromValues)
{
    CoordinateXY c({5.0, 3.0});
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 3.0);
}

TEST_F(CoordinateXYTest, ConstructorFromVector)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXY c(v);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
}

TEST_F(CoordinateXYTest, StaticProperties)
{
    EXPECT_FALSE(CoordinateXY::hasZ());
    EXPECT_FALSE(CoordinateXY::hasM());
    EXPECT_EQ(CoordinateXY::ndims(), 2);
}

TEST_F(CoordinateXYTest, EqualityOperator)
{
    CoordinateXY c1({1.0, 2.0});
    CoordinateXY c2({1.0, 2.0});
    CoordinateXY c3({1.0, 3.0});

    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}

TEST_F(CoordinateXYTest, CompareTo)
{
    CoordinateXY c1({1.0, 1.0});
    CoordinateXY c2({2.0, 1.0});
    CoordinateXY c3({1.0, 2.0});

    // c1 < c2 (X is smaller)
    EXPECT_LT(c1.compareTo(c2), 0);
    // c2 > c1
    EXPECT_GT(c2.compareTo(c1), 0);
    // c1 < c3 (Y is smaller)
    EXPECT_LT(c1.compareTo(c3), 0);
}

TEST_F(CoordinateXYTest, GetNull)
{
    CoordinateXY null = CoordinateXY::getNull();
    EXPECT_TRUE(std::isnan(null.pos.x));
    EXPECT_TRUE(std::isnan(null.pos.y));
}

// ========== CoordinateXYZ (3D) Tests ==========
class CoordinateXYZTest : public ::testing::Test
{
protected:
    CoordinateXYZ c_zero{{0.0, 0.0, 0.0}};
    CoordinateXYZ c_one{{1.0, 1.0, 1.0}};
    CoordinateXYZ c_custom{{2.5, 4.3, 1.8}};
};

TEST_F(CoordinateXYZTest, DefaultConstructor)
{
    CoordinateXYZ c;
    EXPECT_EQ(c.pos.x, 0.0);
    EXPECT_EQ(c.pos.y, 0.0);
    EXPECT_EQ(c.pos.z, 0.0);
}

TEST_F(CoordinateXYZTest, ConstructorFromValues)
{
    CoordinateXYZ c({5.0, 3.0, 7.0});
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 3.0);
    EXPECT_EQ(c.pos.z, 7.0);
}

TEST_F(CoordinateXYZTest, ConstructorFrom2DVector)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYZ c({v, 0.0});
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, DEFAULT_C);
}

TEST_F(CoordinateXYZTest, ConstructorFrom2DVectorWithZ)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYZ c({v, 3.5});
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 3.5);
}

TEST_F(CoordinateXYZTest, ConstructorFrom3DVector)
{
    glm::dvec3 v(4.0, 8.0, 2.0);
    CoordinateXYZ c(v);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 2.0);
}

TEST_F(CoordinateXYZTest, StaticProperties)
{
    EXPECT_TRUE(CoordinateXYZ::hasZ());
    EXPECT_FALSE(CoordinateXYZ::hasM());
    EXPECT_EQ(CoordinateXYZ::ndims(), 3);
}

TEST_F(CoordinateXYZTest, EqualityOperator)
{
    CoordinateXYZ c1({1.0, 2.0, 3.0});
    CoordinateXYZ c2({1.0, 2.0, 3.0});
    CoordinateXYZ c3({1.0, 2.0, 4.0});

    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}

TEST_F(CoordinateXYZTest, ConversionFromXY)
{
    CoordinateXY xy({5.0, 6.0});
    CoordinateXYZ xyz(xy);

    EXPECT_EQ(xyz.pos.x, 5.0);
    EXPECT_EQ(xyz.pos.y, 6.0);
    EXPECT_EQ(xyz.pos.z, DEFAULT_C);
}

TEST_F(CoordinateXYZTest, GetNull)
{
    CoordinateXYZ null = CoordinateXYZ::getNull();
    EXPECT_TRUE(std::isnan(null.pos.x));
    EXPECT_TRUE(std::isnan(null.pos.y));
    EXPECT_TRUE(std::isnan(null.pos.z));
}

// ========== CoordinateXYM (2D with Measure) Tests ==========
class CoordinateXYMTest : public ::testing::Test
{
protected:
    CoordinateXYM c_zero{{0.0, 0.0}, 0.0};
    CoordinateXYM c_one{{1.0, 1.0}, 1.0};
    CoordinateXYM c_custom{{2.5, 4.3}, 5.1};
};

TEST_F(CoordinateXYMTest, DefaultConstructor)
{
    CoordinateXYM c;
    EXPECT_EQ(c.pos.x, 0.0);
    EXPECT_EQ(c.pos.y, 0.0);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYMTest, ConstructorFromValues)
{
    CoordinateXYM c({5.0, 3.0}, 7.5);
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 3.0);
    EXPECT_EQ(c.m, 7.5);
}

TEST_F(CoordinateXYMTest, ConstructorFrom2DVectorWithoutMeasure)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYM c(v);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYMTest, ConstructorFrom2DVectorWithMeasure)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYM c(v, 3.5);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.m, 3.5);
}

TEST_F(CoordinateXYMTest, StaticProperties)
{
    EXPECT_FALSE(CoordinateXYM::hasZ());
    EXPECT_TRUE(CoordinateXYM::hasM());
    EXPECT_EQ(CoordinateXYM::ndims(), 2);
}

TEST_F(CoordinateXYMTest, EqualityOperator)
{
    CoordinateXYM c1({1.0, 2.0}, 3.0);
    CoordinateXYM c2({1.0, 2.0}, 3.0);
    CoordinateXYM c3({1.0, 2.0}, 4.0);
    CoordinateXYM c4({1.0, 3.0}, 3.0);

    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3); // Different measure
    EXPECT_NE(c1, c4); // Different position
}

TEST_F(CoordinateXYMTest, ConversionFromXY)
{
    CoordinateXY xy({5.0, 6.0});
    CoordinateXYM xym(xy);

    EXPECT_EQ(xym.pos.x, 5.0);
    EXPECT_EQ(xym.pos.y, 6.0);
    EXPECT_EQ(xym.m, DEFAULT_M);
}

TEST_F(CoordinateXYMTest, GetNull)
{
    CoordinateXYM null = CoordinateXYM::getNull();
    EXPECT_TRUE(std::isnan(null.pos.x));
    EXPECT_TRUE(std::isnan(null.pos.y));
    EXPECT_TRUE(std::isnan(null.m));
}

// ========== CoordinateXYZM (3D with Measure) Tests ==========
class CoordinateXYZMTest : public ::testing::Test
{
protected:
    CoordinateXYZM c_zero{{0.0, 0.0, 0.0}, 0.0};
    CoordinateXYZM c_one{{1.0, 1.0, 1.0}, 1.0};
    CoordinateXYZM c_custom{{2.5, 4.3, 1.8}, 5.1};
};

TEST_F(CoordinateXYZMTest, DefaultConstructor)
{
    CoordinateXYZM c;
    EXPECT_EQ(c.pos.x, 0.0);
    EXPECT_EQ(c.pos.y, 0.0);
    EXPECT_EQ(c.pos.z, 0.0);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConstructorFromValues)
{
    CoordinateXYZM c({5.0, 3.0, 7.0}, 2.5);
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 3.0);
    EXPECT_EQ(c.pos.z, 7.0);
    EXPECT_EQ(c.m, 2.5);
}

TEST_F(CoordinateXYZMTest, ConstructorFrom2DVector)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYZM c({v, 0.0}, 0.0);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, DEFAULT_C);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConstructorFrom2DVectorWithZ)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYZM c({v, 0.0}, 3.5);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 3.5);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConstructorFrom2DVectorWithZAndM)
{
    glm::dvec2 v(4.0, 8.0);
    CoordinateXYZM c({v, 3.5}, 2.0);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 3.5);
    EXPECT_EQ(c.m, 2.0);
}

TEST_F(CoordinateXYZMTest, ConstructorFrom3DVector)
{
    glm::dvec3 v(4.0, 8.0, 2.0);
    CoordinateXYZM c(v);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 2.0);
    EXPECT_EQ(c.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConstructorFrom3DVectorWithM)
{
    glm::dvec3 v(4.0, 8.0, 2.0);
    CoordinateXYZM c(v, 5.5);
    EXPECT_EQ(c.pos.x, 4.0);
    EXPECT_EQ(c.pos.y, 8.0);
    EXPECT_EQ(c.pos.z, 2.0);
    EXPECT_EQ(c.m, 5.5);
}

TEST_F(CoordinateXYZMTest, StaticProperties)
{
    EXPECT_TRUE(CoordinateXYZM::hasZ());
    EXPECT_TRUE(CoordinateXYZM::hasM());
    EXPECT_EQ(CoordinateXYZM::ndims(), 3);
}

TEST_F(CoordinateXYZMTest, EqualityOperator)
{
    CoordinateXYZM c1({1.0, 2.0, 3.0}, 4.0);
    CoordinateXYZM c2({1.0, 2.0, 3.0}, 4.0);
    CoordinateXYZM c3({1.0, 2.0, 3.0}, 5.0); // Different M
    CoordinateXYZM c4({1.0, 2.0, 4.0}, 4.0); // Different Z

    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
    EXPECT_NE(c1, c4);
}

TEST_F(CoordinateXYZMTest, InequalityOperator)
{
    CoordinateXYZM c1({1.0, 2.0, 3.0}, 4.0);
    CoordinateXYZM c2({1.0, 2.0, 3.0}, 4.0);
    CoordinateXYZM c3({1.0, 2.0, 3.0}, 5.0);

    EXPECT_FALSE(c1 != c2);
    EXPECT_TRUE(c1 != c3);
}

TEST_F(CoordinateXYZMTest, ConversionFromXY)
{
    CoordinateXY xy({5.0, 6.0});
    CoordinateXYZM xyzm(xy);

    EXPECT_EQ(xyzm.pos.x, 5.0);
    EXPECT_EQ(xyzm.pos.y, 6.0);
    EXPECT_EQ(xyzm.pos.z, DEFAULT_C);
    EXPECT_EQ(xyzm.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConversionFromXYZ)
{
    CoordinateXYZ xyz({5.0, 6.0, 7.0});
    CoordinateXYZM xyzm(xyz);

    EXPECT_EQ(xyzm.pos.x, 5.0);
    EXPECT_EQ(xyzm.pos.y, 6.0);
    EXPECT_EQ(xyzm.pos.z, 7.0);
    EXPECT_EQ(xyzm.m, DEFAULT_M);
}

TEST_F(CoordinateXYZMTest, ConversionFromXYM)
{
    CoordinateXYM xym({5.0, 6.0}, 8.0);
    CoordinateXYZM xyzm(xym);

    EXPECT_EQ(xyzm.pos.x, 5.0);
    EXPECT_EQ(xyzm.pos.y, 6.0);
    EXPECT_EQ(xyzm.pos.z, DEFAULT_C);
    EXPECT_EQ(xyzm.m, 8.0);
}

TEST_F(CoordinateXYZMTest, GetNull)
{
    CoordinateXYZM null = CoordinateXYZM::getNull();
    EXPECT_TRUE(std::isnan(null.pos.x));
    EXPECT_TRUE(std::isnan(null.pos.y));
    EXPECT_TRUE(std::isnan(null.pos.z));
    EXPECT_TRUE(std::isnan(null.m));
}

// ========== Coordinate Utility Functions Tests ==========
class CoordinateUtilityTest : public ::testing::Test {};

TEST_F(CoordinateUtilityTest, MixXYZMBasic)
{
    CoordinateXYZM c1({0.0, 0.0, 0.0}, 0.0);
    CoordinateXYZM c2({10.0, 10.0, 10.0}, 10.0);

    CoordinateXYZM mid = mix(c1, c2, 0.5);
    EXPECT_EQ(mid.pos.x, 5.0);
    EXPECT_EQ(mid.pos.y, 5.0);
    EXPECT_EQ(mid.pos.z, 5.0);
    EXPECT_EQ(mid.m, 5.0);
}

TEST_F(CoordinateUtilityTest, MixXYZMStart)
{
    CoordinateXYZM c1({0.0, 0.0, 0.0}, 0.0);
    CoordinateXYZM c2({10.0, 10.0, 10.0}, 10.0);

    CoordinateXYZM result = mix(c1, c2, 0.0);
    EXPECT_EQ(result.pos.x, c1.pos.x);
    EXPECT_EQ(result.pos.y, c1.pos.y);
    EXPECT_EQ(result.pos.z, c1.pos.z);
    EXPECT_EQ(result.m, c1.m);
}

TEST_F(CoordinateUtilityTest, MixXYZMEnd)
{
    CoordinateXYZM c1({0.0, 0.0, 0.0}, 0.0);
    CoordinateXYZM c2({10.0, 10.0, 10.0}, 10.0);

    CoordinateXYZM result = mix(c1, c2, 1.0);
    EXPECT_EQ(result.pos.x, c2.pos.x);
    EXPECT_EQ(result.pos.y, c2.pos.y);
    EXPECT_EQ(result.pos.z, c2.pos.z);
    EXPECT_EQ(result.m, c2.m);
}

TEST_F(CoordinateUtilityTest, Distance2DXYZMBasic)
{
    CoordinateXYZM c1({0.0, 0.0, 10.0}, 5.0);
    CoordinateXYZM c2({3.0, 4.0, 10.0}, 5.0);

    double dist = distance2d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 5.0); // 3-4-5 triangle
}

TEST_F(CoordinateUtilityTest, Distance2DXYZMIgnoresZ)
{
    CoordinateXYZM c1({0.0, 0.0, 100.0}, 5.0);
    CoordinateXYZM c2({3.0, 4.0, 200.0}, 5.0);

    double dist = distance2d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 5.0); // Z difference ignored
}

TEST_F(CoordinateUtilityTest, Distance2DXYZBasic)
{
    CoordinateXYZ c1({0.0, 0.0, 10.0});
    CoordinateXYZ c2({3.0, 4.0, 10.0});

    double dist = distance2d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 5.0); // 3-4-5 triangle
}

TEST_F(CoordinateUtilityTest, Distance2DXYZIgnoresZ)
{
    CoordinateXYZ c1({0.0, 0.0, 100.0});
    CoordinateXYZ c2({3.0, 4.0, 200.0});

    double dist = distance2d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 5.0); // Z difference ignored
}

TEST_F(CoordinateUtilityTest, Distance3DXYZMBasic)
{
    CoordinateXYZM c1({0.0, 0.0, 0.0}, 5.0);
    CoordinateXYZM c2({1.0, 2.0, 2.0}, 5.0);

    double dist = distance3d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 3.0); // 1-2-2 triangle in 3D
}

TEST_F(CoordinateUtilityTest, Distance3DXYZMIgnoresM)
{
    CoordinateXYZM c1({0.0, 0.0, 0.0}, 100.0);
    CoordinateXYZM c2({1.0, 2.0, 2.0}, 200.0);

    double dist = distance3d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 3.0); // M difference ignored
}

TEST_F(CoordinateUtilityTest, Distance3DSamePoint)
{
    CoordinateXYZM c1({1.0, 2.0, 3.0}, 4.0);
    CoordinateXYZM c2({1.0, 2.0, 3.0}, 4.0);

    double dist = distance3d(c1, c2);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

// ========== Cross-Type Conversion Tests ==========
class CoordinateConversionTest : public ::testing::Test {};

TEST_F(CoordinateConversionTest, XYToXYZ)
{
    CoordinateXY xy({1.5, 2.5});
    CoordinateXYZ xyz(xy);

    EXPECT_EQ(xyz.pos.x, xy.pos.x);
    EXPECT_EQ(xyz.pos.y, xy.pos.y);
    EXPECT_EQ(xyz.pos.z, DEFAULT_C);
}

TEST_F(CoordinateConversionTest, XYToXYM)
{
    CoordinateXY xy({1.5, 2.5});
    CoordinateXYM xym(xy);

    EXPECT_EQ(xym.pos.x, xy.pos.x);
    EXPECT_EQ(xym.pos.y, xy.pos.y);
    EXPECT_EQ(xym.m, DEFAULT_M);
}

TEST_F(CoordinateConversionTest, XYToXYZM)
{
    CoordinateXY xy({1.5, 2.5});
    CoordinateXYZM xyzm(xy);

    EXPECT_EQ(xyzm.pos.x, xy.pos.x);
    EXPECT_EQ(xyzm.pos.y, xy.pos.y);
    EXPECT_EQ(xyzm.pos.z, DEFAULT_C);
    EXPECT_EQ(xyzm.m, DEFAULT_M);
}

TEST_F(CoordinateConversionTest, XYZToXYZM)
{
    CoordinateXYZ xyz({1.5, 2.5, 3.5});
    CoordinateXYZM xyzm(xyz);

    EXPECT_EQ(xyzm.pos.x, xyz.pos.x);
    EXPECT_EQ(xyzm.pos.y, xyz.pos.y);
    EXPECT_EQ(xyzm.pos.z, xyz.pos.z);
    EXPECT_EQ(xyzm.m, DEFAULT_M);
}

TEST_F(CoordinateConversionTest, XYMToXYZM)
{
    CoordinateXYM xym({1.5, 2.5}, 4.5);
    CoordinateXYZM xyzm(xym);

    EXPECT_EQ(xyzm.pos.x, xym.pos.x);
    EXPECT_EQ(xyzm.pos.y, xym.pos.y);
    EXPECT_EQ(xyzm.pos.z, DEFAULT_C);
    EXPECT_EQ(xyzm.m, xym.m);
}

// ========== Edge Cases and Special Values Tests ==========
class CoordinateEdgeCasesTest : public ::testing::Test {};

TEST_F(CoordinateEdgeCasesTest, NegativeValues)
{
    CoordinateXYZM c({-1.5, -2.5, -3.5}, -4.5);
    EXPECT_EQ(c.pos.x, -1.5);
    EXPECT_EQ(c.pos.y, -2.5);
    EXPECT_EQ(c.pos.z, -3.5);
    EXPECT_EQ(c.m, -4.5);
}

TEST_F(CoordinateEdgeCasesTest, VeryLargeValues)
{
    double large = 1e100;
    CoordinateXYZM c({large, large, large}, large);
    EXPECT_EQ(c.pos.x, large);
    EXPECT_EQ(c.pos.y, large);
    EXPECT_EQ(c.pos.z, large);
    EXPECT_EQ(c.m, large);
}

TEST_F(CoordinateEdgeCasesTest, VerySmallValues)
{
    double small = 1e-100;
    CoordinateXYZM c({small, small, small}, small);
    EXPECT_EQ(c.pos.x, small);
    EXPECT_EQ(c.pos.y, small);
    EXPECT_EQ(c.pos.z, small);
    EXPECT_EQ(c.m, small);
}

TEST_F(CoordinateEdgeCasesTest, NaNInEquality)
{
    CoordinateXYZM c1 = CoordinateXYZM::getNull();
    CoordinateXYZM c2 = CoordinateXYZM::getNull();

    // NaN != NaN per IEEE 754
    EXPECT_NE(c1, c2);
}

TEST_F(CoordinateEdgeCasesTest, PartialNaNEquality)
{
    CoordinateXYZM c1({1.0, DoubleNotANumber, 3.0}, 4.0);
    CoordinateXYZM c2({1.0, DoubleNotANumber, 3.0}, 4.0);

    // NaN in one component makes overall comparison false
    EXPECT_NE(c1, c2);
}

TEST_F(CoordinateEdgeCasesTest, ZeroDistance)
{
    CoordinateXYZM c1({1.5, 2.5, 3.5}, 4.5);
    CoordinateXYZM c2({1.5, 2.5, 3.5}, 4.5);

    EXPECT_DOUBLE_EQ(distance2d(c1, c2), 0.0);
    EXPECT_DOUBLE_EQ(distance3d(c1, c2), 0.0);
}

} // namespace geom
