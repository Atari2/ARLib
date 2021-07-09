#include "Suite.h"
#include "../String.h"
#include "../Vector.h"
#include "../HashMap.h"

namespace ARLib {
    bool run_all_tests() {
        size_t test_count = 0;
        size_t passed_count = 0;
        auto streq = [](String a, String b) -> bool {
            return assert_eq(a, b);
        };
        auto vec = [](const Vector<String>& a, const Vector<String>& b) -> bool {
            bool len = assert_eq(a.size(), b.size());
            if (!len) return false;
            for (size_t i = 0; i < a.size(); i++) {
                bool eq = assert_eq(a[i], b[i]);
                if (!eq) return false;
            }
            return true;
        };
        auto strlen = [](String a, size_t b) -> bool {
            return assert_eq(a.size(), b);
        };
        auto hashmap = []() -> bool {
            HashMap<String, int> map {};
            auto val = map.add("hello"_s, 10);
            auto ex = map.add("world"_s, 20);
            RETURN_IF_NOT(val, InsertionResult::New);
            RETURN_IF_NOT(ex, InsertionResult::New);
            auto r = map["hello"_s];
            RETURN_IF_NOT(r, 10);
            val = map.add("hello"_s, 30);
            RETURN_IF_NOT(val, InsertionResult::Replace);
            r = map["hello"_s];
            RETURN_IF_NOT(r, 30);
            RETURN_IF_NOT(map.size(), 2ull);
            auto res = map.remove("hello"_s);
            RETURN_IF_NOT(res, DeletionResult::Success);
            res = map.remove("hello"_s);
            RETURN_IF_NOT(res, DeletionResult::Failure);
            RETURN_IF_NOT(map.size(), 1ull);
            return true;
        };
        ASSERT_TEST("String equality", streq, "hello"_s, "hello"_s);
        ASSERT_TEST("Vector equality", vec, Vector{"hello"_s}, Vector{"hello"_s});
        ASSERT_TEST("String length", strlen, "hello"_s, 5ull);
        ASSERT_TEST("Hashmap tests", hashmap);
        printf("Passed %llu tests on %llu total\n", passed_count, test_count);
        return passed_count == test_count;
    }
}