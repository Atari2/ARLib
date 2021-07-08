#pragma once
#include "../Test.h"
#include "../cstdio_compat.h"

namespace ARLib {
#define TEST_DECL_NAMED(name, lambda, ...)                                                                             \
    Test<decltype(lambda), ##__VA_ARGS__> name { lambda }
#define TEST_DECL(type, ...) Test<type, ##__VA_ARGS__>

    template <typename T, typename... Args>
    bool decl_and_run(T lambda, Args... args) {
        TEST_DECL(T, Args...) test{lambda};
        return test.run(args...);
    }
#define RUN_TEST(lambda, ...) decl_and_run(lambda, ##__VA_ARGS__)
#define ASSERT_TEST(message, lambda, ...)                                                                              \
    {                                                                                                                  \
        printf("Running test %llu, named \"%s\"\n", test_count, message);                                              \
        auto res = RUN_TEST(lambda, ##__VA_ARGS__);                                                                    \
        if (!res)                                                                                                      \
            printf("Failed test %llu, named \"%s\"", test_count, message);                                             \
        else                                                                                                           \
            printf("Passed test %llu, named \"%s\"\n", test_count, message);                                           \
        if (res) passed_count++;                                                                                       \
        test_count++;                                                                                                  \
    } // namespace ARLib
    void run_all_tests();
} // namespace ARLib