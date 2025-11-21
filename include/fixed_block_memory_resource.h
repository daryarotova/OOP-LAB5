#ifndef FIXED_BLOCK_MEMORY_RESOURCE_H
#define FIXED_BLOCK_MEMORY_RESOURCE_H

#include <memory_resource>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <stdexcept>

class fixed_block_memory_resource : public std::pmr::memory_resource {
public:
    explicit fixed_block_memory_resource(std::size_t total_size_bytes = 1024 * 1024);
    ~fixed_block_memory_resource() override;

    fixed_block_memory_resource(const fixed_block_memory_resource&) = delete;
    fixed_block_memory_resource& operator=(const fixed_block_memory_resource&) = delete;

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override;
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

private:
    struct Block {
        std::size_t offset;
        std::size_t size;
    };

    static std::size_t align_up(std::size_t offset, std::size_t alignment) noexcept;
    void insert_used_block(Block b);
    void remove_used_block_by_offset(std::size_t offset);
    void try_merge_free_blocks();

private:
    char* m_buffer;
    std::size_t m_capacity;
    std::vector<Block> m_used_blocks;
    std::vector<Block> m_free_blocks;
    std::size_t m_next_offset;
};
#endif
