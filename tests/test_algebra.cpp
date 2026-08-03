
#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../src/gisdom/alg/gc_algebra.h"
#include <numbers>

namespace gce
{
static constexpr double EPS = 1e-9;

static constexpr double M_PI_2 = std::numbers::pi / 2;

static constexpr double M_PI_4 = std::numbers::pi / 4;

TEST(FP_Round_Ceil_Floor, RoundAndBase)
{
    EXPECT_NEAR(round2(2.3, 0.5), 2.5, EPS);
    EXPECT_NEAR(round2(2.2, 0.5), 2.0, EPS);
    EXPECT_NEAR(round2_base(2.3, 0.5, 0.1), 0.1 + 0.5 * std::round((2.3 - 0.1) / 0.5), EPS);
}

TEST(FP_Round_Ceil_Floor, CeilAndFloor)
{
    EXPECT_NEAR(ceil2(2.0, 0.5), 2.0, EPS);
    EXPECT_NEAR(ceil2(2.1, 0.5), 2.5, EPS);
    EXPECT_NEAR(floor2(2.0, 0.5), 2.0, EPS);
    EXPECT_NEAR(floor2(2.4, 0.5), 2.0, EPS);
}

TEST(AnglesAndAzimuth, AngleZ_AngleX_Azimuth)
{
    glm::dvec3 v{1.0, 0.0, 1.0};
    // Calculate angle Z: atan2(z, sqrt(x^2+y^2)) => atan2(1,1)=pi/4
    double angZ = angleZ(v);
    EXPECT_NEAR(angZ, M_PI_4, 1e-12);

    glm::dvec2 v2{0.0, 1.0};
    // Calculate angle from X-axis
    double ax = angleX(v2);
    EXPECT_NEAR(ax, M_PI_2, 1e-12);

    // Azimuth is calculated in degrees using atan2(x,y)
    glm::dvec2 azv{1.0, 0.0};
    double az = azimuth(azv);
    EXPECT_NEAR(az, 90.0, 1e-12);
}

TEST(PerpVectorAndAngle, PerpAndAngle)
{
    glm::dvec2 a{1.0, 0.0};
    glm::dvec2 b{0.0, 1.0};
    glm::dvec2 pa = perpVector(a);
    EXPECT_NEAR(pa.x, 0.0, EPS);
    EXPECT_NEAR(pa.y, 1.0, EPS);

    // Calculate angle from b to a using function: atan2(dot(A, perp(base)), dot(A, base))
    // For A=(1,0), base=(0,1): perp(base)=(-1,0); dot(A,perp)= -1; dot(A,base)=0 => atan2(-1,0) = -pi/2
    double ang = angle(a, b);
    EXPECT_NEAR(ang, -M_PI_2, 1e-12);
}

TEST(VectorUtils, DistanceIsZeroEqualsEpsilon)
{
    glm::dvec3 A{1.0, 2.0, 3.0};
    glm::dvec3 B{1.0, 2.0, 3.0 + 1.0e-12};
    glm::dvec3 C{2.0, 3.0, 4.0};
    EXPECT_NEAR(distance2(A, C), 3.0, 1e-16);
    EXPECT_TRUE(isZeroEpsilon(glm::dvec3{0.0, 0.0, 0.0}));
    EXPECT_TRUE(equalsEpsilon(A, B, gce::big_epsilon<double>()));
}

TEST(ComparePos, Ordering)
{
    glm::dvec2 a{0.0, 0.0}, b{1.0, 0.0}, c{0.0, 1.0}, a2{0.0, 0.0};
    EXPECT_LT(ComparePos(a, b), 0);
    EXPECT_GT(ComparePos(b, a), 0);
    EXPECT_LT(ComparePos(a, c), 0);
    EXPECT_EQ(ComparePos(a, a2), 0);
}

TEST(TransformBasis, BasisToMatrix)
{
    // Standard basis: e1=(1,0,0), e2=(0,1,0), e3=(0,0,1), r=(1,2,3)
    glm::dvec3 e1{1, 0, 0}, e2{0, 1, 0}, e3{0, 0, 1}, r{1, 2, 3};
    glm::dmat4 M = transformBasis(e1, e2, e3, r);

    // Verify that the matrix applied to local origin position (0,0,0,1) gives r
    glm::dvec4 origin = M * glm::dvec4(0, 0, 0, 1);
    EXPECT_NEAR(origin.x, r.x, EPS);
    EXPECT_NEAR(origin.y, r.y, EPS);
    EXPECT_NEAR(origin.z, r.z, EPS);

    // Transform local e1 as (1,0,0,1) should give r + e1 (since e1 is stored as a column in the matrix)
    glm::dvec4 t_e1 = M * glm::dvec4(1, 0, 0, 1);
    EXPECT_NEAR(t_e1.x, r.x + e1.x, EPS);
    EXPECT_NEAR(t_e1.y, r.y + e1.y, EPS);
    EXPECT_NEAR(t_e1.z, r.z + e1.z, EPS);
}

TEST(PointsOnLine, Collinearity2D)
{
    glm::dvec2 A{0.0, 0.0}, B{1.0, 1.0}, C{2.0, 2.0};
    EXPECT_TRUE(pointsOnLine(A, B, C));
    glm::dvec2 D{2.0, 2.1};
    EXPECT_FALSE(pointsOnLine(A, B, D));
}

TEST(PointsOnLine, Collinearity3D)
{
    glm::dvec3 A{0.0, 0.0, 0.0}, B{1.0, 1.0, 1.0}, C{2.0, 2.0, 2.0};
    EXPECT_TRUE(pointsOnLine(A, B, C));
    glm::dvec3 D{2.0, 2.1, 2.1};
    EXPECT_FALSE(pointsOnLine(A, B, D));
}
} // namespace gce
