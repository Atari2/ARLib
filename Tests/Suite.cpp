#include "Suite.h"
#include "../String.h"
#include "../Vector.h"

namespace ARLib {
    void run_all_tests() {
        size_t test_count = 0;
        size_t passed_count = 0;
        auto lam = [](String a, String b) -> bool {
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
        ASSERT_TEST("String equality", lam, "hello"_s, "hello"_s);
        ASSERT_TEST("Vector equality", vec, Vector{"hello"_s}, Vector{"hello"_s});
        printf("Passed %llu tests on %llu total\n", passed_count, test_count);
    }
}