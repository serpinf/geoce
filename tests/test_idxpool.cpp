#include "pch.h"
#include "gcprec.h"
#include "../src/gisdom/idxpool.h"

using namespace gce;

// ============================================================================
// idxpool Tests
// ============================================================================

class IdxpoolTest : public ::testing::Test
{
protected:
    static constexpr uint32_t POOL_SIZE = 10;
};

TEST_F(IdxpoolTest, ConstructorInitializesPool)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    EXPECT_EQ(pool.size(), POOL_SIZE);
    EXPECT_EQ(pool.freeCount(), POOL_SIZE);
}

TEST_F(IdxpoolTest, IsIdValidatesIndices)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    EXPECT_TRUE(pool.isId(0));
    EXPECT_TRUE(pool.isId(POOL_SIZE - 1));
    EXPECT_FALSE(pool.isId(POOL_SIZE));
    EXPECT_FALSE(pool.isId(POOL_SIZE + 1));
}

TEST_F(IdxpoolTest, NewXAllocatesIndices)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t idx = idxpool<uint32_t>::index_max;

    bool result = pool.newX(idx);
    EXPECT_TRUE(result);
    EXPECT_NE(idx, idxpool<uint32_t>::index_max);
    EXPECT_TRUE(pool.isId(idx));
    EXPECT_EQ(pool.freeCount(), POOL_SIZE - 1);
}

TEST_F(IdxpoolTest, NewXMultipleAllocations)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < POOL_SIZE; ++i)
    {
        uint32_t idx = idxpool<uint32_t>::index_max;
        bool result = pool.newX(idx);
        EXPECT_TRUE(result);
        indices.push_back(idx);
        EXPECT_EQ(pool.freeCount(), POOL_SIZE - i - 1);
    }

    // All indices should be unique
    std::sort(indices.begin(), indices.end());
    for (uint32_t i = 0; i < POOL_SIZE; ++i)
    {
        EXPECT_EQ(indices[i], i);
    }
}

TEST_F(IdxpoolTest, NewXExhaustsPool)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t idx = idxpool<uint32_t>::index_max;

    // Allocate all indices
    for (uint32_t i = 0; i < POOL_SIZE; ++i)
    {
        EXPECT_TRUE(pool.newX(idx));
    }

    // Next allocation should fail
    idx = idxpool<uint32_t>::index_max;
    bool result = pool.newX(idx);
    EXPECT_FALSE(result);
    EXPECT_EQ(idx, idxpool<uint32_t>::index_max);
    EXPECT_EQ(pool.freeCount(), 0);
}

TEST_F(IdxpoolTest, DelXFreesIndices)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t idx = idxpool<uint32_t>::index_max;

    // Allocate an index
    pool.newX(idx);
    uint32_t allocated_idx = idx;
    EXPECT_EQ(pool.freeCount(), POOL_SIZE - 1);

    // Free the index
    pool.delX(idx);
    EXPECT_EQ(idx, idxpool<uint32_t>::index_max);
    EXPECT_EQ(pool.freeCount(), POOL_SIZE);
}

TEST_F(IdxpoolTest, DelXInvalidIndex)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t invalid_idx = POOL_SIZE + 5;
    size_t initial_free = pool.freeCount();

    pool.delX(invalid_idx);

    // Invalid index should not change free count
    EXPECT_EQ(pool.freeCount(), initial_free);
    EXPECT_EQ(invalid_idx, POOL_SIZE + 5); // Index unchanged
}

TEST_F(IdxpoolTest, DelXResetsIndexToMax)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t idx = idxpool<uint32_t>::index_max;

    pool.newX(idx);
    uint32_t allocated = idx;
    pool.delX(idx);

    EXPECT_EQ(idx, idxpool<uint32_t>::index_max);
}

TEST_F(IdxpoolTest, AllocateAndDeallocateCycle)
{
    idxpool<uint32_t> pool(POOL_SIZE);
    uint32_t idx1 = idxpool<uint32_t>::index_max;
    uint32_t idx2 = idxpool<uint32_t>::index_max;

    // Allocate two indices
    pool.newX(idx1);
    pool.newX(idx2);
    EXPECT_EQ(pool.freeCount(), POOL_SIZE - 2);

    uint32_t idx1_bak = idx1;
    // Free first index
    pool.delX(idx1);
    EXPECT_EQ(pool.freeCount(), POOL_SIZE - 1);

    // Allocate again - should reuse freed index
    uint32_t idx3 = idxpool<uint32_t>::index_max;
    pool.newX(idx3);
    EXPECT_EQ(idx3, idx1_bak);
    EXPECT_EQ(pool.freeCount(), POOL_SIZE - 2);
}

TEST_F(IdxpoolTest, DifferentIndexTypes)
{
    idxpool<uint8_t> pool8(100);
    EXPECT_EQ(pool8.size(), 100);
    EXPECT_EQ(pool8.freeCount(), 100);

    idxpool<uint16_t> pool16(1000);
    EXPECT_EQ(pool16.size(), 1000);
    EXPECT_EQ(pool16.freeCount(), 1000);
}

TEST_F(IdxpoolTest, SmallPoolSize)
{
    idxpool<uint32_t> pool(1);
    uint32_t idx = idxpool<uint32_t>::index_max;

    bool result = pool.newX(idx);
    EXPECT_TRUE(result);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(pool.freeCount(), 0);

    // Next allocation fails
    result = pool.newX(idx);
    EXPECT_FALSE(result);
}

// ============================================================================
// idxalloca Tests
// ============================================================================

class IdxallocaTest : public ::testing::Test
{
protected:
    static constexpr uint32_t POOL_SIZE = 10;
};

TEST_F(IdxallocaTest, ConstructorWithDefaultDeleter)
{
    idxalloca<int> alloc(POOL_SIZE);
    EXPECT_EQ(alloc.size(), POOL_SIZE);
    EXPECT_EQ(alloc.freeCount(), POOL_SIZE);
}

TEST_F(IdxallocaTest, GetXValidIndex)
{
    idxalloca<int> alloc(POOL_SIZE);
    uint32_t idx = idxalloca<int>::index_max;

    alloc.newX(idx);
    int *ptr = alloc.getX(idx);

    EXPECT_NE(ptr, nullptr);
}

TEST_F(IdxallocaTest, GetXInvalidIndex)
{
    idxalloca<int> alloc(POOL_SIZE);
    int *ptr = alloc.getX(POOL_SIZE + 5);

    EXPECT_EQ(ptr, nullptr);
}

TEST_F(IdxallocaTest, GetXDataAccess)
{
    idxalloca<int> alloc(POOL_SIZE);
    uint32_t idx = idxalloca<int>::index_max;

    alloc.newX(idx);
    int *ptr = alloc.getX(idx);

    *ptr = 42;
    EXPECT_EQ(*ptr, 42);
}

TEST_F(IdxallocaTest, GetXMultipleIndices)
{
    idxalloca<int> alloc(POOL_SIZE);
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < 5; ++i)
    {
        uint32_t idx = idxalloca<int>::index_max;
        alloc.newX(idx);
        indices.push_back(idx);
    }

    // Verify all pointers are valid and distinct
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int *ptr = alloc.getX(indices[i]);
        EXPECT_NE(ptr, nullptr);

        // Verify pointers are distinct
        for (size_t j = i + 1; j < indices.size(); ++j)
        {
            int *other_ptr = alloc.getX(indices[j]);
            EXPECT_NE(ptr, other_ptr);
        }
    }
}

TEST_F(IdxallocaTest, AllocateAndWriteData)
{
    idxalloca<double> alloc(POOL_SIZE);
    std::vector<uint32_t> indices;

    // Allocate and write data
    for (uint32_t i = 0; i < 5; ++i)
    {
        uint32_t idx = idxalloca<double>::index_max;
        alloc.newX(idx);
        indices.push_back(idx);

        double *ptr = alloc.getX(idx);
        *ptr = static_cast<double>(i) * 3.14;
    }

    // Verify data
    for (size_t i = 0; i < indices.size(); ++i)
    {
        double *ptr = alloc.getX(indices[i]);
        EXPECT_DOUBLE_EQ(*ptr, static_cast<double>(i) * 3.14);
    }
}

TEST_F(IdxallocaTest, DifferentDataTypes)
{
    // Test with different types
    idxalloca<char> alloc_char(10);
    uint32_t idx_char = idxalloca<char>::index_max;
    alloc_char.newX(idx_char);
    *alloc_char.getX(idx_char) = 'A';
    EXPECT_EQ(*alloc_char.getX(idx_char), 'A');

    idxalloca<double> alloc_double(10);
    uint32_t idx_double = idxalloca<double>::index_max;
    alloc_double.newX(idx_double);
    *alloc_double.getX(idx_double) = 3.14159;
    EXPECT_DOUBLE_EQ(*alloc_double.getX(idx_double), 3.14159);
}

TEST_F(IdxallocaTest, AllocationAndDeallocation)
{
    idxalloca<int> alloc(POOL_SIZE);
    uint32_t idx = idxalloca<int>::index_max;

    // Allocate
    alloc.newX(idx);
    int *ptr = alloc.getX(idx);
    EXPECT_NE(ptr, nullptr);
    *ptr = 100;

    // Deallocate
    alloc.delX(idx);
    EXPECT_EQ(idx, idxalloca<int>::index_max);

    // Index should be reusable
    EXPECT_EQ(alloc.freeCount(), POOL_SIZE);
}

TEST_F(IdxallocaTest, ExhaustAllocation)
{
    idxalloca<int> alloc(POOL_SIZE);
    std::vector<uint32_t> indices;

    // Allocate all
    for (uint32_t i = 0; i < POOL_SIZE; ++i)
    {
        uint32_t idx = idxalloca<int>::index_max;
        bool result = alloc.newX(idx);
        EXPECT_TRUE(result);
        indices.push_back(idx);
    }

    // Next allocation fails
    uint32_t idx = idxalloca<int>::index_max;
    bool result = alloc.newX(idx);
    EXPECT_FALSE(result);
    EXPECT_EQ(idx, idxalloca<int>::index_max);
}

TEST_F(IdxallocaTest, InheritanceFromIdxpool)
{
    idxalloca<int> alloc(POOL_SIZE);

    // Verify inherited methods work
    EXPECT_EQ(alloc.size(), POOL_SIZE);
    EXPECT_EQ(alloc.freeCount(), POOL_SIZE);
    EXPECT_TRUE(alloc.isId(0));
    EXPECT_FALSE(alloc.isId(POOL_SIZE));
}

TEST_F(IdxallocaTest, GetXBoundaryConditions)
{
    idxalloca<int> alloc(POOL_SIZE);

    // Boundary indices
    EXPECT_EQ(alloc.getX(0), alloc.getX(0)); // First index should be same
    EXPECT_EQ(alloc.getX(POOL_SIZE - 1), alloc.getX(POOL_SIZE - 1)); // Last valid
    EXPECT_EQ(alloc.getX(POOL_SIZE), nullptr); // First invalid
    EXPECT_EQ(alloc.getX(POOL_SIZE + 100), nullptr); // Far beyond
}

TEST_F(IdxallocaTest, ComplexDataType)
{
    struct TestData
    {
        int value;
        double ratio;
        char flag;
    };

    idxalloca<TestData> alloc(POOL_SIZE);
    uint32_t idx = idxalloca<TestData>::index_max;

    alloc.newX(idx);
    TestData *ptr = alloc.getX(idx);

    ptr->value = 42;
    ptr->ratio = 2.71828;
    ptr->flag = 'X';

    EXPECT_EQ(ptr->value, 42);
    EXPECT_DOUBLE_EQ(ptr->ratio, 2.71828);
    EXPECT_EQ(ptr->flag, 'X');
}

// ============================================================================
// Integration Tests
// ============================================================================

class IdxpoolIntegrationTest : public ::testing::Test
{
protected:
    static constexpr uint32_t POOL_SIZE = 100;
};

TEST_F(IdxpoolIntegrationTest, LargePoolAllocationPattern)
{
    idxalloca<int> alloc(POOL_SIZE);
    std::set<uint32_t> active_indices;

    // Allocate 50 items
    for (int i = 0; i < 50; ++i)
    {
        uint32_t idx = idxalloca<int>::index_max;
        EXPECT_TRUE(alloc.newX(idx));
        active_indices.insert(idx);
        *alloc.getX(idx) = i;
    }

    // Free every other item
    std::vector<uint32_t> to_free;
    int count = 0;
    for (uint32_t idx : active_indices)
    {
        if (count++ % 2 == 0)
            to_free.push_back(idx);
    }

    for (uint32_t idx : to_free)
    {
        alloc.delX(idx);
        active_indices.erase(idx);
    }

    // Reallocate and verify reuse
    std::set<uint32_t> newly_allocated;
    for (int i = 0; i < 25; ++i)
    {
        uint32_t idx = idxalloca<int>::index_max;
        EXPECT_TRUE(alloc.newX(idx));
        newly_allocated.insert(idx);
    }

    // Newly allocated indices should overlap with freed ones
    int overlap = 0;
    for (uint32_t idx : newly_allocated)
    {
        if (std::find(to_free.begin(), to_free.end(), idx) != to_free.end())
            overlap++;
    }
    EXPECT_GT(overlap, 0);
}

TEST_F(IdxpoolIntegrationTest, StressTest)
{
    idxalloca<int> alloc(1000);
    std::vector<uint32_t> allocated;

    // Allocate and deallocate multiple times
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        allocated.clear();

        // Allocate 500 items
        for (int i = 0; i < 500; ++i)
        {
            uint32_t idx = idxalloca<int>::index_max;
            EXPECT_TRUE(alloc.newX(idx));
            allocated.push_back(idx);
        }

        // Free all
        for (uint32_t idx : allocated)
        {
            alloc.delX(idx);
        }
    }

    // All should be freed
    EXPECT_EQ(alloc.freeCount(), 1000);
}
