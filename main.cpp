#include "include/fixed_block_memory_resource.h"
#include "include/stack.h"
#include <iostream>
#include <string>

struct Book {
    int id;
    std::string title;
    double price;

    Book(int i, std::string t, double p)
        : id(i), title(std::move(t)), price(p) {}
};

int main() {
    fixed_block_memory_resource memoryPool(1024 * 1024);

    {
        std::cout << "Test stack<int>\n";

        std::pmr::polymorphic_allocator<int> intAlloc(&memoryPool);
        stack<int> s(intAlloc);

        s.push(17);
        s.push(3);
        s.push(90);

        std::cout << "Stack contents: ";
        for (int value : s) {
            std::cout << value << " ";
        }
        std::cout << "\n";

        std::cout << "Top element: " << s.top() << "\n";

        s.pop();
        std::cout << "New top after pop previous top: " << s.top() << "\n";
    }

    {
        std::cout << "\nTest stack<Book>\n";

        std::pmr::polymorphic_allocator<Book> bookAlloc(&memoryPool);
        stack<Book> s(bookAlloc);

        s.emplace(1, "Jane Eyre", 59.99);
        s.emplace(2, "Music", 19.76);
        s.emplace(3, "How to besome millionere", 99.50);

        std::cout << "Stack contents:\n";
        for (auto& book : s) {
            std::cout << "  id=" << book.id
                      << ", title=\"" << book.title << "\""
                      << ", price=" << book.price << "\n";
        }

        std::cout << "Top book: \"" << s.top().title
                  << "\" (id=" << s.top().id << ")\n";

        s.pop();
        std::cout << "New top after pop previous top: \"" << s.top().title << "\"\n";
    }

    std::cout << "\nAll is successfull. The end.\n";
    return 0;
}
