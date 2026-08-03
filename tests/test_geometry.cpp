#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <cmath>
#include <numbers>
#include "../src/gisdom/geom/Geometry.h"
#include "../src/gisdom/geom/Point.h"
#include "../src/gisdom/geom/LineString.h"
#include "../src/gisdom/geom/Polygon.h"
#include "../src/gisdom/geom/MPoint.h"
#include "../src/gisdom/geom/CoordSeq.h"

#include <cstdarg>

namespace geom
{

// PI constant for C++20
constexpr double PI = std::numbers::pi;

// ========== Geometry Type Checking Tests ==========
class GeometryTypeCheckTest : public ::testing::Test
{
protected:
    Point point{CoordinateType::XY};
    LineString linestring{CoordinateType::XY};
    Polygon polygon{CoordinateType::XY};
};

TEST_F(GeometryTypeCheckTest, PointIsPoint)
{
    EXPECT_NE(point.isPoint(), nullptr);
    EXPECT_EQ(point.isLineString(), nullptr);
    EXPECT_EQ(point.isPolygon(), nullptr);
}

TEST_F(GeometryTypeCheckTest, PointIsPointConst)
{
    const Point &constPoint = point;
    EXPECT_NE(constPoint.isPoint(), nullptr);
    EXPECT_EQ(constPoint.isLineString(), nullptr);
    EXPECT_EQ(constPoint.isPolygon(), nullptr);
}

TEST_F(GeometryTypeCheckTest, LineStringIsLineString)
{
    EXPECT_EQ(linestring.isPoint(), nullptr);
    EXPECT_NE(linestring.isLineString(), nullptr);
    EXPECT_EQ(linestring.isPolygon(), nullptr);
}

TEST_F(GeometryTypeCheckTest, LineStringIsLineStringConst)
{
    const LineString &constLineString = linestring;
    EXPECT_EQ(constLineString.isPoint(), nullptr);
    EXPECT_NE(constLineString.isLineString(), nullptr);
    EXPECT_EQ(constLineString.isPolygon(), nullptr);
}

TEST_F(GeometryTypeCheckTest, PolygonIsPolygon)
{
    EXPECT_EQ(polygon.isPoint(), nullptr);
    EXPECT_EQ(polygon.isLineString(), nullptr);
    EXPECT_NE(polygon.isPolygon(), nullptr);
}

TEST_F(GeometryTypeCheckTest, PolygonIsPolygonConst)
{
    const Polygon &constPolygon = polygon;
    EXPECT_EQ(constPolygon.isPoint(), nullptr);
    EXPECT_EQ(constPolygon.isLineString(), nullptr);
    EXPECT_NE(constPolygon.isPolygon(), nullptr);
}

// ========== Geometry Coordinate Type Tests ==========
class GeometryCoordinateTypeTest : public ::testing::Test
{};

TEST_F(GeometryCoordinateTypeTest, PointHasZWithXYZ)
{
    Point point(CoordinateType::XYZ);
    EXPECT_TRUE(point.hasZ());
    EXPECT_FALSE(point.hasM());
}

TEST_F(GeometryCoordinateTypeTest, PointHasZWithXY)
{
    Point point(CoordinateType::XY);
    EXPECT_FALSE(point.hasZ());
    EXPECT_FALSE(point.hasM());
}

TEST_F(GeometryCoordinateTypeTest, PointHasMWithXYM)
{
    Point point(CoordinateType::XYM);
    EXPECT_FALSE(point.hasZ());
    EXPECT_TRUE(point.hasM());
}

TEST_F(GeometryCoordinateTypeTest, PointHasZAndMWithXYZM)
{
    Point point(CoordinateType::XYZM);
    EXPECT_TRUE(point.hasZ());
    EXPECT_TRUE(point.hasM());
}

TEST_F(GeometryCoordinateTypeTest, LineStringRetainsCoordinateType)
{
    LineString ls(CoordinateType::XYZ);
    EXPECT_EQ(ls.getCoordinateType(), CoordinateType::XYZ);
}

TEST_F(GeometryCoordinateTypeTest, PolygonRetainsCoordinateType)
{
    Polygon poly(CoordinateType::XYM);
    EXPECT_EQ(poly.getCoordinateType(), CoordinateType::XYM);
}

// ========== Geometry Validation Tests ==========
class GeometryValidationTest : public ::testing::Test
{};

TEST_F(GeometryValidationTest, DefaultPointIsValid)
{
    Point point(CoordinateType::XY);
    EXPECT_TRUE(point.isValid());
}

TEST_F(GeometryValidationTest, DefaultLineStringIsValid)
{
    LineString ls(CoordinateType::XY);
    EXPECT_TRUE(ls.isValid());
}

TEST_F(GeometryValidationTest, DefaultPolygonIsValid)
{
    Polygon poly(CoordinateType::XY);
    EXPECT_TRUE(poly.isValid());
}

TEST_F(GeometryValidationTest, CanMakeValidWhenAlreadyValid)
{
    Point point(CoordinateType::XY);
    EXPECT_TRUE(point.CanMakeValid());
}

// ========== Geometry Clone Tests ==========
class GeometryCloneTest : public ::testing::Test
{};

TEST_F(GeometryCloneTest, PointClone)
{
    Point original(CoordinateType::XY);
    original.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});
    Point *cloned = original.clone();

    EXPECT_NE(cloned, nullptr);
    EXPECT_NE(cloned, &original);
    EXPECT_TRUE(cloned->equals(&original));

    delete cloned;
}

// ========== Geometry Clear Tests ==========
class GeometryClearTest : public ::testing::Test
{};

TEST_F(GeometryClearTest, PointClear)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});
    EXPECT_EQ(point.getNumPoints(), 1);

    point.Clear();
    EXPECT_TRUE(point.isEmpty());
}

// ========== Geometry Movement Tests ==========
class GeometryMovementTest : public ::testing::Test
{};

TEST_F(GeometryMovementTest, PointMove)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    glm::dvec3 offset(3.0, 4.0, 5.0);
    point.Move(offset);

    const auto &coord = point.getCoordinate();
    EXPECT_NEAR(coord.pos.x, 4.0, 1e-10);
    EXPECT_NEAR(coord.pos.y, 6.0, 1e-10);
}

// ========== Geometry Bounding Box Tests ==========
class GeometryBoundingBoxTest : public ::testing::Test
{};

TEST_F(GeometryBoundingBoxTest, PointBbox)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    auto bbox = point.bbox();
    EXPECT_NEAR(bbox.cmin.x, 1.0, 1e-10);
    EXPECT_NEAR(bbox.cmin.y, 2.0, 1e-10);
    EXPECT_NEAR(bbox.cmax.x, 1.0, 1e-10);
    EXPECT_NEAR(bbox.cmax.y, 2.0, 1e-10);
}

// ========== Geometry Base Coordinate Tests ==========
class GeometryBaseCoordTest : public ::testing::Test
{};

TEST_F(GeometryBaseCoordTest, PointBaseCoord)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(5.0, 10.0, 0.0), 0.0});

    auto baseCoord = point.GetBaseCoord();
    EXPECT_NEAR(baseCoord.x, 5.0, 1e-10);
    EXPECT_NEAR(baseCoord.y, 10.0, 1e-10);
}

// ========== Geometry Affine Transformation Tests ==========
class GeometryAffineTest : public ::testing::Test
{};

TEST_F(GeometryAffineTest, PointAffineTranslation)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    glm::dmat4 translation = glm::translate(glm::dmat4(1.0), glm::dvec3(5.0, 10.0, 0.0));
    const auto &result = point.affine(translation);

    EXPECT_EQ(&result, &point);
    const auto &coord = point.getCoordinate();
    EXPECT_NEAR(coord.pos.x, 6.0, 1e-10);
    EXPECT_NEAR(coord.pos.y, 12.0, 1e-10);
}

TEST_F(GeometryAffineTest, PointAffineScale)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    glm::dmat4 scale = glm::scale(glm::dmat4(1.0), glm::dvec3(2.0, 3.0, 1.0));
    point.affine(scale);

    const auto &coord = point.getCoordinate();
    EXPECT_NEAR(coord.pos.x, 2.0, 1e-10);
    EXPECT_NEAR(coord.pos.y, 6.0, 1e-10);
}

// ========== Geometry Rotation Tests ==========
class GeometryRotationTest : public ::testing::Test
{};

TEST_F(GeometryRotationTest, PointRotateAroundBase)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(2.0, 0.0, 0.0), 0.0});

    glm::dvec2 baseCoord(0.0, 0.0);
    double angle = PI / 2.0; // 90 degrees
    point.RotateAroundBase(baseCoord, angle);

    const auto &coord = point.getCoordinate();
    EXPECT_NEAR(coord.pos.x, 0.0, 1e-10);
    EXPECT_NEAR(coord.pos.y, 2.0, 1e-10);
}

TEST_F(GeometryRotationTest, PointRotateAroundCenter)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(3.0, 0.0, 0.0), 0.0});

    glm::dvec2 baseCoord(1.0, 0.0);
    double angle = PI / 2.0; // 90 degrees
    point.RotateAroundBase(baseCoord, angle);

    const auto &coord = point.getCoordinate();
    EXPECT_NEAR(coord.pos.x, 1.0, 1e-10);
    EXPECT_NEAR(coord.pos.y, 2.0, 1e-10);
}

// ========== Geometry Simple Tests ==========
class GeometrySimpleTest : public ::testing::Test
{};

TEST_F(GeometrySimpleTest, PointIsSimple)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});
    EXPECT_TRUE(point.isSimple());
}

TEST_F(GeometrySimpleTest, PolygonIsNotSimple)
{
    Polygon poly(CoordinateType::XY);
    // Polygon is considered a compound geometry, not simple
    EXPECT_FALSE(poly.isSimple());
}

// ========== Geometry Dimension Tests ==========
class GeometryDimensionTest : public ::testing::Test
{};

TEST_F(GeometryDimensionTest, PointDimension)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});
    EXPECT_EQ(point.getDimension(), 0);
}

TEST_F(GeometryDimensionTest, LineStringDimension)
{
    LineString ls(CoordinateType::XY);
    EXPECT_EQ(ls.getDimension(), 1);
}

TEST_F(GeometryDimensionTest, PolygonDimension)
{
    Polygon poly(CoordinateType::XY);
    EXPECT_EQ(poly.getDimension(), 2);
}

// ========== Geometry Empty Tests ==========
class GeometryEmptyTest : public ::testing::Test
{};

TEST_F(GeometryEmptyTest, EmptyPointIsEmpty)
{
    Point point(CoordinateType::XY);
    EXPECT_TRUE(point.isEmpty());
}

TEST_F(GeometryEmptyTest, NonEmptyPointIsNotEmpty)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});
    EXPECT_FALSE(point.isEmpty());
}

TEST_F(GeometryEmptyTest, EmptyLineStringIsEmpty)
{
    LineString ls(CoordinateType::XY);
    EXPECT_TRUE(ls.isEmpty());
}

// ========== Geometry Geometry Type Tests ==========
class GeometryTypeIdTest : public ::testing::Test
{};

TEST_F(GeometryTypeIdTest, PointGeometryTypeId)
{
    Point point(CoordinateType::XY);
    EXPECT_EQ(point.getGeometryTypeId(), gceGeometryType::Point);
}

TEST_F(GeometryTypeIdTest, LineStringGeometryTypeId)
{
    LineString ls(CoordinateType::XY);
    EXPECT_EQ(ls.getGeometryTypeId(), gceGeometryType::LineString);
}

TEST_F(GeometryTypeIdTest, PolygonGeometryTypeId)
{
    Polygon poly(CoordinateType::XY);
    EXPECT_EQ(poly.getGeometryTypeId(), gceGeometryType::Polygon);
}

// ========== Geometry Multi-Geometry Tests ==========
class GeometryMultiGeometryTest : public ::testing::Test
{};

TEST_F(GeometryMultiGeometryTest, MPointIsMGeometry)
{
    MPoint mpoint(CoordinateType::XY);
    EXPECT_NE(mpoint.isMGeometry(), nullptr);
}

TEST_F(GeometryMultiGeometryTest, MPointIsMPoint)
{
    MPoint mpoint(CoordinateType::XY);
    EXPECT_NE(mpoint.isMPoint(), nullptr);
}

TEST_F(GeometryMultiGeometryTest, PointIsNotMGeometry)
{
    Point point(CoordinateType::XY);
    EXPECT_EQ(point.isMGeometry(), nullptr);
}

TEST_F(GeometryMultiGeometryTest, PointIsNotMPoint)
{
    Point point(CoordinateType::XY);
    EXPECT_EQ(point.isMPoint(), nullptr);
}

// ========== Geometry Default Properties Tests ==========
class GeometryDefaultPropertiesTest : public ::testing::Test
{};

TEST_F(GeometryDefaultPropertiesTest, DefaultPointArea)
{
    Point point(CoordinateType::XY);
    EXPECT_NEAR(point.area(), 0.0, 1e-10);
}

TEST_F(GeometryDefaultPropertiesTest, DefaultPointLength)
{
    Point point(CoordinateType::XY);
    EXPECT_NEAR(point.length(), 0.0, 1e-10);
}

TEST_F(GeometryDefaultPropertiesTest, DefaultPointLength3D)
{
    Point point(CoordinateType::XY);
    EXPECT_NEAR(point.length3d(), 0.0, 1e-10);
}

// ========== Geometry Copy Constructor Tests ==========
class GeometryCopyConstructorTest : public ::testing::Test
{};

TEST_F(GeometryCopyConstructorTest, PointCopyConstructor)
{
    Point original(CoordinateType::XYZ);
    original.Create(Coordinate{glm::dvec3(1.0, 2.0, 3.0), 0.0});

    Point copy(original);

    EXPECT_EQ(copy.getCoordinateType(), original.getCoordinateType());
    EXPECT_TRUE(copy.equals(&original));
}

// ========== Geometry Equals Tests ==========
class GeometryEqualsTest : public ::testing::Test
{};

TEST_F(GeometryEqualsTest, PointEqualsItself)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_TRUE(point.equals(&point));
}

TEST_F(GeometryEqualsTest, PointEqualsCopy)
{
    Point point1(CoordinateType::XY);
    point1.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    Point point2(CoordinateType::XY);
    point2.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_TRUE(point1.equals(&point2));
}

TEST_F(GeometryEqualsTest, PointNotEqualsDifferentCoords)
{
    Point point1(CoordinateType::XY);
    point1.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    Point point2(CoordinateType::XY);
    point2.Create(Coordinate{glm::dvec3(3.0, 4.0, 0.0), 0.0});

    EXPECT_FALSE(point1.equals(&point2));
}

// ========== Geometry NumPoints Tests ==========
class GeometryNumPointsTest : public ::testing::Test
{};

TEST_F(GeometryNumPointsTest, PointHasOnePoint)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_EQ(point.getNumPoints(), 1);
}

TEST_F(GeometryNumPointsTest, EmptyPointHasZeroPoints)
{
    Point point(CoordinateType::XY);

    EXPECT_EQ(point.getNumPoints(), 0);
}

// ========== Geometry NumGeometries Tests ==========
class GeometryNumGeometriesTest : public ::testing::Test
{};

TEST_F(GeometryNumGeometriesTest, PointDefaultNumGeometries)
{
    Point point(CoordinateType::XY);

    EXPECT_EQ(point.getNumGeometries(), 1);
}

TEST_F(GeometryNumGeometriesTest, LineStringDefaultNumGeometries)
{
    LineString ls(CoordinateType::XY);

    EXPECT_EQ(ls.getNumGeometries(), 1);
}

TEST_F(GeometryNumGeometriesTest, PolygonDefaultNumGeometries)
{
    Polygon poly(CoordinateType::XY);

    EXPECT_EQ(poly.getNumGeometries(), 1);
}

// ========== Geometry Behavior With Different Coordinate Types ==========
class GeometryCoordinateTypesBehaviorTest : public ::testing::Test
{};

TEST_F(GeometryCoordinateTypesBehaviorTest, PointXYCoordinateType)
{
    Point point(CoordinateType::XY);
    EXPECT_EQ(point.getCoordinateType(), CoordinateType::XY);
    EXPECT_FALSE(point.hasZ());
    EXPECT_FALSE(point.hasM());
}

TEST_F(GeometryCoordinateTypesBehaviorTest, PointXYZCoordinateType)
{
    Point point(CoordinateType::XYZ);
    EXPECT_EQ(point.getCoordinateType(), CoordinateType::XYZ);
    EXPECT_TRUE(point.hasZ());
    EXPECT_FALSE(point.hasM());
}

TEST_F(GeometryCoordinateTypesBehaviorTest, PointXYMCoordinateType)
{
    Point point(CoordinateType::XYM);
    EXPECT_EQ(point.getCoordinateType(), CoordinateType::XYM);
    EXPECT_FALSE(point.hasZ());
    EXPECT_TRUE(point.hasM());
}

TEST_F(GeometryCoordinateTypesBehaviorTest, PointXYZMCoordinateType)
{
    Point point(CoordinateType::XYZM);
    EXPECT_EQ(point.getCoordinateType(), CoordinateType::XYZM);
    EXPECT_TRUE(point.hasZ());
    EXPECT_TRUE(point.hasM());
}

TEST_F(GeometryCoordinateTypesBehaviorTest, LineStringWithXYZ)
{
    LineString ls(CoordinateType::XYZ);
    EXPECT_TRUE(ls.hasZ());
    EXPECT_FALSE(ls.hasM());
}

TEST_F(GeometryCoordinateTypesBehaviorTest, PolygonWithXYZM)
{
    Polygon poly(CoordinateType::XYZM);
    EXPECT_TRUE(poly.hasZ());
    EXPECT_TRUE(poly.hasM());
}

// ========== Geometry Vertex Tests ==========
class GeometryVertexTest : public ::testing::Test
{};

TEST_F(GeometryVertexTest, PointGetGeometryForVertex)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    auto [idx, geom] = point.getGeometryForVertex(0);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(geom, &point);
}

// ========== Geometry Type Cast Tests ==========
class GeometryTypeCastTest : public ::testing::Test
{};

TEST_F(GeometryTypeCastTest, PointCastToConst)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    const Point &constRef = point;
    EXPECT_NE(constRef.isPoint(), nullptr);
}

TEST_F(GeometryTypeCastTest, LineStringCastToConst)
{
    LineString ls(CoordinateType::XY);
    const LineString &constRef = ls;
    EXPECT_NE(constRef.isLineString(), nullptr);
}

TEST_F(GeometryTypeCastTest, PolygonCastToConst)
{
    Polygon poly(CoordinateType::XY);
    const Polygon &constRef = poly;
    EXPECT_NE(constRef.isPolygon(), nullptr);
}

// ========== Geometry Distance Tests ==========
class GeometryDistanceTest : public ::testing::Test
{};

TEST_F(GeometryDistanceTest, PointDistanceFromItself)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(5.0, 10.0, 0.0), 0.0});

    double dist = point.distance(glm::dvec2(5.0, 10.0));
    EXPECT_NEAR(dist, 0.0, 1e-10);
}

TEST_F(GeometryDistanceTest, PointDistanceUnitDistance)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(0.0, 0.0, 0.0), 0.0});

    double dist = point.distance(glm::dvec2(1.0, 0.0));
    EXPECT_NEAR(dist, 1.0, 1e-10);
}

TEST_F(GeometryDistanceTest, PointDistance345Triangle)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(0.0, 0.0, 0.0), 0.0});

    double dist = point.distance(glm::dvec2(3.0, 4.0));
    EXPECT_NEAR(dist, 5.0, 1e-10);
}

// ========== Geometry Make Valid Tests ==========
class GeometryMakeValidTest : public ::testing::Test
{};

TEST_F(GeometryMakeValidTest, PointMakeValid)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_TRUE(point.isValid());
    point.MakeValid();
    EXPECT_TRUE(point.isValid());
}

// ========== Geometry WKT Tests ==========
class GeometryWKTTest : public ::testing::Test
{
public:
    GeometryWKTTest()
    {
        ::initGEOS(LogMessage, LogError);
    }
    ~GeometryWKTTest()
    {
        ::finishGEOS();
    }
    static void LogMessage(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        std::vprintf(format, args);
        va_end(args);
    }
    static void LogError(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        std::vprintf(format, args);
        va_end(args);
    }
};

TEST_F(GeometryWKTTest, PointToWKT)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    std::string wkt = point.toWKT();
    // WKT should not be empty for valid geometry
    EXPECT_FALSE(wkt.empty());
}

// ========== Geometry Type Mismatch Tests ==========
class GeometryTypeMismatchTest : public ::testing::Test
{};

TEST_F(GeometryTypeMismatchTest, PointIsNotLineString)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_EQ(point.isLineString(), nullptr);
}

TEST_F(GeometryTypeMismatchTest, PointIsNotPolygon)
{
    Point point(CoordinateType::XY);
    point.Create(Coordinate{glm::dvec3(1.0, 2.0, 0.0), 0.0});

    EXPECT_EQ(point.isPolygon(), nullptr);
}

TEST_F(GeometryTypeMismatchTest, LineStringIsNotPoint)
{
    LineString ls(CoordinateType::XY);

    EXPECT_EQ(ls.isPoint(), nullptr);
}

TEST_F(GeometryTypeMismatchTest, LineStringIsNotPolygon)
{
    LineString ls(CoordinateType::XY);

    EXPECT_EQ(ls.isPolygon(), nullptr);
}

TEST_F(GeometryTypeMismatchTest, PolygonIsNotPoint)
{
    Polygon poly(CoordinateType::XY);

    EXPECT_EQ(poly.isPoint(), nullptr);
}

TEST_F(GeometryTypeMismatchTest, PolygonIsNotLineString)
{
    Polygon poly(CoordinateType::XY);

    EXPECT_EQ(poly.isLineString(), nullptr);
}

// ========== Geometry Const Correctness Tests ==========
class GeometryConstCorrectnessTest : public ::testing::Test
{};

TEST_F(GeometryConstCorrectnessTest, ConstPointIsPoint)
{
    const Point constPoint(CoordinateType::XY);

    EXPECT_NE(constPoint.isPoint(), nullptr);
    EXPECT_EQ(constPoint.isLineString(), nullptr);
    EXPECT_EQ(constPoint.isPolygon(), nullptr);
}

TEST_F(GeometryConstCorrectnessTest, ConstLineStringIsLineString)
{
    const LineString constLS(CoordinateType::XY);

    EXPECT_EQ(constLS.isPoint(), nullptr);
    EXPECT_NE(constLS.isLineString(), nullptr);
    EXPECT_EQ(constLS.isPolygon(), nullptr);
}

TEST_F(GeometryConstCorrectnessTest, ConstPolygonIsPolygon)
{
    const Polygon constPoly(CoordinateType::XY);

    EXPECT_EQ(constPoly.isPoint(), nullptr);
    EXPECT_EQ(constPoly.isLineString(), nullptr);
    EXPECT_NE(constPoly.isPolygon(), nullptr);
}

} // namespace geom
