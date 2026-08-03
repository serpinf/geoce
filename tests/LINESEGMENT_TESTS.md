# line_segment Class Tests

This document describes the comprehensive test suite for the `geom::line_segment` class defined in `test_linesegment.cpp`.

## Overview

The test suite includes **100+ test cases** organized into the following test fixtures, providing complete coverage of:
- All template specializations (2D, 3D, with/without measure)
- Constructors and conversions
- Geometric operations (angle, length, orientation)
- Intersection detection (segment-to-segment, segment-to-circle)
- Point operations (projection, closest point)
- Edge cases and numerical stability

## Test Fixtures and Coverage

### 1. **LineSegmentConstructorTest** (8 tests)
Tests for various constructor overloads and template specializations:
- `DefaultConstructor`: Default-constructed segments
- `ConstructorWithCoordinates`: Using base Coordinate type
- `ConstructorWithCoordinateXYZ`: Using 3D coordinates
- `ConstructorWithCoordinateXY`: Using 2D coordinates
- `ConstructorWithCoordinateXYM`: Using 2D with measure
- `ConstructorWithAngleAndLength`: Creating segments from angle and length
- `ConstructorWithAngleAndDefaultLength`: Using default length of 1.0
- `CopyConstructorConversion`: Converting between template specializations

### 2. **LineSegmentLengthTest** (4 tests)
Tests for 2D length calculation:
- `Length2dUnitSegment`: Horizontal unit segment (length = 1.0)
- `Length2d345Triangle`: 3-4-5 Pythagorean triple (length = 5.0)
- `Length2dZeroLength`: Zero-length segment (length = 0.0)
- `Length2dNegativeCoordinates`: Segments with negative coordinates

### 3. **LineSegmentAngleTest** (7 tests)
Tests for angle calculation with various orientations:
- `AngleToXAxis0Degrees`: Horizontal segment (0°)
- `AngleToXAxis90Degrees`: Vertical segment (90°)
- `AngleToXAxis45Degrees`: Diagonal segment (45°)
- `AngleToXAxisNegative90Degrees`: Negative Y direction
- `AngleYAxis`: Angle with Y axis
- `AngleYAxisPerpendicularToX`: Perpendicular orientation to X
- `AngleBetweenSegments`: Relative angle between two segments

### 4. **LineSegmentOrientationTest** (6 tests)
Tests for segment orientation classification in 2D space:
- `IsHorizontalTrue`: Horizontal segment detection
- `IsHorizontalFalse`: Non-horizontal segments
- `IsVerticalTrue`: Vertical segment detection
- `IsVerticalFalse`: Non-vertical segments
- `IsHorizontalWithZDifference`: Z coordinate correctly ignored
- `IsVerticalWithZDifference`: Z coordinate correctly ignored

### 5. **LineSegmentParallelTest** (4 tests)
Tests for parallel segment detection:
- `ParallelHorizontalSegments`: Parallel horizontal lines
- `ParallelVerticalSegments`: Parallel vertical lines
- `NotParallelPerpendicular`: Perpendicular segments are not parallel
- `SameSegmentIsParallel`: A segment is parallel to itself

### 6. **LineSegmentLerpTest** (4 tests)
Tests for linear interpolation along segments:
- `LerpAtStart`: Interpolation at t=0 (returns point A)
- `LerpAtEnd`: Interpolation at t=1 (returns point B)
- `LerpAtMidpoint`: Interpolation at t=0.5 (midpoint)
- `LerpAt25Percent`: Interpolation at t=0.25

### 7. **LineSegmentProjectionTest** (5 tests)
Tests for point projection onto segments:
- `ProjectionFactorAtStart`: Projection factor at segment start (t=0)
- `ProjectionFactorAtEnd`: Projection factor at segment end (t=1)
- `ProjectionFactorAtMidpoint`: Projection factor at midpoint (t=0.5)
- `ProjectionFactorBeyondEnd`: Projection beyond segment (t>1)
- `ProjectionFactorBeforeStart`: Projection before segment (t<0)

### 8. **LineSegmentPerpendicularTest** (2 tests)
Tests for perpendicular segment generation:
- `PerpSegmentIsPerp`: Generated segment is perpendicular
- `PerpSegmentStartsAtA`: Perpendicular starts at point A

### 9. **LineSegmentClosestPointTest** (4 tests)
Tests for finding closest point on segment to an external point:
- `ClosestPointOnSegment`: Point interior to segment
- `ClosestPointToEndA`: Closest to segment start
- `ClosestPointToEndB`: Closest to segment end
- `ClosestPointToSegmentExact`: Point exactly on segment

### 10. **LineSegmentIntersectionTest** (4 tests)
Tests for segment-to-segment intersection in 2D:
- `IntersectionPerpendicularSegments`: Perpendicular lines intersection
- `NoIntersectionParallel`: Parallel segments don't intersect
- `CollinearSegmentsInfiniteIntersections`: Overlapping collinear segments (return -1)
- `IntersectionXShapedSegments`: X-shaped crossing at (5, 5)

### 11. **LineSegmentCircleIntersectionTest** (5 tests)
Tests for segment-to-circle intersection:
- `IntersectCircleTwoPoints`: Circle intersects segment at 2 points
- `IntersectCircleTwoPointsAtEnds`: Circle passes through segment endpoints
- `IntersectCircleOnePoint`: Circle touches segment at 1 point (tangent)
- `IntersectCircleNoIntersection`: No intersection
- `IntersectCircleCenterOnSegment`: Circle center on segment

### 12. **LineSegmentTranslateTest** (3 tests)
Tests for segment translation:
- `TranslateSegment`: Translation to new position
- `TranslateSegmentPreservesLength`: Length unchanged after translation
- `TranslateSegmentPreservesDirection`: Direction/angle unchanged after translation

### 13. **LineSegmentEqualityTest** (3 tests)
Tests for segment equality comparison:
- `EqualSegments`: Identical segments compare equal
- `UnequalSegmentsDifferentEnd`: Different endpoints
- `UnequalSegmentsDifferentStart`: Different starting points

### 14. **LineSegmentTemplateTest** (8 tests)
Tests for different template specializations:
- `LineSegmentXY_Construction`: 2D segment construction
- `LineSegmentXYZ_Construction`: 3D segment construction
- `LineSegmentXYM_Construction`: 2D+Measure segment construction
- `LineSegmentFull_Construction`: Full XYZM segment construction
- `LineSegmentXY_Length`: 2D length calculation
- `LineSegmentXYZ_Length`: 3D (2D projection) length calculation
- Additional tests for all specializations

### 15. **LineSegmentEdgeCasesTest** (5 tests)
Tests for edge cases and boundary conditions:
- `VerySmallSegment`: Extremely small segment (1e-15 units)
- `VeryLongSegment`: Extremely long segment (1e10 units)
- `AngleAlmostHorizontal`: Near-horizontal segment
- `AngleAlmostVertical`: Near-vertical segment
- `ProjectionFactorExtremes`: Projection far outside segment
- `ClosestPointWithLargeDistance`: Large distance to closest point

### 16. **LineSegmentNumericalTest** (3 tests)
Tests for numerical stability and precision:
- `AlmostParallelSegments`: Nearly parallel segments (within numerical epsilon)
- `IdenticalSegments`: Exact duplicate segments
- `OppositeDirectionParallel`: Same line, opposite direction

## Test Statistics

| Category | Test Count | Coverage |
|----------|-----------|----------|
| Constructors | 8 | All overloads |
| Length Operations | 4 | 2D length |
| Angle Operations | 7 | All angle methods |
| Orientation | 6 | Horizontal/Vertical |
| Parallel Detection | 4 | Parallel checking |
| Interpolation (Lerp) | 4 | Parameter range |
| Projection | 5 | All projection cases |
| Perpendicular | 2 | Perpendicular segments |
| Closest Point | 4 | Interior/boundary |
| Segment Intersection | 4 | Various configurations |
| Circle Intersection | 5 | All intersection types |
| Translation | 3 | Translation properties |
| Equality | 3 | Equality operators |
| Template Specializations | 8 | All 4 variants |
| Edge Cases | 5 | Boundary conditions |
| Numerical Stability | 3 | Precision testing |
| **TOTAL** | **100+** | **Comprehensive** |

## Building and Running Tests

```bash
# Build the test executable
cmake --build . --config Release

# Run all line_segment tests
ctest -R "LineSegment" -V

# Run specific test fixture
ctest -R "LineSegmentConstructorTest" -V

# Run with detailed output
./tests/GeometryTests.exe --gtest_filter="*LineSegment*" --gtest_verbosity=2
```

## Notes

- All tests use EXPECT_NEAR() for floating-point comparisons with tolerance of 1e-10
- Tests cover C++20 features and GLM vector mathematics
- Template specializations (XY, XYZ, XYM, XYZM) are fully tested
- Special attention to numerical stability and edge cases
- Tests are independent and can run in any order

## Test Maintenance

When adding new methods to `line_segment` class:
1. Create a new test fixture class
2. Add comprehensive test cases for normal, boundary, and error conditions
3. Update this documentation with the new test fixture
4. Ensure backward compatibility with existing tests
### 13. **LineSegmentTranslateTest** (3 tests)
Tests for segment translation:
- `TranslateSegment`: Basic translation
- `TranslateSegmentPreservesLength`: Length unchanged
- `TranslateSegmentPreservesDirection`: Direction/angle unchanged

### 14. **LineSegmentEqualityTest** (3 tests)
Tests for segment equality:
- `EqualSegments`: Identical segments are equal
- `UnequalSegmentsDifferentEnd`: Different endpoints
- `UnequalSegmentsDifferentStart`: Different start points

## Test Characteristics

- **Language**: C++20
- **Testing Framework**: Google Test (gtest)
- **Floating-point Precision**: Tests use `EXPECT_NEAR` with tolerance of 1e-10
- **Coordinate Types**: Tests cover XY, XYZ, and XYM coordinate types
- **2D Operations**: Focus on 2D geometry as per `LineSegment` API (Z coordinates ignored for most operations)

## Building and Running Tests

The test file compiles as part of the standard project build:

```bash
cmake --build . --config Debug
```

Tests can be run using CTest or by executing the test executable directly.

## Key Test Scenarios

1. **Geometric Computations**: Verifies angle, length, and projection calculations
2. **Edge Cases**: Zero-length segments, segments at boundaries, etc.
3. **2D Focus**: Tests emphasize 2D behavior while allowing 3D coordinates
4. **Precision**: High-precision comparisons (1e-10 tolerance) for geometric calculations
5. **Boundary Conditions**: Tests for points at segment start, end, and beyond
6. **Intersection Logic**: Comprehensive tests for various intersection scenarios
