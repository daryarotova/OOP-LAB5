#ifndef STACK_H
#define STACK_H

#include <memory_resource>
#include <memory>
#include <iterator>
#include <cstddef>
#include <utility>
#include <stdexcept>

template <typename T, typename Alloc = std::pmr::polymorphic_allocator<T>>
class stack {
private:
    struct Node {
        T data;
        Node* next_node;
        
        template <typename... Args>
        Node(Args&&... args) 
            : data(std::forward<Args>(args)...), next_node(nullptr) {}
    };

    using node_allocator_type = std::pmr::polymorphic_allocator<Node>;

public:
    using value_type = T;
    using allocator_type = Alloc;

    // итератор
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(Node* node_ptr = nullptr) noexcept : current(node_ptr) {}

        reference operator*() const { return current->data; }
        pointer operator->() const { return std::addressof(current->data); }

        iterator& operator++() { 
            if (current) current = current->next_node;
            return *this;
        }
        iterator operator++(int) { 
            iterator tmp = *this; // сохраняем исходное состояние
            ++(*this); // для движения итератора
            return tmp;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs) noexcept {
            return lhs.current == rhs.current;
        }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) noexcept {
            return !(lhs == rhs);
        }

    private:
        Node* current;
    };

    // конструктор с аллокатором
    explicit stack(const allocator_type& alloc = allocator_type())
        : m_alloc(node_allocator_type(alloc.resource())),
          m_top(nullptr),
          m_count(0),
          m_value_alloc(alloc) {}

    stack(const stack&) = delete;
    stack& operator=(const stack&) = delete;

    stack(stack&& other) noexcept
        : m_alloc(std::move(other.m_alloc)),
          m_top(other.m_top),
          m_count(other.m_count),
          m_value_alloc(other.m_value_alloc) 
    {
        other.m_top = nullptr;
        other.m_count = 0;
    }

    stack& operator=(stack&& other) noexcept {
        if (this != &other) {
            clear();
            m_alloc = std::move(other.m_alloc);
            m_top = other.m_top;
            m_count = other.m_count;
            m_value_alloc = other.m_value_alloc;

            other.m_top = nullptr;
            other.m_count = 0;
        }
        return *this;
    }

    ~stack() {
        clear();
    }

    // просто добавляем элементы в начало стека (li-fo)
    void push(const T& val) {
        emplace(val);
    }
    void push(T&& val) {
        emplace(std::move(val));
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        // выделяем память для 1 узла
        Node* node = std::allocator_traits<node_allocator_type>::allocate(m_alloc, 1);
        try {
            std::allocator_traits<node_allocator_type>::construct(m_alloc, node, std::forward<Args>(args)...);
        } catch (...) {
            // если ошибка, не забыть очистить память
            std::allocator_traits<node_allocator_type>::deallocate(m_alloc, node, 1);
            throw;
        }
        // вставляем узел в top
        node->next_node = m_top;
        m_top = node;
        ++m_count;
    }

    void pop() {
        if (empty()) throw std::out_of_range("Cannot pop from empty stack");
        Node* node = m_top;
        m_top = m_top->next_node;
        std::allocator_traits<node_allocator_type>::destroy(m_alloc, node);
        std::allocator_traits<node_allocator_type>::deallocate(m_alloc, node, 1);
        --m_count;
    }

    // просмотр врхнего элемента
    T& top() {
        if (empty()) throw std::out_of_range("Stack is empty");
        return m_top->data;
    }

    const T& top() const {
        if (empty()) throw std::out_of_range("Stack is empty");
        return m_top->data;
    }

    bool empty() const noexcept {
        return m_top == nullptr;
    }
    std::size_t size() const noexcept {
        return m_count;
    }

    iterator begin() noexcept {
        return iterator(m_top);
    }
    iterator end() noexcept {
        return iterator(nullptr);
    }

    void clear() noexcept {
        while (m_top) {
            Node* next = m_top->next_node;
            std::allocator_traits<node_allocator_type>::destroy(m_alloc, m_top);
            std::allocator_traits<node_allocator_type>::deallocate(m_alloc, m_top, 1);
            m_top = next;
        }
        m_count = 0;
    }

    allocator_type get_allocator_for_values() const {
        return m_value_alloc;
    }

private:
    node_allocator_type m_alloc;
    Node* m_top;
    std::size_t m_count;
    allocator_type m_value_alloc;
};

#endif
