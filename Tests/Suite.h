#pragma once
#include "../Algorithm.h"
#include "../Array.h"
#include "../Async.h"
#include "../BigInt.h"
#include "../CharConv.h"
#include "../Chrono.h"
#include "../Enumerate.h"
#include "../EventLoop.h"
#include "../FixedMatrix.h"
#include "../Functional.h"
#include "../GenericView.h"
#include "../Hash.h"
#include "../HashMap.h"
#include "../JSONParser.h"
#include "../LinkedSet.h"
#include "../List.h"
#include "../Map.h"
#include "../Matrix.h"
#include "../Optional.h"
#include "../Printer.h"
#include "../PriorityQueue.h"
#include "../Process.h"
#include "../Random.h"
#include "../Result.h"
#include "../SSOVector.h"
#include "../Set.h"
#include "../SortedVector.h"
#include "../Stack.h"
#include "../String.h"
#include "../StringLiteral.h"
#include "../Test.h"
#include "../Threading.h"
#include "../Tree.h"
#include "../Tuple.h"
#include "../UniqueString.h"
#include "../Variant.h"
#include "../Vector.h"
#include "../ArgParser.h"
#include "../cstdio_compat.h"
namespace ARLib {
size_t test_partial_func(int a, String b, Tuple<String, int> c);
#define TEST_DECL_NAMED(name, lambda, ...)                                                                             \
    Test<decltype(lambda), ##__VA_ARGS__> name { lambda }
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
