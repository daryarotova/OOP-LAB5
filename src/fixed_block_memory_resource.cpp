#include "../include/fixed_block_memory_resource.h"

// конструктор
fixed_block_memory_resource::fixed_block_memory_resource(std::size_t size)
    : m_buffer(nullptr), m_capacity(size), m_next_offset(0)
{
    if (m_capacity == 0) m_capacity = 1;
    m_buffer = new char[m_capacity];
}

// деструктор, всё очищаем
fixed_block_memory_resource::~fixed_block_memory_resource()
{
    m_used_blocks.clear();
    m_free_blocks.clear();
    delete[] m_buffer;
    m_buffer = nullptr;
    m_capacity = 0;
    m_next_offset = 0;
}

std::size_t fixed_block_memory_resource::align_up(std::size_t offset, std::size_t alignment) noexcept
{
    if (alignment <= 1) return offset; // делаем выравнивание
    std::size_t rem = offset % alignment;
    return rem == 0 ? offset : offset + (alignment - rem);
}

void fixed_block_memory_resource::insert_used_block(Block block)
{
    auto it = std::upper_bound(
        m_used_blocks.begin(), m_used_blocks.end(), block.offset,
        [](std::size_t val, const Block& b) {
            return val < b.offset;
        }
    );
    m_used_blocks.insert(it, block);
}

// удаление использ. блока по смещению
void fixed_block_memory_resource::remove_used_block_by_offset(std::size_t offset)
{
    for (auto it = m_used_blocks.begin(); it != m_used_blocks.end(); ++it) {
        if (it->offset == offset) {
            m_used_blocks.erase(it);
            return;
        }
    }
}

// объединяем свободные блоки (смежные)
void fixed_block_memory_resource::try_merge_free_blocks()
{
    if (m_free_blocks.empty()) return;

    std::sort(m_free_blocks.begin(), m_free_blocks.end(),
              [](const Block& a, const Block& b) { return a.offset < b.offset; });

    std::vector<Block> merged;
    merged.reserve(m_free_blocks.size());
    merged.push_back(m_free_blocks.front());

    for (std::size_t i = 1; i < m_free_blocks.size(); ++i) {
        Block& last = merged.back();
        const Block& curr = m_free_blocks[i];

        if (last.offset + last.size >= curr.offset) {
            last.size = std::max(last.offset + last.size, curr.offset + curr.size) - last.offset;
        } else {
            merged.push_back(curr);
        }
    }

    m_free_blocks.swap(merged);
}

void* fixed_block_memory_resource::do_allocate(std::size_t bytes, std::size_t alignment)
{
    if (bytes == 0) return nullptr;

    // ищем подходящий свободный блок
    for (std::size_t i = 0; i < m_free_blocks.size(); ++i) {
        Block free_block = m_free_blocks[i];
        std::size_t aligned = align_up(free_block.offset, alignment);

        if (aligned + bytes <= free_block.offset + free_block.size) {
            Block used{aligned, bytes};
            insert_used_block(used);

            std::vector<Block> remaining;
            std::size_t prefix = aligned - free_block.offset;
            std::size_t suffix_offset = aligned + bytes;
            std::size_t suffix = (free_block.offset + free_block.size > suffix_offset)
                                 ? free_block.offset + free_block.size - suffix_offset
                                 : 0;

            if (prefix > 0) remaining.push_back({free_block.offset, prefix});
            if (suffix > 0) remaining.push_back({suffix_offset, suffix});

            m_free_blocks.erase(m_free_blocks.begin() + i);
            if (!remaining.empty())
                m_free_blocks.insert(m_free_blocks.begin() + i, remaining.begin(), remaining.end());

            return static_cast<void*>(m_buffer + used.offset);
        }
    }

    // если свободного блока нет, выделяем в конце буфера
    std::size_t aligned = align_up(m_next_offset, alignment);
    if (aligned + bytes > m_capacity) throw std::bad_alloc();

    Block used{aligned, bytes};
    insert_used_block(used);
    m_next_offset = aligned + bytes;

    return static_cast<void*>(m_buffer + used.offset);
}

void fixed_block_memory_resource::do_deallocate(void* ptr, std::size_t bytes, std::size_t /*alignment*/)
{
    if (!ptr || bytes == 0) return;

    std::size_t offset = static_cast<char*>(ptr) - m_buffer;
    if (offset >= m_capacity) throw std::logic_error("Pointer out of buffer range");

    bool found = false;
    for (const auto& blk : m_used_blocks) {
        if (blk.offset == offset && blk.size == bytes) {
            found = true;
            break;
        }
    }
    if (!found) throw std::logic_error("Invalid deallocation");

    remove_used_block_by_offset(offset);
    m_free_blocks.push_back({offset, bytes});
    try_merge_free_blocks();

    // если блок в конце, m_next_offset сдвигается
    for (auto it = m_free_blocks.begin(); it != m_free_blocks.end(); ++it) {
        if (it->offset + it->size == m_next_offset) {
            m_next_offset = it->offset;
            m_free_blocks.erase(it);
            try_merge_free_blocks();
            break;
        }
    }
}

// сравнение объектов по адресу
bool fixed_block_memory_resource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    return this == &other;
}
