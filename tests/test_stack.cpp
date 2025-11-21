#include <gtest/gtest.h>
#include "../include/stack.h"
#include "../include/fixed_block_memory_resource.h"
#include <vector>

TEST(StackTest, PushPopInt) {
    fixed_block_memory_resource mem(1024);
    std::pmr::polymorphic_allocator<int> alloc(&mem);

    stack<int> st(alloc);

    st.push(10);
    st.push(20);
    st.push(30);

    EXPECT_EQ(st.size(), 3);
    EXPECT_EQ(st.top(), 30);

    st.pop();
    EXPECT_EQ(st.top(), 20);

    st.pop();
    EXPECT_EQ(st.top(), 10);

    st.pop();
    EXPECT_TRUE(st.empty());
}

struct Person {
    int id;
    std::string name;
    double rating;

    Person(int i, std::string n, double r)
        : id(i), name(std::move(n)), rating(r) {}
};

TEST(StackTest, PushPerson) {
    fixed_block_memory_resource mem(2048);
    std::pmr::polymorphic_allocator<Person> alloc(&mem);

    stack<Person> st(alloc);

    st.emplace(1, "Elena", 4.8);
    st.emplace(2, "Maxim", 4.0);

    EXPECT_EQ(st.size(), 2);
    EXPECT_EQ(st.top().name, "Maxim");

    st.pop();
    EXPECT_EQ(st.top().name, "Elena");
}

TEST(StackTest, IterationOrder) {
    fixed_block_memory_resource mem(2048);
    std::pmr::polymorphic_allocator<int> alloc(&mem);

    stack<int> st(alloc);
    st.push(1);
    st.push(2);
    st.push(3);

    std::vector<int> v;
    for (int x : st) v.push_back(x);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 1);
}

TEST(StackTest, ClearAndReuse) {
    fixed_block_memory_resource mem(1024);
    std::pmr::polymorphic_allocator<int> alloc(&mem);
    stack<int> st(alloc);

    st.push(5);
    st.push(10);
    st.push(15);

    EXPECT_EQ(st.size(), 3);
    st.clear();
    EXPECT_TRUE(st.empty());

    st.push(42);
    EXPECT_EQ(st.size(), 1);
    EXPECT_EQ(st.top(), 42);
}

TEST(StackTest, MoveStack) {
    fixed_block_memory_resource mem(1024);
    std::pmr::polymorphic_allocator<int> alloc(&mem);

    stack<int> st1(alloc);
    st1.push(1);
    st1.push(2);

    stack<int> st2(std::move(st1));

    EXPECT_EQ(st2.size(), 2);
    EXPECT_EQ(st2.top(), 2);
    EXPECT_TRUE(st1.empty());
}


TEST(StackTest, PopTopOnEmptyThrows) {
    fixed_block_memory_resource mem(512);
    std::pmr::polymorphic_allocator<int> alloc(&mem);
    stack<int> st(alloc);

    EXPECT_TRUE(st.empty());
    EXPECT_THROW(st.pop(), std::out_of_range);
    EXPECT_THROW(st.top(), std::out_of_range);
}
