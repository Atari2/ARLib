#pragma once
#include "Algorithm.hpp"
#include "Array.hpp"
#include "Async.hpp"
#include "BigInt.hpp"
#include "CharConv.hpp"
#include "Chrono.hpp"
#include "CSVParser.hpp"
#include "Enumerate.hpp"
#include "EventLoop.hpp"
#include "FixedMatrix.hpp"
#include "FlatMap.hpp"
#include "Functional.hpp"
#include "GenericView.hpp"
#include "Hash.hpp"
#include "JSONParser.hpp"
#include "LinkedSet.hpp"
#include "List.hpp"
#include "Map.hpp"
#include "Matrix.hpp"
#include "Optional.hpp"
#include "Printer.hpp"
#include "PriorityQueue.hpp"
#include "Process.hpp"
#include "Random.hpp"
#include "SSOVector.hpp"
#include "Set.hpp"
#include "SortedVector.hpp"
#include "Stack.hpp"
#include "Stream.hpp"
#include "String.hpp"
#include "StringLiteral.hpp"
#include "Test.hpp"
#include "Threading.hpp"
#include "Tree.hpp"
#include "Tuple.hpp"
#include "UniqueString.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "ArgParser.hpp"
#include "cstdio_compat.hpp"
namespace ARLib {
size_t test_partial_func(int a, String b, Tuple<String, int> c);
#define TEST_DECL_NAMED(name, lambda, ...)                                                                             \
    Test<decltype(lambda), ##__VA_ARGS__> name {                                                                       \
        lambda                                                                                                         \
    }
#define TEST_DECL(type, ...) Test<type, ##__VA_ARGS__>
template <typename T, typename... Args>
bool decl_and_run(T lambda, Args... args) {
    TEST_DECL(T, Args...) test{ lambda };
    return test.run(args...);
}
#define RUN_TEST(lambda, ...) decl_and_run(lambda, ##__VA_ARGS__)
#define ASSERT_TEST(message, lambda, ...)                                                                              \
    {                                                                                                                  \
        auto res = RUN_TEST(lambda, ##__VA_ARGS__);                                                                    \
        if (res) passed_count++;                                                                                       \
        test_count++;                                                                                                  \
    }    // namespace ARLib
#define RETURN_IF_NOT_EQ(x, y)                                                                                         \
    if (!assert_eq(x, y)) {                                                                                            \
        PRINT_SOURCE_LOCATION();                                                                                       \
        return false;                                                                                                  \
    }
#define RETURN_IF_EQ(x, y)                                                                                             \
    if (assert_eq(x, y)) {                                                                                             \
        PRINT_SOURCE_LOCATION();                                                                                       \
        return false;                                                                                                  \
    }
bool run_all_legacy_tests();
}    // namespace ARLib
