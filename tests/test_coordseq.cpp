#include "pch.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../src/gisdom/geom/CoordSeq.h"
//#include "../src/gisdom/geom/type_CoordSeq.inl"

namespace geom
{

// ========== CoordSeq Factory Tests ==========
class CoordSeqFactoryTest : public ::testing::Test {};

TEST_F(CoordSeqFactoryTest, CreateXYSequence)
{
    CoordinateSeq seq(CoordinateType::XY);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seq.hasZ());
    EXPECT_FALSE(seq.hasM());
    EXPECT_EQ(seq.stride(), 2);
}

TEST_F(CoordSeqFactoryTest, CreateXYZSequence)
{
    CoordinateSeq seq(CoordinateType::XYZ);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_TRUE(seq.hasZ());
    EXPECT_FALSE(seq.hasM());
    EXPECT_EQ(seq.stride(), 3);
}

TEST_F(CoordSeqFactoryTest, CreateXYMSequence)
{
    CoordinateSeq seq(CoordinateType::XYM);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_FALSE(seq.hasZ());
    EXPECT_TRUE(seq.hasM());
    EXPECT_EQ(seq.stride(), 3);
}

TEST_F(CoordSeqFactoryTest, CreateXYZMSequence)
{
    CoordinateSeq seq(CoordinateType::XYZM);
    EXPECT_TRUE(seq.empty());
    EXPECT_EQ(seq.size(), 0);
    EXPECT_TRUE(seq.hasZ());
    EXPECT_TRUE(seq.hasM());
    EXPECT_EQ(seq.stride(), 4);
}

// ========== CoordSeq Basic Operations Tests ==========
class CoordSeqBasicOpsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
    CoordinateSeq seqXYZ{CoordinateType::XYZ};
    CoordinateSeq seqXYM{CoordinateType::XYM};
    CoordinateSeq seqXYZM{CoordinateType::XYZM};
};

TEST_F(CoordSeqBasicOpsTest, PushBackXY)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    EXPECT_EQ(seqXY.size(), 1);
    EXPECT_FALSE(seqXY.empty());

    CoordinateXY c;
    seqXY.get(c, 0);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
}

TEST_F(CoordSeqBasicOpsTest, PushBackXYZ)
{
    seqXYZ.push_back(CoordinateXYZM({1.0, 2.0, 3.0}, 0.0));
    EXPECT_EQ(seqXYZ.size(), 1);

    CoordinateXYZ c;
    seqXYZ.get(c, 0);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
    EXPECT_EQ(c.pos.z, 3.0);
}

TEST_F(CoordSeqBasicOpsTest, PushBackMultiple)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({2.0, 2.0, 0.0}, 0.0));

    EXPECT_EQ(seqXY.size(), 3);
}

TEST_F(CoordSeqBasicOpsTest, GetFront)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    CoordinateXY c;
    seqXY.getFront(c);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
}

TEST_F(CoordSeqBasicOpsTest, GetBack)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    CoordinateXY c;
    seqXY.getBack(c);
    EXPECT_EQ(c.pos.x, 3.0);
    EXPECT_EQ(c.pos.y, 4.0);
}

TEST_F(CoordSeqBasicOpsTest, GetAtReverseIndex)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({5.0, 6.0, 0.0}, 0.0));

    CoordinateXY c;
    seqXY.get_rev(c, 0);  // Last element
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 6.0);

    seqXY.get_rev(c, 1);  // Second to last
    EXPECT_EQ(c.pos.x, 3.0);
    EXPECT_EQ(c.pos.y, 4.0);
}

TEST_F(CoordSeqBasicOpsTest, SetAt)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    seqXY.set(CoordinateXYZM({10.0, 20.0, 0.0}, 0.0), 0);

    CoordinateXY c;
    seqXY.get(c, 0);
    EXPECT_EQ(c.pos.x, 10.0);
    EXPECT_EQ(c.pos.y, 20.0);
}

TEST_F(CoordSeqBasicOpsTest, SetFront)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.setFront(CoordinateXYZM({99.0, 88.0, 0.0}, 0.0));

    CoordinateXY c;
    seqXY.getFront(c);
    EXPECT_EQ(c.pos.x, 99.0);
    EXPECT_EQ(c.pos.y, 88.0);
}

TEST_F(CoordSeqBasicOpsTest, SetBack)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.setBack(CoordinateXYZM({99.0, 88.0, 0.0}, 0.0));

    CoordinateXY c;
    seqXY.getBack(c);
    EXPECT_EQ(c.pos.x, 99.0);
    EXPECT_EQ(c.pos.y, 88.0);
}

TEST_F(CoordSeqBasicOpsTest, PopBack)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));
    EXPECT_EQ(seqXY.size(), 2);

    seqXY.pop_back();
    EXPECT_EQ(seqXY.size(), 1);

    CoordinateXY c;
    seqXY.getBack(c);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
}

TEST_F(CoordSeqBasicOpsTest, Clear)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));
    EXPECT_EQ(seqXY.size(), 2);

    seqXY.clear();
    EXPECT_EQ(seqXY.size(), 0);
    EXPECT_TRUE(seqXY.empty());
}

TEST_F(CoordSeqBasicOpsTest, Insert)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({5.0, 6.0, 0.0}, 0.0));

    seqXY.Insert(1, CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));
    EXPECT_EQ(seqXY.size(), 3);

    CoordinateXY c;
    seqXY.get(c, 1);
    EXPECT_EQ(c.pos.x, 3.0);
    EXPECT_EQ(c.pos.y, 4.0);
}

TEST_F(CoordSeqBasicOpsTest, Erase)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({5.0, 6.0, 0.0}, 0.0));

    seqXY.Erase(1);
    EXPECT_EQ(seqXY.size(), 2);

    CoordinateXY c;
    seqXY.get(c, 1);
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 6.0);
}

TEST_F(CoordSeqBasicOpsTest, Resize)
{
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    seqXY.resize(5);
    EXPECT_EQ(seqXY.size(), 5);

    seqXY.resize(2);
    EXPECT_EQ(seqXY.size(), 2);
}

// ========== CoordSeq Sequence Properties Tests ==========
class CoordSeqPropertiesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
    CoordinateSeq seqXYZ{CoordinateType::XYZ};
};

TEST_F(CoordSeqPropertiesTest, IsClosed)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));  // Close the ring

    EXPECT_TRUE(seqXY.isClosed());
}

TEST_F(CoordSeqPropertiesTest, IsNotClosed)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));

    EXPECT_FALSE(seqXY.isClosed());
}

TEST_F(CoordSeqPropertiesTest, Length2D)
{
    // Create a path: (0,0) -> (3,0) -> (3,4)
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    double len = seqXY.length2D();
    EXPECT_DOUBLE_EQ(len, 7.0);  // 3 + 4 = 7
}

TEST_F(CoordSeqPropertiesTest, Length3D)
{
    // Create a path in 3D
    seqXYZ.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXYZ.push_back(CoordinateXYZM({1.0, 0.0, 0.0}, 0.0));
    seqXYZ.push_back(CoordinateXYZM({1.0, 1.0, 1.0}, 0.0));

    double len = seqXYZ.length3D();
    // First segment: 1.0
    // Second segment: sqrt(1^2 + 1^2) = sqrt(2)
    EXPECT_NEAR(len, 1.0 + std::sqrt(2.0), 1e-10);
}

TEST_F(CoordSeqPropertiesTest, SignedArea)
{
    // Create a unit square counter-clockwise: (0,0) -> (1,0) -> (1,1) -> (0,1) -> (0,0)
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({0.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));

    double area = seqXY.signedArea();
    EXPECT_NEAR(std::abs(area), 1.0, 1e-10);
}

// ========== CoordSeq Distance Tests ==========
class CoordSeqDistanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
};

TEST_F(CoordSeqDistanceTest, DistanceToPoint)
{
    // Create a horizontal line from (0,0) to (4,0)
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({4.0, 0.0, 0.0}, 0.0));

    // Distance from (2, 3) to the line should be 3
    double dist = seqXY.distance(glm::dvec2(2.0, 3.0));
    EXPECT_DOUBLE_EQ(dist, 3.0);
}

TEST_F(CoordSeqDistanceTest, DistanceToEndpoint)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 0.0, 0.0}, 0.0));

    // Distance from (0,0) to the line (should touch endpoint)
    double dist = seqXY.distance(glm::dvec2(0.0, 0.0));
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

/*TEST_F(CoordSeqDistanceTest, ClosestPoint)
{
    seqXY.push_back(CoordinateXYZM(0.0, 0.0, 0.0, 0.0));
    seqXY.push_back(CoordinateXYZM(2.0, 0.0, 0.0, 0.0));
    seqXY.push_back(CoordinateXYZM(2.0, 2.0, 0.0, 0.0));

    Coordinate closest = seqXY.closestPoint(Coordinate(1.0, 1.0, 0.0, 0.0));
    EXPECT_NEAR(closest.pos.x, 1.0, 1e-6);
    EXPECT_NEAR(closest.pos.y, 0.0, 1e-6);
}*/

/*TEST_F(CoordSeqDistanceTest, ClosestPointIndex)
{
    seqXY.push_back(CoordinateXYZM(0.0, 0.0, 0.0, 0.0));
    seqXY.push_back(CoordinateXYZM(1.0, 0.0, 0.0, 0.0));
    seqXY.push_back(CoordinateXYZM(2.0, 2.0, 0.0, 0.0));

    size_t idx = seqXY->closestPointIndex(Coordinate(2.0, 2.1, 0.0, 0.0));
    EXPECT_EQ(idx, 2);  // Closest to the third point
}*/

// ========== CoordSeq Sequence Composition Tests ==========
class CoordSeqCompositionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
    CoordinateSeq seqXY2{CoordinateType::XY};
};

TEST_F(CoordSeqCompositionTest, PushBackSequence)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));

    seqXY2.push_back(CoordinateXYZM({2.0, 2.0, 0.0}, 0.0));
    seqXY2.push_back(CoordinateXYZM({3.0, 3.0, 0.0}, 0.0));

    seqXY.append(seqXY2);
    EXPECT_EQ(seqXY.size(), 4);

    CoordinateXY c;
    seqXY.get(c, 2);
    EXPECT_EQ(c.pos.x, 2.0);
    EXPECT_EQ(c.pos.y, 2.0);
}

TEST_F(CoordSeqCompositionTest, AssignSequence)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));

    seqXY2.push_back(CoordinateXYZM({5.0, 5.0, 0.0}, 0.0));
    seqXY2.push_back(CoordinateXYZM({6.0, 6.0, 0.0}, 0.0));
    seqXY2.push_back(CoordinateXYZM({7.0, 7.0, 0.0}, 0.0));

    seqXY.assign(seqXY2);
    EXPECT_EQ(seqXY.size(), 3);

    CoordinateXY c;
    seqXY.get(c, 0);
    EXPECT_EQ(c.pos.x, 5.0);
    EXPECT_EQ(c.pos.y, 5.0);
}

TEST_F(CoordSeqCompositionTest, CopyConstructor)
{
    seqXY.push_back(CoordinateXYZM({0.0, 0.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({2.0, 2.0, 0.0}, 0.0));

    auto seqCopy = CoordinateSeq(CoordinateType::XY, seqXY);
    EXPECT_EQ(seqCopy.size(), 3);

    CoordinateXY c;
    seqCopy.get(c, 1);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 1.0);
}

// ========== CoordSeq Ordinate Operations Tests ==========
class CoordSeqOrdinateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXYZ{CoordinateType::XYZ};
};

/*TEST_F(CoordSeqOrdinateTest, SetOrdinate)
{
    seqXYZ.push_back(CoordinateXYZM(1.0, 2.0, 3.0, 0.0));

    seqXYZ.setOrdinate(0, OrdinateIndex::X, 10.0);
    seqXYZ->setOrdinate(0, OrdinateIndex::Y, 20.0);
    seqXYZ->setOrdinate(0, OrdinateIndex::Z, 30.0);

    CoordinateXYZ c;
    seqXYZ->getAt(c, 0);
    EXPECT_EQ(c.pos.x, 10.0);
    EXPECT_EQ(c.pos.y, 20.0);
    EXPECT_EQ(c.pos.z, 30.0);
}*/

/*TEST_F(CoordSeqOrdinateTest, SetOrdinateAll)
{
    seqXYZ.push_back(CoordinateXYZM(1.0, 2.0, 3.0, 0.0));
    seqXYZ.push_back(CoordinateXYZM(4.0, 5.0, 6.0, 0.0));
    seqXYZ.push_back(CoordinateXYZM(7.0, 8.0, 9.0, 0.0));

    seqXYZ.setOrdinate_all(OrdinateIndex::Z, 0.0);

    CoordinateXYZ c;
    for (size_t i = 0; i < seqXYZ->size(); ++i)
    {
        seqXYZ->getAt(c, i);
        EXPECT_EQ(c.pos.z, 0.0);
    }
}*/

// ========== CoordSeq Type Conversion Tests ==========
class CoordSeqTypeConversionTest : public ::testing::Test {};

TEST_F(CoordSeqTypeConversionTest, ConvertXYtoXYZ)
{
    auto seqXY = CoordinateSeq(CoordinateType::XY);
    seqXY.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    auto seqXYZ = CoordinateSeq(CoordinateType::XYZ, seqXY);
    EXPECT_EQ(seqXYZ.size(), 2);

    CoordinateXYZ c;
    seqXYZ.get(c, 0);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
    EXPECT_EQ(c.pos.z, 0.0);  // Should default to 0
}

TEST_F(CoordSeqTypeConversionTest, ConvertXYZtoXY)
{
    auto seqXYZ = CoordinateSeq(CoordinateType::XYZ);
    seqXYZ.push_back(CoordinateXYZM({1.0, 2.0, 3.0}, 0.0));
    seqXYZ.push_back(CoordinateXYZM({4.0, 5.0, 6.0}, 0.0));

    auto seqXY = CoordinateSeq(CoordinateType::XY, seqXYZ);
    EXPECT_EQ(seqXY.size(), 2);

    CoordinateXY c;
    seqXY.get(c, 0);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 2.0);
}

// ========== CoordSeq Reverse Tests ==========
class CoordSeqReverseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
};

TEST_F(CoordSeqReverseTest, ReverseSequence)
{
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({2.0, 2.0, 0.0}, 0.0));
    seqXY.push_back(CoordinateXYZM({3.0, 3.0, 0.0}, 0.0));

    seqXY.reverse();

    CoordinateXY c;
    seqXY.get(c, 0);
    EXPECT_EQ(c.pos.x, 3.0);
    EXPECT_EQ(c.pos.y, 3.0);

    seqXY.get(c, 2);
    EXPECT_EQ(c.pos.x, 1.0);
    EXPECT_EQ(c.pos.y, 1.0);
}

// ========== CoordSeq Equals Tests ==========
class CoordSeqEqualsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY1{CoordinateType::XY};
    CoordinateSeq seqXY2{CoordinateType::XY};
};

TEST_F(CoordSeqEqualsTest, EqualsIdentical)
{
    seqXY1.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY1.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    seqXY2.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY2.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    EXPECT_TRUE(seqXY1 == seqXY2);
}

TEST_F(CoordSeqEqualsTest, NotEqualsDifferentSize)
{
    seqXY1.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY1.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    seqXY2.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));

    EXPECT_FALSE(seqXY1 == seqXY2);
}

TEST_F(CoordSeqEqualsTest, NotEqualsDifferentCoordinates)
{
    seqXY1.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY1.push_back(CoordinateXYZM({3.0, 4.0, 0.0}, 0.0));

    seqXY2.push_back(CoordinateXYZM({1.0, 2.0, 0.0}, 0.0));
    seqXY2.push_back(CoordinateXYZM({5.0, 6.0, 0.0}, 0.0));

    EXPECT_FALSE(seqXY1 == seqXY2);
}

// ========== CoordSeq Duplicate Prevention Tests ==========
class CoordSeqDuplicateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {}

    CoordinateSeq seqXY{CoordinateType::XY};
};

TEST_F(CoordSeqDuplicateTest, AllowDuplicates)
{
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0), true);
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0), true);

    EXPECT_EQ(seqXY.size(), 2);
}

TEST_F(CoordSeqDuplicateTest, SkipDuplicates)
{
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0), true);
    seqXY.push_back(CoordinateXYZM({1.0, 1.0, 0.0}, 0.0), false);

    EXPECT_EQ(seqXY.size(), 1);
}

} // namespace geom
