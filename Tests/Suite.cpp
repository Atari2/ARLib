#include "Suite.hpp"
namespace ARLib {
size_t test_partial_func(int a, String b, Tuple<String, int> c) {
    return static_cast<size_t>(a) + b.size() + c.get<0>().size() + static_cast<size_t>(c.get<1>());
}
bool run_all_legacy_tests() {
    size_t test_count   = 0;
    size_t passed_count = 0;
    auto streq          = [](const String& a, const String& b) -> bool {
        return assert_eq(a, b);
    };
    auto vec_test = [](const Vector<String>& a, const Vector<String>& b) -> bool {
        bool len = assert_eq(a.size(), b.size());
        if (!len) return false;
        for (size_t i = 0; i < a.size(); i++) { RETURN_IF_NOT_EQ(a[i], b[i]) }
        return true;
    };
    auto strlen_test = [](const String& a, size_t b) -> bool {
        return assert_eq(a.size(), b);
    };
    auto charconv = []() -> bool {
        String a{ "1234" };
        String b{ "123.123" };
        RETURN_IF_NOT_EQ(StrToInt(a), 1234)
        RETURN_IF_NOT_EQ(StrToFloat(b), 123.123f)
        int c   = 101010;
        float d = 987.65f;
        RETURN_IF_NOT_EQ(IntToStr(c), "101010"_s)
        RETURN_IF_NOT_EQ(FloatToStr(d).substring(0, 6), "987.65"_s)
        return true;
    };
    auto sortedvec = []() -> bool {
        String a{ "aaa" };
        String b{ "bbb" };
        String c{ "ccc" };
        SortedVector<String> sortedv{};
        sortedv.insert(c);
        sortedv.insert(b);
        sortedv.insert(a);
        RETURN_IF_NOT_EQ(sortedv.size(), 3ull)
        RETURN_IF_NOT_EQ(sortedv[0], "aaa"_s)
        RETURN_IF_NOT_EQ(sortedv[1], "bbb"_s)
        RETURN_IF_NOT_EQ(sortedv[2], "ccc"_s)
        return true;
    };
    auto set = []() -> bool {
        Set<int> s{};
        RETURN_IF_NOT_EQ(s.insert(10), 10)
        RETURN_IF_NOT_EQ(s.insert(20), 20)
        RETURN_IF_NOT_EQ(s.insert(10), 10)
        RETURN_IF_NOT_EQ(s.size(), 2_sz);
        RETURN_IF_NOT_EQ(s.remove(20), true)
        RETURN_IF_NOT_EQ(s.remove(20), false)
        return true;
    };
    auto optional = []() -> bool {
        Optional<String> opt{};
        RETURN_IF_NOT_EQ(opt.empty(), true)
        opt.emplace("hello", 5ull);
        RETURN_IF_NOT_EQ(opt.has_value(), true)
        opt.evict();
        RETURN_IF_NOT_EQ(opt.empty(), true)
        return true;
    };
    auto result = []() -> bool {
        Result<String> res{ "hello"_s, emplace_ok };
        RETURN_IF_NOT_EQ(res.is_ok(), true)
        RETURN_IF_NOT_EQ(res.is_error(), false)
        String b = res.to_ok();
        RETURN_IF_NOT_EQ(b, "hello"_s)
        auto res2 = Result<String>{ "error"_s, emplace_error };
        RETURN_IF_NOT_EQ(res2.is_ok(), false)
        RETURN_IF_NOT_EQ(res2.is_error(), true)
        res2.ignore_error();
        return true;
    };
    auto stack_test = []() -> bool {
        Stack<String> stack{};
        RETURN_IF_NOT_EQ(stack.size(), 0ull)
        stack.push("hello"_s);
        stack.push("world"_s);
        stack.push("testing"_s);
        RETURN_IF_NOT_EQ(stack.size(), 3ull)
        RETURN_IF_NOT_EQ(stack.peek(), "testing"_s)
        RETURN_IF_NOT_EQ(stack.pop(), "testing"_s)
        RETURN_IF_NOT_EQ(stack.peek(), "world"_s)
        RETURN_IF_NOT_EQ(stack.pop(), "world"_s)
        RETURN_IF_NOT_EQ(stack.size(), 1ull)
        return true;
    };
    auto tuple = []() -> bool {
        Tuple<int, String, double, Vector<String>> tup{
            0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}
        };
        Tuple<int, String, double, Vector<String>> tup3{
            0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}
        };
        RETURN_IF_NOT_EQ(tup, tup3)
        RETURN_IF_NOT_EQ(tup.get<0>(), 0)
        RETURN_IF_NOT_EQ(tup.get<1>(), "hello"_s)
        RETURN_IF_NOT_EQ(tup.get<2>(), 10.0)
        RETURN_IF_NOT_EQ(tup.get<3>()[0], "a"_s)
        RETURN_IF_NOT_EQ(tup.get<3>().size(), 3ull)
        tup.get<3>().push_back("k"_s);
        RETURN_IF_NOT_EQ(tup.get<3>().size(), 4ull)
        tup.set("world"_s);
        tup.set(54.4);
        RETURN_IF_NOT_EQ(tup.get<1>(), "world"_s)
        RETURN_IF_NOT_EQ(tup.get<2>(), 54.4)
        auto tup_2 = move(tup);
        RETURN_IF_NOT_EQ(tup_2.get<0>(), 0)
        RETURN_IF_NOT_EQ(tup_2.get<1>(), "world"_s)
        RETURN_IF_NOT_EQ(tup_2.get<2>(), 54.4)
        RETURN_IF_NOT_EQ(tup_2.get<3>()[3], "k"_s)
        RETURN_IF_NOT_EQ(tup_2.get<3>().size(), 4ull)
        return true;
    };
    auto partial_func = []() -> bool {
        auto decl = [](int a, const String& b, Tuple<String, int> c) {
            return static_cast<size_t>(a) + b.size() + c.get<0>().size() + static_cast<size_t>(c.get<1>());
        };
        auto func1 = make_partial_function(test_partial_func, 10, "hello"_s);
        auto func2 = make_partial_function(decl, 10, "hello"_s);
        auto res1  = func1(Tuple<String, int>{ "world"_s, 10 });
        auto res2  = func2(Tuple<String, int>{ "world"_s, 10 });
        RETURN_IF_NOT_EQ(res1, res2)
        RETURN_IF_NOT_EQ(res1, 30ull)
        RETURN_IF_NOT_EQ(res2, 30ull)
        RETURN_IF_NOT_EQ(res1, test_partial_func(10, "hello"_s, Tuple<String, int>{ "world"_s, 10 }))
        RETURN_IF_NOT_EQ(res2, decl(10, "hello"_s, Tuple<String, int>{ "world"_s, 10 }))
        return true;
    };
    auto fill_test = []() -> bool {
        Vector<String> vec{};
        fill_with(vec, 10, 5ull, 'a');
        RETURN_IF_NOT_EQ(vec.size(), 10ull)
        for (auto& str : vec) { RETURN_IF_NOT_EQ(str, "aaaaa"_s) }
        return true;
    };

    auto math_algos = []() -> bool {
        Vector<int> vec{};
        int j = 0;
        for (size_t i = 0; i < 100; i++, j++) vec.insert(99ull - i, j);
        sort(vec);
        for (const auto& [i, v] : Enumerate{ vec }) RETURN_IF_NOT_EQ(static_cast<int>(i), v)
        auto s = sum(vec, [](int a) { return a; });
        auto m = min(vec);
        auto x = max(vec);
        RETURN_IF_NOT_EQ(s, 4950)
        RETURN_IF_NOT_EQ(*m, 0)
        RETURN_IF_NOT_EQ(*x, 99)
        return true;
    };
    auto array_test = []() -> bool {
        Array<String, 3> arr{
            {"hello"_s, "world"_s, "testing"_s}
        };
        RETURN_IF_NOT_EQ(arr.size(), 3ull)
        RETURN_IF_NOT_EQ(arr[0], "hello"_s)
        RETURN_IF_NOT_EQ(arr[1], "world"_s)
        RETURN_IF_NOT_EQ(arr[2], "testing"_s)
        Array<String, 10> arr2{};
        for (auto& str : arr2) { RETURN_IF_NOT_EQ(str, ""_s) }
        arr2[5] = "this is a very long string eheheheh"_s;
        RETURN_IF_NOT_EQ(arr2[5], "this is a very long string eheheheh"_s)
        return true;
    };

    auto unique_str = []() -> bool {
        UniqueString s1{ "hello"_s };
        UniqueString s2{ "hello"_s };
        RETURN_IF_NOT_EQ(s1, s2)
        s2 = "other"_s;
        RETURN_IF_EQ(s1, s2)
        RETURN_IF_NOT_EQ(s1, "hello"_s)
        RETURN_IF_NOT_EQ(s2, "other"_s)
        return true;
    };

    auto string_test = []() -> bool {
        String str{};
        RETURN_IF_NOT_EQ(str, ""_sv)
        RETURN_IF_NOT_EQ(str.size(), 0ull)
        str.append("hello world");
        RETURN_IF_NOT_EQ(str, "hello world"_s)
        RETURN_IF_NOT_EQ(str.size(), strlen("hello world"))
        auto sub = str.substring(6);
        RETURN_IF_NOT_EQ(sub, "world"_s)
        StringView view{ str };
        RETURN_IF_NOT_EQ(str, view)
        RETURN_IF_NOT_EQ(sub.index_of('w'), 0ull)
        String repls = str.replace("l"_sv, "foo"_sv);
        RETURN_IF_NOT_EQ(repls, "hefoofooo worfood"_s)
        RETURN_IF_NOT_EQ(repls.replace("foo"_sv, "te"_sv), "heteteo worted"_s)
        return true;
    };

    auto format_test = []() -> bool {
        Vector<double> vec{ 1.0, 2.0, 3.0 };
        Map<String, int> map{};
        map.add("Hello"_s, 1);
        map.add("World"_s, 2);
        auto ret = Printer::format("My name is {}, I'm {} years old, vector: {}, map: {}", "Alessio"_s, 22, vec, map);
        RETURN_IF_NOT_EQ(
        ret,
        "My name is Alessio, I'm 22 years old, vector: [1.000000, 2.000000, 3.000000], map: { Hello: 1, World: 2 }"_s
        )
        return true;
    };

    auto variant_test = []() -> bool {
        String a{ "hello" };
        Variant<int, String, float> variant{};
        RETURN_IF_NOT_EQ(variant.is_active(), false)
        variant = a;
        RETURN_IF_NOT_EQ(variant.is_active(), true)
        RETURN_IF_NOT_EQ(variant.contains_type<String>(), true)
        RETURN_IF_NOT_EQ(variant.get<String>(), "hello"_s)
        variant = 10.0f;
        RETURN_IF_NOT_EQ(variant.contains_type<String>(), false)
        RETURN_IF_NOT_EQ(variant.contains_type<float>(), true)
        RETURN_IF_NOT_EQ(variant.get<float>(), 10.0f)
        variant = 10;
        RETURN_IF_NOT_EQ(variant.contains_type<float>(), false)
        RETURN_IF_NOT_EQ(variant.contains_type<int>(), true)
        RETURN_IF_NOT_EQ(variant.get<int>(), 10)
        Variant<Monostate> mono{};
        RETURN_IF_NOT_EQ(mono.is_active(), false)
        mono = Monostate{};
        RETURN_IF_NOT_EQ(mono.is_active(), true)
        RETURN_IF_NOT_EQ(mono.get<Monostate>(), Monostate{})
        return true;
    };
    ASSERT_TEST("String equality", streq, "hello"_s, "hello"_s)
    ASSERT_TEST("Vector equality", vec_test, Vector{ "hello"_s, "world"_s }, Vector{ "hello"_s, "world"_s })
    ASSERT_TEST("String length", strlen_test, "hello"_s, ARLib::strlen("hello"))
    ASSERT_TEST("String to integer/float conversions", charconv)
    ASSERT_TEST("Sorted vector correctness", sortedvec)
    ASSERT_TEST("Set correctness, uniqueness", set)
    ASSERT_TEST("Optional correctness", optional)
    ASSERT_TEST("Result correctness", result)
    ASSERT_TEST("Stack correctness", stack_test)
    ASSERT_TEST("Tuple correctness", tuple)
    ASSERT_TEST("PartialFunction correctness", partial_func)
    ASSERT_TEST("Fill algorihtm correctness", fill_test)
    ASSERT_TEST("Sum, min, max, sort algorithm correctness", math_algos)
    ASSERT_TEST("Array tests", array_test)
    ASSERT_TEST("Unique string test", unique_str)
    ASSERT_TEST("String tests", string_test)
    ASSERT_TEST("Formatting tests on strings", format_test)
    ASSERT_TEST("Variant correctness test", variant_test)
    return passed_count == test_count;
}
}    // namespace ARLib
