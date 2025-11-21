cmake_minimum_required(VERSION 3.10)

project(lab5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(LIB_SOURCES
    src/fixed_block_memory_resource.cpp
)

add_library(stackLib ${LIB_SOURCES})
target_include_directories(stackLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# основной файл
add_executable(stack main.cpp)
target_link_libraries(stack PRIVATE stackLib)

# тесты
enable_testing()
find_package(GTest REQUIRED)

# тест stack
add_executable(test_stack tests/test_stack.cpp)
target_link_libraries(test_stack PRIVATE stackLib GTest::gtest GTest::gtest_main)
add_test(NAME StackTests COMMAND test_stack)

# тест memory_resource
add_executable(test_memory_resource tests/test_memory_resource.cpp)
target_link_libraries(test_memory_resource PRIVATE stackLib GTest::gtest GTest::gtest_main)
add_test(NAME MemoryResourceTests COMMAND test_memory_resource)
