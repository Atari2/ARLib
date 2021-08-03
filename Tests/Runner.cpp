#include "Suite.h"
#include <gtest/gtest.h>

using namespace ARLib;

TEST(LegacyARLibTests, AllTests) {
    EXPECT_EQ(run_all_legacy_tests(), true);
}

TEST(ARLibTests, StringEquality) {
    EXPECT_EQ("hello"_s, "hello"_s);
}

TEST(ARLibTests, VectorEquality) {
    Vector a{"hello"_s, "world"_s};
    Vector b{"hello"_s, "world"_s};
    EXPECT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); i++) {
        EXPECT_EQ(a[i], b[i]);
    }
}

TEST(ARLibTests, Strlen) {
    EXPECT_EQ("hello"_s.size(), ARLib::strlen("hello"));
}

TEST(ARLibTests, HashMap) {
    HashMap<String, int> map{};
    auto val = map.add("hello"_s, 10);
    auto ex = map.add("world"_s, 20);
    EXPECT_EQ(val, InsertionResult::New);
    EXPECT_EQ(ex, InsertionResult::New);
    auto r = map["hello"_s];
    EXPECT_EQ(r, 10);
    val = map.add("hello"_s, 30);
    EXPECT_EQ(val, InsertionResult::Replace);
    r = map["hello"_s];
    EXPECT_EQ(r, 30);
    EXPECT_EQ(map.size(), 2ull);
    auto res = map.remove("hello"_s);
    EXPECT_EQ(res, DeletionResult::Success);
    res = map.remove("hello"_s);
    EXPECT_EQ(res, DeletionResult::Failure);
    EXPECT_EQ(map.size(), 1ull);
}

TEST(ARLibTests, CharConv) {
    String a{"1234"};
    String b{"123.123"};
    EXPECT_EQ(StrToInt(a), 1234);
    EXPECT_EQ(StrToFloat(b), 123.123f);
    int c = 101010;
    float d = 987.65f;
    EXPECT_EQ(IntToStr(c), "101010"_s);
    EXPECT_EQ(FloatToStr(d).substring(0, 6), "987.65"_s);
}

TEST(ARLibTests, SortedVec) {
    String a{"aaa"};
    String b{"bbb"};
    String c{"ccc"};
    SortedVector<String> sortedv{};
    sortedv.insert(c);
    sortedv.insert(b);
    sortedv.insert(a);
    EXPECT_EQ(sortedv.size(), 3ull);
    EXPECT_EQ(sortedv[0], "aaa"_s);
    EXPECT_EQ(sortedv[1], "bbb"_s);
    EXPECT_EQ(sortedv[2], "ccc"_s);
}

TEST(ARLibTests, SetTests) {
    Set<int> s{};
    EXPECT_EQ(s.insert(10), true);
    EXPECT_EQ(s.insert(20), true);
    EXPECT_EQ(s.insert(10), false);
    EXPECT_EQ(s.remove(20), true);
    EXPECT_EQ(s.remove(20), false);
}

TEST(ARLibTests, OptionalTests) {
    Optional<String> opt{};
    EXPECT_EQ(opt.empty(), true);
    opt.emplace("hello", 5ull);
    EXPECT_EQ(opt.has_value(), true);
    delete opt.detach();
    EXPECT_EQ(opt.empty(), true);
}

TEST(ARLibTests, ResulTests) {
    Result<String> res{"hello"_s};
    EXPECT_EQ(res.is_ok(), true);
    EXPECT_EQ(res.is_error(), false);
    String b = res.to_ok();
    EXPECT_EQ(b, "hello"_s);
    auto res2 = Result<String>::from_error();
    EXPECT_EQ(res2.is_ok(), false);
    EXPECT_EQ(res2.is_error(), true);
}

TEST(ARLibTests, StackTests) {
    Stack<String> stack{};
    EXPECT_EQ(stack.size(), 0ull);
    stack.push("hello"_s);
    stack.push("world"_s);
    stack.push("testing"_s);
    EXPECT_EQ(stack.size(), 3ull);
    EXPECT_EQ(stack.peek(), "testing"_s);
    EXPECT_EQ(stack.pop(), "testing"_s);
    EXPECT_EQ(stack.peek(), "world"_s);
    EXPECT_EQ(stack.pop(), "world"_s);
    EXPECT_EQ(stack.size(), 1ull);
}

TEST(ARLibTests, TupleTests) {
    Tuple<int, String, double, Vector<String>> tup{0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}};
    Tuple<int, String, double, Vector<String>> tup3{0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}};
    EXPECT_EQ(tup, tup3);
    EXPECT_EQ(tup.get<0>(), 0);
    EXPECT_EQ(tup.get<1>(), "hello"_s);
    EXPECT_EQ(tup.get<2>(), 10.0);
    EXPECT_EQ(tup.get<3>()[0], "a"_s);
    EXPECT_EQ(tup.get<3>().size(), 3ull);
    tup.get<3>().push_back("k"_s);
    EXPECT_EQ(tup.get<3>().size(), 4ull);
    tup.set_typed("world"_s);
    tup.set_typed(54.4);
    EXPECT_EQ(tup.get<1>(), "world"_s);
    EXPECT_EQ(tup.get<2>(), 54.4);
    auto tup_2 = move(tup);
    EXPECT_EQ(tup_2.get<0>(), 0);
    EXPECT_EQ(tup_2.get<1>(), "world"_s);
    EXPECT_EQ(tup_2.get<2>(), 54.4);
    EXPECT_EQ(tup_2.get<3>()[3], "k"_s);
    EXPECT_EQ(tup_2.get<3>().size(), 4ull);
}

TEST(ARLibTests, PartialFuncTests) {
    auto decl = [](int a, String b, Tuple<String, int> c) {
        return static_cast<size_t>(a) + b.size() + c.get<0>().size() + static_cast<size_t>(c.get<1>());
    };
    PartialFunction func1{test_partial_func, 10, "hello"_s};
    PartialFunction func2{decl, 10, "hello"_s};
    auto res1 = func1(Tuple<String, int>{"world"_s, 10});
    auto res2 = func2(Tuple<String, int>{"world"_s, 10});
    EXPECT_EQ(res1, res2);
    EXPECT_EQ(res1, 30ull);
    EXPECT_EQ(res2, 30ull);
    EXPECT_EQ(res1, test_partial_func(10, "hello"_s, {"world"_s, 10}));
    EXPECT_EQ(res2, decl(10, "hello"_s, {"world"_s, 10}));
}

TEST(ARLibTests, FillTest) {
    Vector<String> vec{};
    fill_with<String, Vector<String>>(vec, 10, 5ull, 'a');
    EXPECT_EQ(vec.size(), 10ull);
    for (auto& str : vec) {
        EXPECT_EQ(str, "aaaaa"_s);
    }
}

TEST(ARLibTests, MathAlgorithmTest) {
    Vector<int> vec{};
    int j = 0;
    for (size_t i = 0; i < 100; i++, j++)
        vec.insert(99ull - i, j);
    sort(vec);
    for (const auto& [i, v] : Enumerate{vec})
        EXPECT_EQ(static_cast<int>(i), v);
    auto s = sum(vec, [](int a) { return a; });
    auto m = min(vec);
    auto x = max(vec);
    auto copy = transform(vec, [](int a) { return a * 2; });
    auto cs = sum(copy, [](int a) { return a; });
    auto cm = min(copy);
    auto cx = max(copy);
    EXPECT_EQ(cs, 9900);
    EXPECT_EQ(*cm, 0);
    EXPECT_EQ(*cx, 99 * 2);
    EXPECT_EQ(s, 4950);
    EXPECT_EQ(*m, 0);
    EXPECT_EQ(*x, 99);
}

TEST(ARLibTests, ArrayTest) {
    Array<String, 3> arr{{"hello"_s, "world"_s, "testing"_s}};
    EXPECT_EQ(arr.size(), 3ull);
    EXPECT_EQ(arr[0], "hello"_s);
    EXPECT_EQ(arr[1], "world"_s);
    EXPECT_EQ(arr[2], "testing"_s);
    Array<String, 10> arr2{};
    for (auto& str : arr2) {
        EXPECT_EQ(str, ""_s);
    }
    arr2[5] = "this is a very long string eheheheh"_s;
    EXPECT_EQ(arr2[5], "this is a very long string eheheheh"_s);
}

TEST(ARLibTests, UniqueStr) {
    UniqueString s1{"hello"_s};
    UniqueString s2{"hello"_s};
    EXPECT_EQ(s1, s2);
    s2 = "other"_s;
    EXPECT_NE(s1, s2);
    EXPECT_EQ(s1, "hello"_s);
    EXPECT_EQ(s2, "other"_s);
}

TEST(ARLibTests, StringTest) {
    String str{};
    EXPECT_EQ(str, ""_sv);
    EXPECT_EQ(str.size(), 0ull);
    str.concat("hello world");
    EXPECT_EQ(str, "hello world"_s);
    EXPECT_EQ(str.size(), ARLib::strlen("hello world"));
    auto sub = str.substring(6);
    EXPECT_EQ(sub, "world"_s);
    StringView view{str};
    EXPECT_EQ(str, view);
    EXPECT_EQ(sub.index_of('w'), 0ull);
    String repls = str.replace("l", "foo");
    EXPECT_EQ(repls, "hefoofooo worfood"_s);
    EXPECT_EQ(repls.replace("foo", "te"), "heteteo worted"_s);
}

TEST(ARLibTests, FormatTest) {
    Vector<double> vec{1.0, 2.0, 3.0};
    Map<String, int> map{};
    map.add("Hello"_s, 1);
    map.add("World"_s, 2);
    auto ret = Printer::format("My name is {}, I'm {} years old, vector: {}, map: {}", "Alessio"_s, 22, vec, map);
    EXPECT_EQ(
    ret,
    "My name is Alessio, I'm 22 years old, vector: [1.000000], [2.000000], [3.000000], map: { Hello: 1, World: 2 }"_s);
}

TEST(ARLibTests, VariantTests) {
    String a{"hello"};
    Variant<int, String, float> variant{};
    EXPECT_EQ(variant.is_active(), false);
    variant = a;
    EXPECT_EQ(variant.is_active(), true);
    EXPECT_EQ(variant.contains_type<String>(), true);
    EXPECT_EQ(variant.get<String>(), "hello"_s);
    variant = 10.0f;
    EXPECT_EQ(variant.contains_type<String>(), false);
    EXPECT_EQ(variant.contains_type<float>(), true);
    EXPECT_EQ(variant.get<float>(), 10.0f);
    variant = 10;
    EXPECT_EQ(variant.contains_type<float>(), false);
    EXPECT_EQ(variant.contains_type<int>(), true);
    EXPECT_EQ(variant.get<int>(), 10);
    Variant<Monostate> mono{};
    EXPECT_EQ(mono.is_active(), false);
    mono = Monostate{};
    EXPECT_EQ(mono.is_active(), true);
    EXPECT_EQ(mono.get<Monostate>(), Monostate{});
}