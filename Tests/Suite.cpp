#include "Suite.h"
#include "../String.h"
#include "../Vector.h"
#include "../HashMap.h"
#include "../CharConv.h"
#include "../SortedVector.h"
#include "../Set.h"
#include "../Optional.h"
#include "../Result.h"

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
                RETURN_IF_NOT(a[i], b[i]);
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
        auto charconv = []() -> bool {
            String a{"1234"};
            String b{"123.123"};
            RETURN_IF_NOT(StrToInt(a), 1234);
            RETURN_IF_NOT(StrToFloat(b), 123.123f);
            int c = 101010;
            float d = 987.65f;
            RETURN_IF_NOT(IntToStr(c), "101010"_s);
            RETURN_IF_NOT(FloatToStr(d).substring(0, 6), "987.65"_s);
            return true;
        };
        auto sortedvec = []() -> bool {
            String a{"aaa"};
            String b{"bbb"};
            String c{"ccc"};
            SortedVector<String> vec{};
            vec.insert(c);
            vec.insert(b);
            vec.insert(a);
            RETURN_IF_NOT(vec.size(), 3ull);
            RETURN_IF_NOT(vec[0], "aaa"_s);
            RETURN_IF_NOT(vec[1], "bbb"_s);
            RETURN_IF_NOT(vec[2], "ccc"_s);
            return true;
        };
        auto set = []() -> bool {
            Set<int> s{};
            RETURN_IF_NOT(s.insert(10), true);
            RETURN_IF_NOT(s.insert(20), true);
            RETURN_IF_NOT(s.insert(10), false);
            RETURN_IF_NOT(s.remove(20), true);
            RETURN_IF_NOT(s.remove(20), false);
            return true;
        };
        auto optional = []() -> bool {
            Optional<String> opt{};
            RETURN_IF_NOT(opt.empty(), true);
            opt.emplace("hello", 5);
            RETURN_IF_NOT(opt.has_value(), true);
            delete opt.detach();
            RETURN_IF_NOT(opt.empty(), true);
            return true;
        };
        auto result = []() -> bool {
            Result<String> res{"hello"_s};
            RETURN_IF_NOT(res.is_ok(), true);
            RETURN_IF_NOT(res.is_error(), false);
            String b = res.to_ok();
            RETURN_IF_NOT(b, "hello"_s);
            auto res2 = Result<String>::from_error();
            RETURN_IF_NOT(res2.is_ok(), false);
            RETURN_IF_NOT(res2.is_error(), true);
            return true;
        };
        ASSERT_TEST("String equality", streq, "hello"_s, "hello"_s);
        ASSERT_TEST("Vector equality", vec, Vector{"hello"_s}, Vector{"hello"_s});
        ASSERT_TEST("String length", strlen, "hello"_s, ARLib::strlen("hello"));
        ASSERT_TEST("Hashmap tests", hashmap);
        ASSERT_TEST("String to integer/float conversions", charconv);
        ASSERT_TEST("Sorted vector correctness", sortedvec);
        ASSERT_TEST("Set correctness, uniqueness", set);
        ASSERT_TEST("Optional correctness", optional);
        ASSERT_TEST("Result correctness", result);
        printf("Passed %llu tests on %llu total\n", passed_count, test_count);
        return passed_count == test_count;
    }
}