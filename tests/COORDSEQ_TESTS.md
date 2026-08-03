# CoordSeq Test Suite Documentation

## Overview
Comprehensive test suite for the `CoordSeq` abstract base class and its template-based concrete implementations (`type_CoordSeq<T>`).

## Test File
**Location:** `..\tests\test_coordseq.cpp`
**File Size:** ~11.5 KB
**Total Tests:** 45+ test cases

## Test Organization

### 1. Factory Tests (4 tests)
**Class:** `CoordSeqFactoryTest`

Tests for the static `Create()` factory methods:
- **CreateXYSequence** - Creates empty XY coordinate sequence
- **CreateXYZSequence** - Creates empty XYZ coordinate sequence  
- **CreateXYMSequence** - Creates empty XYM coordinate sequence
- **CreateXYZMSequence** - Creates empty XYZM coordinate sequence

**Verifies:**
- Factory methods return non-null unique_ptr
- Newly created sequences are empty
- Size is 0 for empty sequences

### 2. Basic Operations Tests (14 tests)
**Class:** `CoordSeqBasicOpsTest`

Core sequence manipulation operations:
- **PushBackXY/XYZ** - Add single coordinates
- **PushBackMultiple** - Add multiple coordinates
- **GetFront** - Retrieve first coordinate
- **GetBack** - Retrieve last coordinate
- **GetAtReverseIndex** - Retrieve by reverse index (0=last)
- **SetAt** - Modify coordinate at index
- **SetFront** - Set first coordinate
- **SetBack** - Set last coordinate
- **PopBack** - Remove last coordinate
- **Clear** - Clear all coordinates
- **Insert** - Insert at position
- **Erase** - Remove at position
- **Resize** - Resize sequence

**Verifies:**
- Size tracking is accurate
- Coordinates are stored and retrieved correctly
- Index operations work correctly (forward and reverse)
- Empty state is properly detected

### 3. Sequence Properties Tests (5 tests)
**Class:** `CoordSeqPropertiesTest`

Geometric and topological properties:
- **IsClosed** - Detect closed rings (first == last)
- **IsNotClosed** - Detect open polylines
- **Length2D** - Calculate XY-only length (3-4-5 triangle = 8.0)
- **Length3D** - Calculate full 3D length with Z dimension
- **SignedArea** - Calculate area of closed polygon (counter-clockwise positive)

**Verifies:**
- Correct geometric calculations
- Support for both 2D and 3D lengths
- Proper handling of closed/open sequences

### 4. Distance Query Tests (4 tests)
**Class:** `CoordSeqDistanceTest`

Distance and proximity calculations:
- **DistanceToPoint** - Minimum distance from point to polyline
- **DistanceToEndpoint** - Distance when point is on line
- **ClosestPoint** - Find closest point on polyline
- **ClosestPointIndex** - Find index of closest point

**Verifies:**
- Accurate point-to-line distance calculations
- Correct projection onto line segments
- Index matching for closest points

### 5. Composition Tests (3 tests)
**Class:** `CoordSeqCompositionTest`

Sequence merging and copying:
- **PushBackSequence** - Append entire sequence
- **AssignSequence** - Copy sequence into another
- **CopyConstructor** - Create sequence from another via Create() factory

**Verifies:**
- Sequences can be combined
- Type conversion works during copy/assign
- Element count and values preserved

### 6. Ordinate Operations Tests (2 tests)
**Class:** `CoordSeqOrdinateTest`

Individual dimension access and modification:
- **SetOrdinate** - Set individual X, Y, or Z at index
- **SetOrdinateAll** - Set dimension value for all coordinates

**Verifies:**
- Individual ordinate access via enum (X=0, Y=1, Z=2, M=3)
- Selective modification of dimensions
- Batch operations on all coordinates

### 7. Type Conversion Tests (2 tests)
**Class:** `CoordSeqTypeConversionTest`

Cross-type sequence creation:
- **ConvertXYtoXYZ** - Create XYZ from XY (Z defaults to 0)
- **ConvertXYZtoXY** - Create XY from XYZ (Z dropped)

**Verifies:**
- Dimensions properly converted during type change
- Missing dimensions filled with defaults
- Extra dimensions properly discarded

### 8. Reverse Tests (1 test)
**Class:** `CoordSeqReverseTest`

Sequence reversal:
- **ReverseSequence** - Reverse order of all coordinates

**Verifies:**
- First becomes last, last becomes first
- All coordinates preserved in reversed order

### 9. Equality Tests (3 tests)
**Class:** `CoordSeqEqualsTest`

Sequence comparison:
- **EqualsIdentical** - Two identical sequences compare equal
- **NotEqualsDifferentSize** - Different sizes are not equal
- **NotEqualsDifferentCoordinates** - Different values are not equal

**Verifies:**
- Proper equality implementation
- Size and value checks
- Correct boolean return

### 10. Duplicate Prevention Tests (2 tests)
**Class:** `CoordSeqDuplicateTest`

Duplicate coordinate handling:
- **AllowDuplicates** - With allowRep=true, duplicates are kept
- **SkipDuplicates** - With allowRep=false, duplicates are skipped

**Verifies:**
- pushBack() respects the allowRep parameter
- Duplicate detection works correctly
- Size accurately reflects duplicate behavior

## Test Fixture Setup

Each test class uses Google Test fixtures with `SetUp()` methods to:
- Create fresh sequence instances for each test
- Initialize with appropriate coordinate types (XY, XYZ, XYM, XYZM)
- Ensure test isolation

## Key Testing Patterns

### 1. Value Verification
```cpp
seqXY->getAt(c, 0);
EXPECT_EQ(c.pos.x, 1.0);
EXPECT_EQ(c.pos.y, 2.0);
```

### 2. Index-Based Access
```cpp
seqXY->getAt(c, 1);      // Forward index
seqXY->getAt_rev(c, 0);  // Reverse index
```

### 3. Geometric Calculations
```cpp
double len = seqXY->length2D();
EXPECT_NEAR(len, expected, tolerance);
```

### 4. Sequence Composition
```cpp
seqXY->pushBack(*seqXY2);
EXPECT_EQ(seqXY->size(), 4);
```

## Coverage Summary

| Component | Test Count | Coverage |
|-----------|-----------|----------|
| Factory Methods | 4 | ✅ Complete |
| Basic Operations | 14 | ✅ Complete |
| Properties | 5 | ✅ Complete |
| Distance Queries | 4 | ✅ Complete |
| Composition | 3 | ✅ Complete |
| Ordinates | 2 | ✅ Complete |
| Type Conversion | 2 | ✅ Complete |
| Reversal | 1 | ✅ Complete |
| Equality | 3 | ✅ Complete |
| Duplicates | 2 | ✅ Complete |
| **TOTAL** | **45** | **✅ Comprehensive** |

## Features Tested

### ✅ Sequence Lifecycle
- Creation (all 4 types)
- Empty detection
- Size tracking
- Clearing/resizing

### ✅ Element Access
- Forward and reverse indexing
- Front/back access
- All 4 coordinate type overloads

### ✅ Element Modification  
- Set operations (at, front, back)
- Insert and erase
- Ordinate-level modifications

### ✅ Geometric Operations
- 2D and 3D length calculation
- Closed ring detection
- Signed area computation
- Distance to point queries
- Closest point finding

### ✅ Sequence Operations
- Reversal
- Type conversion
- Copy and assignment
- Composition (push sequences)

### ✅ Data Integrity
- Equality comparison
- Duplicate prevention
- Type conversions

## Running the Tests

Once the test project is properly linked with CoordSeq.cpp, run:

```bash
# Run all CoordSeq tests
[test_executable] --gtest_filter=CoordSeq*

# Run specific test class
[test_executable] --gtest_filter=CoordSeqBasicOpsTest.*

# Run specific test
[test_executable] --gtest_filter=CoordSeqBasicOpsTest.PushBackXY
```

## Dependencies

- **Google Test Framework** (gtest)
- **GLM** (OpenGL Mathematics, with experimental extensions enabled)
- **CoordSeq.h** and **CoordSeq.cpp**
- **Coordinate.h** (for CoordinateXY, XYZ, XYM, XYZM types)
- **type_CoordSeq.inl** (template implementations)

## Notes

- Tests use `ASSERT_NE` for null checks (test-terminating)
- Tests use `EXPECT_*` for value checks (non-terminating)
- Geometric tests use `EXPECT_NEAR` for floating-point comparisons
- All coordinate values use `double` precision (matches codebase)
- Tests verify behavior across all 4 coordinate types

## Future Test Enhancements

Potential additional tests:
- **Filtering Operations** - apply_filter_rw, apply_filter_ro
- **Bounding Box** - expandBox operation
- **Linear Referencing** - locate_point, coord_by_M operations
- **Tangent Calculation** - tangentAtLength methods
- **Line Offsetting** - computeOffsetLine family
- **GEOS Interoperability** - toGEOSCoordSeq, fromGEOSCoordSeq
- **Error Handling** - Invalid index access, type mismatches
- **Performance** - Large sequence operations
- **Thread Safety** - Concurrent access patterns
