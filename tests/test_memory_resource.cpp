#include <gtest/gtest.h>
#include "../include/fixed_block_memory_resource.h"

TEST(MemoryResourceTest, AllocateDeallocate) {
    fixed_block_memory_resource mem(1024);

    void* p1 = mem.allocate(64);
    ASSERT_NE(p1, nullptr);

    void* p2 = mem.allocate(32);
    ASSERT_NE(p2, nullptr);

    mem.deallocate(p1, 64);
    mem.deallocate(p2, 32);

    void* p3 = mem.allocate(64);
    ASSERT_NE(p3, nullptr);
}

TEST(MemoryResourceTest, AlignmentTest) {
    fixed_block_memory_resource mem(2048);

    void* p = mem.allocate(1, alignof(std::max_align_t));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % alignof(std::max_align_t), 0);

    void* p2 = mem.allocate(1, alignof(std::max_align_t)/2);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p2) % (alignof(std::max_align_t)/2), 0);
}


TEST(MemoryResourceTest, OutOfRangeDeallocate) {
    fixed_block_memory_resource mem(1024);

    char dummy[10];
    EXPECT_THROW(mem.deallocate(dummy, 10), std::logic_error);
}

TEST(MemoryResourceTest, AllocateUntilFull) {
    fixed_block_memory_resource mem(128);

    void* p1 = mem.allocate(64);
    ASSERT_NE(p1, nullptr);

    void* p2 = mem.allocate(64);
    ASSERT_NE(p2, nullptr);

    void* p_unused = nullptr;

    EXPECT_THROW(p_unused = mem.allocate(1), std::bad_alloc);
}

TEST(MemoryResourceTest, ReuseFreedBlocks) {
    fixed_block_memory_resource mem(256);

    void* p1 = mem.allocate(64);
    void* p2 = mem.allocate(64);
    void* p3 = mem.allocate(64);

    mem.deallocate(p2, 64);
    mem.deallocate(p1, 64);

    void* p4 = mem.allocate(64);
    EXPECT_TRUE(p4 == p1 || p4 == p2);

    void* p5 = mem.allocate(64);
    EXPECT_TRUE(p5 == p1 || p5 == p2);
}

TEST(MemoryResourceTest, MergeAdjacentFreeBlocks) {
    fixed_block_memory_resource mem(256);

    void* p1 = mem.allocate(64);
    void* p2 = mem.allocate(64);
    void* p3 = mem.allocate(64);

    mem.deallocate(p1, 64);
    mem.deallocate(p2, 64);

    void* p4 = mem.allocate(128);
    ASSERT_NE(p4, nullptr);
}

TEST(MemoryResourceTest, DeallocateEndBlockReducesNextOffset) {
    fixed_block_memory_resource mem(256);

    void* p1 = mem.allocate(64);
    void* p2 = mem.allocate(64);

    mem.deallocate(p2, 64);

    void* p3 = mem.allocate(64);
    EXPECT_EQ(p3, p2);
}
