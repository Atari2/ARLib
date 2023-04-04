#include "Suite.h"
#include <gtest/gtest.h>
#include "GTestPrintHelpers.h"

using namespace ARLib;
template <typename T>
auto test_tuple_get(const Tuple<T, String>& tuple) {
    return get<T>(tuple);
}
template <typename T>
void test_tuple_set(Tuple<T, String>& tuple, T value) {
    set<T>(tuple, value);
}
template <typename... Args>
auto test_tuple_get_by_idx(const Tuple<Args...>& tuple) {
    return get<0>(tuple);
}
template <typename... Args>
void test_tuple_set_by_idx(Tuple<Args...>& tuple, auto value) {
    set<0>(tuple, value);
}
TEST(LegacyARLibTests, AllTests) {
    EXPECT_TRUE(run_all_legacy_tests());
}
TEST(ARLibTests, StringEquality) {
    EXPECT_EQ("hello"_s, "hello"_s);
}
TEST(ARLibTests, VectorEquality) {
    Vector a{ "hello"_s, "world"_s };
    Vector b{ "hello"_s, "world"_s };
    EXPECT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); i++) { EXPECT_EQ(a[i], b[i]); }
}
TEST(ARLibTests, Strlen) {
    EXPECT_EQ("hello"_s.size(), ARLib::strlen("hello"));
}
TEST(ARLibTests, CharConv) {
    String a{ "1234" };
    String b{ "123.123" };

    constexpr auto expected   = NumberTraits<ARLib::int64_t>::max;
    constexpr auto expected_u = NumberTraits<ARLib::uint64_t>::max;

    constexpr StringView s_hex{ "-0x7fffffffffffffff" };
    constexpr auto res_hex = cxpr::StrViewToI64(s_hex, 16);
    static_assert(res_hex == -expected, "StrViewToI64 failed with base 16");

    constexpr StringView s_uhex{ "0xffffffffffffffff" };
    constexpr auto res_uhex = cxpr::StrViewToU64(s_uhex, 16);
    static_assert(res_uhex == expected_u, "StrViewToU64 failed with base 16");

    constexpr StringView s_oct{ "0o137726051051" };
    constexpr auto res_oct = cxpr::StrViewToI64(s_oct, 8);
    static_assert(res_oct == 12873912873, "StrViewToI64 failed with base 8");

    constexpr StringView s_dec{ "   -123809" };
    constexpr auto res_dec = cxpr::StrViewToI64(s_dec);
    static_assert(res_dec == -123809, "StrViewToI64 failed with base 10");

    constexpr StringView s_bin{ "0b111111111111111111111111111111111111111111111111111111111111111" };
    constexpr auto res_bin = cxpr::StrViewToI64(s_bin, 2);
    static_assert(res_bin == expected, "StrViewToI64 failed with base 2");

    constexpr StringView s_4{ "  1233   " };
    constexpr auto res_4 = cxpr::StrViewToI64(s_4, 4);
    static_assert(res_4 == 111, "StrViewToI64 failed with base 4");

    constexpr StringView s_15{ "1233" };
    constexpr auto res_15 = cxpr::StrViewToI64(s_15, 15);
    static_assert(res_15 == 3873, "StrViewToI64 failed with base 4");

    EXPECT_EQ(StrToUInt(a), Result{ 1234_u32 });
    EXPECT_EQ(StrToFloat(b), Result{ 123.123f });
    int c   = 101010;
    float d = 987.65f;
    EXPECT_EQ(IntToStr(c), "101010"_s);
    EXPECT_EQ(FloatToStr(d).substring(0, 6), "987.65"_s);
}
TEST(ARLibTests, SortedVec) {
    String a{ "aaa" };
    String b{ "bbb" };
    String c{ "ccc" };
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
    EXPECT_EQ(s.insert(10), 10);
    EXPECT_EQ(s.insert(20), 20);
    EXPECT_EQ(s.insert(10), 10);
    EXPECT_EQ(s.size(), 2);
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
    auto str  = Optional<String>{}.value_or("Hello world"_s);
    auto str2 = Optional<String>{ "hello cpp"_s }.value_or("shouldn't get returned"_s);
    EXPECT_EQ(str, "Hello world"_s);
    EXPECT_EQ(str2, "hello cpp"_s);
}
TEST(ARLibTests, ResulTests) {
    Result<String> res{ "hello"_s, emplace_ok };
    EXPECT_EQ(res.is_ok(), true);
    EXPECT_EQ(res.is_error(), false);
    String b = res.to_ok();
    EXPECT_EQ(b, "hello"_s);
    auto res2 = Result<String>{ "error"_s, emplace_error };
    EXPECT_EQ(res2.is_ok(), false);
    EXPECT_EQ(res2.is_error(), true);
    res2.ignore_error();
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
    Tuple<int, String, double, Vector<String>> tup{
        0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}
    };
    Tuple<int, String, double, Vector<String>> tup3{
        0, "hello"_s, 10.0, Vector{"a"_s, "b"_s, "c"_s}
    };
    Tuple<int, String> tup4{ 110, "hello"_s };

    EXPECT_EQ(test_tuple_get(tup4), 110);
    test_tuple_set(tup4, 120);
    EXPECT_EQ(test_tuple_get(tup4), 120);

    EXPECT_EQ(test_tuple_get_by_idx(tup4), 120);
    test_tuple_set_by_idx(tup4, 130);
    EXPECT_EQ(test_tuple_get_by_idx(tup4), 130);

    EXPECT_EQ(tup, tup3);
    EXPECT_EQ(tup.get<0>(), 0);
    EXPECT_EQ(tup.get<1>(), "hello"_s);
    EXPECT_EQ(tup.get<2>(), 10.0);
    EXPECT_EQ(tup.get<3>()[0], "a"_s);
    EXPECT_EQ(tup.get<3>().size(), 3ull);
    tup.get<3>().push_back("k"_s);
    EXPECT_EQ(tup.get<3>().size(), 4ull);
    tup = "world"_s;
    tup = 54.4;
    EXPECT_EQ(tup.get<1>(), "world"_s);
    EXPECT_EQ(tup.get<2>(), 54.4);
    auto&& tup_2 = move(tup);
    EXPECT_EQ(tup_2.get<int>(), 0);
    EXPECT_EQ(tup_2.get<String>(), "world"_s);
    EXPECT_EQ(tup_2.get<double>(), 54.4);
    EXPECT_EQ(tup_2.get<Vector<String>>()[3], "k"_s);
    EXPECT_EQ(tup_2.get<Vector<String>>().size(), 4ull);
}
TEST(ARLibTests, PartialFuncTests) {
    auto decl = [](int a, const String& b, Tuple<String, int> c) {
        return static_cast<size_t>(a) + b.size() + c.get<0>().size() + static_cast<size_t>(c.get<1>());
    };
    auto decl2 = [](int a, int b) {
        return a + b;
    };
    auto func3 = make_partial_function(decl2, 10);
    auto func1 = make_partial_function(test_partial_func, 10, "hello"_s);
    auto func2 = make_partial_function(decl, 10, "hello"_s);
    auto res1  = func1(Tuple<String, int>{ "world"_s, 10 });
    auto res2  = func2(Tuple<String, int>{ "world"_s, 10 });
    EXPECT_EQ(res1, res2);
    EXPECT_EQ(res1, 30ull);
    EXPECT_EQ(res2, 30ull);
    EXPECT_EQ(res1, test_partial_func(10, "hello"_s, Tuple<String, int>{ "world"_s, 10 }));
    EXPECT_EQ(res2, decl(10, "hello"_s, Tuple<String, int>{ "world"_s, 10 }));
    EXPECT_EQ(func3(10), 20);
}
TEST(ARLibTests, FillTest) {
    Vector<String> vec{};
    fill_with<String, Vector<String>>(vec, 10, 5ull, 'a');
    EXPECT_EQ(vec.size(), 10ull);
    for (auto& str : vec) { EXPECT_EQ(str, "aaaaa"_s); }
}
TEST(ARLibTests, MathAlgorithmTest) {
    Vector<int> vec{};
    int j = 0;
    for (size_t i = 0; i < 100; i++, j++) vec.insert(99ull - i, j);
    sort(vec);
    for (const auto& [i, v] : Enumerate{ vec }) EXPECT_EQ(static_cast<int>(i), v);
    auto s    = sum(vec, [](int a) { return a; });
    auto m    = min(vec);
    auto x    = max(vec);
    auto copy = transform(vec, [](int a) { return a * 2; });
    auto cs   = sum(copy, [](int a) { return a; });
    auto cm   = min(copy);
    auto cx   = max(copy);
    EXPECT_EQ(cs, 9900);
    EXPECT_EQ(*cm, 0);
    EXPECT_EQ(*cx, 99 * 2);
    EXPECT_EQ(s, 4950);
    EXPECT_EQ(*m, 0);
    EXPECT_EQ(*x, 99);
    auto res  = avg(vec);
    auto res2 = avg(vec.begin(), vec.end());
    EXPECT_EQ(res, res2);
    EXPECT_EQ(res, 49.5);
    EXPECT_EQ(res2, 49.5);
    EXPECT_EQ(avg(vec), avg(vec.begin(), vec.end()));
    auto res_rounded  = avg<decltype(vec), true>(vec);
    auto res2_rounded = avg<decltype(vec.begin()), true>(vec.begin(), vec.end());
    EXPECT_EQ(res_rounded, res2_rounded);
    EXPECT_EQ(res_rounded, 49);
    EXPECT_EQ(res2_rounded, 49);
}
TEST(ARLibTests, ArrayTest) {
    Array arr{ "hello"_s, "world"_s, "testing"_s };
    EXPECT_EQ(arr.size(), 3ull);
    EXPECT_EQ(arr[0], "hello"_s);
    EXPECT_EQ(arr[1], "world"_s);
    EXPECT_EQ(arr[2], "testing"_s);
    Array<String, 10> arr2{};
    for (auto& str : arr2) { EXPECT_EQ(str, ""_s); }
    arr2[5] = "this is a very long string eheheheh"_s;
    EXPECT_EQ(arr2[5], "this is a very long string eheheheh"_s);
}
TEST(ARLibTests, UniqueStr) {
    UniqueString s1{ "hello"_s };
    UniqueString s2{ "hello"_s };
    EXPECT_EQ(s1, s2);
    s2 = "other"_s;
    EXPECT_NE(s1, s2);
    EXPECT_EQ(s1, "hello"_s);
    EXPECT_EQ(s2, "other"_s);
    s2->append("help");
    EXPECT_EQ(s2, "otherhelp"_s);
}
TEST(ARLibTests, StringTest) {
    String str{};
    EXPECT_EQ(str, ""_sv);
    EXPECT_EQ(str.size(), 0ull);
    str.append("hello world");
    EXPECT_EQ(str, "hello world"_s);
    EXPECT_EQ(str.size(), ARLib::strlen("hello world"));
    auto sub = str.substring(6);
    EXPECT_EQ(sub, "world"_s);
    StringView view{ str };
    EXPECT_EQ(str, view);
    EXPECT_EQ(sub.index_of('w'), 0ull);
    String repls = str.replace("l"_sv, "foo"_sv);
    EXPECT_EQ(repls, "hefoofooo worfood"_s);
    EXPECT_EQ(repls.replace("foo"_sv, "te"_sv), "heteteo worted"_s);
}
TEST(ARLibTests, StringViewTests) {
    constexpr StringView view = "hello world"_sv;
    auto vec                  = view.split();
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "hello"_sv);
    EXPECT_EQ(vec[1], "world"_sv);
    constexpr char c = view[7];
    static_assert(c == 'o');    // this needs to compile
    static_assert(view == "hello world"_sv);
    static_assert(view == "hello world");
}
TEST(ARLibTests, FormatTest) {
    Vector<double> vec{ 1.0, 2.0, 3.0 };
    Map<String, int> map{};
    map.add("Hello"_s, 1);
    map.add("World"_s, 2);
    auto ret = Printer::format("My name is {{}}, I'm {} years old, vector: {}, map: {}", 22, vec, map);
    EXPECT_EQ(
    ret, "My name is {}, I'm 22 years old, vector: [1.000000, 2.000000, 3.000000], map: { Hello: 1, World: 2 }"_s
    );
    auto ret2 = Printer::format("{{}}");
    EXPECT_EQ(ret2, "{}"_s);
    auto ret3 = Printer::format("{}", nullptr);
    EXPECT_EQ(ret3, "nullptr"_s);
}
TEST(ARLibTests, VariantTests) {
    String a{ "hello" };
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
TEST(ARLibTests, SSOVectorTests) {
    SSOVector vec{ "hello"_s, "world"_s };
    EXPECT_TRUE(vec.is_in_situ());
    auto sz = vec.size();
    for (size_t i = 0; i < vec.sso() - sz; i++) { vec.push_back("a"_s); }
    EXPECT_TRUE(vec.is_in_situ());
    vec.push_back("b"_s);
    EXPECT_FALSE(vec.is_in_situ());
    EXPECT_EQ(vec[0], "hello"_s);
    SSOVector<int, 25> sso1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    SSOVector<int, 100> sso2{};
    sso2 = sso1;
    EXPECT_EQ(sso1.size(), sso2.size());
    for (size_t i = 0; i < sso1.size(); i++) EXPECT_EQ(sso1[i], sso2[i]);
}
TEST(ARLibTests, LinkedSetTests) {
    LinkedSet lset{ "hello"_s, "hello"_s };
    EXPECT_EQ(lset.size(), 1ull);
    auto& lhs = lset.append("hello"_s);
    EXPECT_EQ(lhs, "hello"_s);
    EXPECT_EQ(lset.size(), 1ull);
    auto& rhs = lset.prepend("world"_s);
    EXPECT_EQ(rhs, "world"_s);
    EXPECT_EQ(lset.size(), 2ull);
    auto& ths = lset.prepend("world"_s);
    EXPECT_EQ(ths, "world"_s);
    EXPECT_EQ(lset.size(), 2ull);
    lset.remove(lset.find("hello"_s).current());
    EXPECT_EQ(lset.size(), 1ull);
    lset.prepend("hello"_s);
    EXPECT_EQ(lset.size(), 2ull);
}
TEST(ARLibTests, LinkedListTests) {
    LinkedList list{ 1, 2, 3, 4 };
    EXPECT_EQ(list.size(), 4ull);
    list.prepend(5);
    list.append(6);
    EXPECT_EQ(list.size(), 6ull);
    EXPECT_EQ(list.find(7), list.end());
    list.remove(6);
    EXPECT_EQ(list.size(), 5ull);
    int i = 5;
    for (auto val : list) {
        EXPECT_EQ(val, i);
        i--;
    }
    EXPECT_EQ(list.pop(), 1);
    EXPECT_EQ(list.size(), 4ull);
    LinkedList list2{ "hello"_s };
    EXPECT_EQ(list2.pop(), "hello"_s);
}
TEST(ARLibTests, GenericViewTests) {
    Vector<int> vec{};
    vec.fill_pattern([](int i) { return ++i; }, 1000);
    const Vector veccp{ vec };
    IteratorView view{ vec };
    IteratorView view2{ veccp };
    String form_filtered{
        R"(["112", "128", "144", "160", "176", "192", "208", "224", "240", "256", "272", "288", "304", "320", "336", "352", "368", "384", "400", "416", "432", "448", "464", "480", "496", "512", "528", "544", "560", "576", "592", "608", "624", "640", "656", "672", "688", "704", "720", "736", "752", "768", "784", "800", "816", "832", "848", "864", "880", "896", "912", "928", "944", "960", "976", "992"])"_s
    };
    view.inplace_transform([](int a) { return a * 2; });
    for (auto [index, item] : Enumerate{ vec }) { EXPECT_EQ(item, index * 2); }
    auto vec2 = view.map([](int) { return 0; }).collect<Vector<int>>();
    auto vec3 = view.map([](int a) { return IntToStr(a); }).collect<Vector<String>>();
    for (const auto& [index, item] : Enumerate{ vec3 }) { EXPECT_EQ(item, IntToStr(index * 2)); }
    for (auto item : vec2) { EXPECT_EQ(item, 0); }
    auto filtered = view2.map([](int a) { return a * 2; }
    ).map([](int b) {
         return b * 2;
     }).map([](int c) {
           return IntToStr(c * 4);
       }).filter([](const String& str) {
             return str.size() == 3;
         }).collect<Vector<String>>();
    EXPECT_EQ(filtered.size(), 56ull);
    EXPECT_EQ(Printer::format("{}", filtered), form_filtered);

    Array expected_from_en{
        Pair{0_sz,  1.0},
        Pair{ 1_sz, 2.0},
        Pair{ 2_sz, 3.0},
        Pair{ 3_sz, 4.0},
        Pair{ 4_sz, 5.0}
    };
    for (const auto& [exp, act] : "1\n2\n3\nasdf\n4\n\n5"_sv.split("\n")
                                  .iter()
                                  .filter(&StringView::size)
                                  .map(StrViewToDouble)
                                  .filter([](auto&& res) {
                                      if (res.is_error()) { res.ignore_error(); }
                                      return res.is_ok();
                                  })
                                  .map(&Result<double>::to_ok)
                                  .enumerate()
                                  .zip(expected_from_en)) {
        const auto& [exp_i, exp_v] = exp;
        const auto& [act_i, act_v] = act;
        EXPECT_EQ(exp_i, act_i);
        EXPECT_EQ(exp_v, act_v);
    }
}
TEST(ARLibTests, StringTest2) {
    String str{ "ciao come ciao io ciao sono ciao pippo" };
    String str2{ "ciao ciao" };
    EXPECT_EQ(str2.last_index_not_of("ciao"_sv, str2.size() - 1), 4ull);
    auto ret  = str.split("ciao");
    auto retv = str.split_view("ciao");
    Vector vec{ ""_s, " come "_s, " io "_s, " sono "_s, " pippo"_s };
    EXPECT_EQ(ret, vec);
    EXPECT_EQ(retv, vec);
    EXPECT_EQ(str.last_index_of_any("po"_sv), str.size() - 1);
    str2 += 'c';
    EXPECT_EQ(str2, "ciao ciaoc"_s);
}
TEST(ARLibTests, PartialFuncTest2) {
    auto func = [](int a, int b) {
        return a + b;
    };
    auto partial = make_partial_function(func, 10);
    Map<ARLib::String, decltype(partial)> map{};
    map.add("hello"_s, partial);
    map.add("world"_s, partial);
    EXPECT_EQ(map["hello"_s](10), 20);
    EXPECT_EQ(map["world"_s](20), 30);
    EXPECT_EQ(map["hello"_s](30), 40);
}
TEST(ARLibTests, FunctionalTest) {
    struct TestStruct {
        static bool ret() { return true; }
        bool other_ret(bool b) { return b; }
    };
    TestStruct st{};

    auto func = [](int a, int b) {
        return a + b;
    };

    Function<int(int, int)> fn{ func };
    Function<bool(void)> fn2{ TestStruct::ret };
    Function<bool(TestStruct*, bool)> fn3{ &TestStruct::other_ret };
    EXPECT_EQ(fn(10, 10), 20);
    EXPECT_EQ(fn2(), true);
    EXPECT_EQ(fn3(&st, false), false);
}
TEST(ARLibTests, MoreFormatTests) {
    auto map_print         = R"([{ hello: 10, cap: 10, world: 20 }, {}, {}])"_s;
    auto vec_of_vecs_print = R"([["hello", "world"], ["name"], ["cap"], [], [], [], [], [], [], []])"_s;
    auto mat_print =
    "[\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]\n]"_s;
    auto row_print = R"([0, 0, 0, 0, 0, 0, 0, 0, 0, 0])"_s;
    int mat[10][10]{};
    decltype(auto) val = mat[0];
    Map<String, int> map[3]{
        {{ "hello"_s, 10 }, { "cap"_s, 10 }, { "world"_s, 20 }}
    };
    Vector<String> vec[10] = {
        Vector{ "hello"_s, "world"_s },
        Vector{ "name"_s },
        Vector{ "cap"_s }
    };
    EXPECT_EQ(Printer::format("{}", map), map_print);
    EXPECT_EQ(Printer::format("{}", vec), vec_of_vecs_print);
    EXPECT_EQ(Printer::format("{}", val), row_print);
    EXPECT_EQ(Printer::format("{}", mat), mat_print);
    auto data = vec[0][0].data();
    EXPECT_EQ(Printer::format("{}", data), "hello"_s);
}
TEST(ARLibTests, ContainerAlgoTest) {
    Vector<int> vec1{ 1, 2, 3, 4, 5, 6 };
    Vector<String> vec2{ "hello"_s, "world"_s, "why"_s };
    FlatMap<String, Vector<int>> map{
        {"hello"_s,  Vector<int>{ 1, 2, 3, 4, 5 } },
        { "world"_s, Vector<int>{ 6, 7, 8, 9, 10 }}
    };
    EXPECT_EQ(all_of(vec1, [](int a) { return a < 7; }), true);
    EXPECT_EQ(any_of(vec1, [](int a) { return a == 3; }), true);
    EXPECT_EQ(
    exactly_n(
    vec1, [](int a) { return a < 4; }, 3
    ),
    true
    );
    EXPECT_EQ(all_of(vec2, [](const String& str) { return str == "hello"_s; }), false);
    EXPECT_EQ(any_of(vec2, [](const String& str) { return str == "hello"_s; }), true);
    EXPECT_EQ(
    exactly_n(
    vec2, [](const String& str) { return str == "hello"_s; }, 1
    ),
    true
    );
    EXPECT_EQ(
    all_of(
    map,
    [](const auto& entry) {
        auto b = entry.key() == "hello"_s || entry.key() == "world"_s;
        return b && all_of(entry.val(), [](int a) { return a < 11; });
    }
    ),
    true
    );
}
#ifndef DISABLE_THREADING
TEST(ARLibTests, ThreadingTests) {
    auto func = [](int val, String help) {
        EXPECT_EQ(val, 30);
        EXPECT_EQ(help, "hello world"_s);
    };

    Thread t{};
    EXPECT_FALSE(t.joinable());
    t = move(Thread{ func, 30, "hello world"_s });
    EXPECT_TRUE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());
    Mutex m1{};
    Mutex m2{};
    Mutex m3{};
    { ScopedLock lock{ m1, m2, m3 }; }
    RecursiveMutex m4{};
    { UniqueLock ll{ m4 }; }
}
TEST(ARLibTests, EventLoop) {
    auto func = [](int val, String help) {
        EXPECT_EQ(val, 30);
        EXPECT_EQ(help, "hello world"_s);
    };
    EventLoop loop{};
    EXPECT_FALSE(loop.running());
    loop.subscribe_callback(func, 30, "hello world"_s);
    loop.join();
}
#endif
TEST(ARLibTests, ChronoTest) {
    auto now = Clock::now();
    String s{};
    for (int i = 0; i < 100; i++) { s.append('c'); }
    auto new_now = Clock::now();
    EXPECT_GT(new_now, now);
    auto diff = Clock::diff(now, new_now);
    EXPECT_GT(diff, 0);
}
TEST(ARLibTests, JSONTest) {
    auto maybe_obj = JSON::Parser::parse(R"({"hello world": 10, "array": [1, 2, 3, 4]})"_sv);
    EXPECT_TRUE(maybe_obj.is_ok());
    auto obj = maybe_obj.to_ok();
    Vector vec{ 1, 2, 3, 4 };
    auto& ptr_vec = obj["array"_s].get<JSON::Type::JArray>();
    EXPECT_EQ(vec.size(), ptr_vec.size());
    for (size_t i = 0; i < vec.size(); i++) {
        EXPECT_EQ(vec[i], ptr_vec[i]);
        ptr_vec[i] = 10.0;
    }
    EXPECT_EQ(obj["hello world"_s], 10);
    auto maybe_obj_2 = JSON::Parser::parse(R"({"hello world": 10, "array: [1, 2, 3, 4]})"_sv);
    EXPECT_FALSE(maybe_obj_2.is_ok());
    auto err = maybe_obj_2.to_error();
    EXPECT_EQ(err.message(), "Missing end of quotation on string"_s);
    EXPECT_EQ(err.offset(), 41);
}
TEST(ARLibTests, RandomTest) {
    EXPECT_EQ(Random::PCG::random_s(), 355248013);
    auto pcg = Random::PCG::create();
    EXPECT_NE(pcg.random(), 355248013);
}
TEST(ARLibTests, TreeTest) {
    Tree<int> tree{};
    EXPECT_FALSE(tree.head().exists());
    tree.insert_leaf(10);
    tree.insert_leaf(5);
    tree.insert_leaf(100);
    tree.insert_leaf(2);
    tree.insert_leaf(7);
    tree.insert_leaf(9);
    tree.insert_leaf(200);
    tree.insert_leaf(11);
    tree.insert_leaf(6);
    tree.remove(7);
    EXPECT_EQ(tree.head()->value(), 10);
    EXPECT_TRUE(tree.find(10).exists());
    tree.remove(10);
    EXPECT_FALSE(tree.find(10).exists());
    EXPECT_EQ(tree.head()->value(), 9);
}
TEST(ARLibTests, BigIntTest) {
    auto a = BigInt{ "12389123908"_s };
    auto b = BigInt{ "-983458171238123"_s };
    auto c = BigInt{ "875679183741987"_s };
    auto d = BigInt{ "-1238767812763"_s };

    auto f = BigInt{ 1234 };
    auto g = BigInt{ 4321 };
    auto h = BigInt{ -1234 };
    auto i = BigInt{ -4321 };

    f -= g;
    h -= i;

    EXPECT_EQ(f, -3087);
    EXPECT_EQ(h, 3087);

    auto result_diff_ab = BigInt{ "983470560362031"_s };
    auto result_diff_ba = BigInt{ "-983470560362031"_s };
    auto result_diff_ac = BigInt{ "-875666794618079"_s };
    auto result_diff_ca = BigInt{ "875666794618079"_s };
    auto result_diff_bd = BigInt{ "-982219403425360"_s };
    auto result_diff_db = BigInt{ "982219403425360"_s };

    auto result_sum_ab = BigInt{ "-983445782114215"_s };
    auto result_sum_ba = BigInt{ "-983445782114215"_s };
    auto result_sum_ac = BigInt{ "875691572865895"_s };
    auto result_sum_ca = BigInt{ "875691572865895"_s };
    auto result_sum_bd = BigInt{ "-984696939050886"_s };
    auto result_sum_db = BigInt{ "-984696939050886"_s };

    EXPECT_EQ(a - b, result_diff_ab);
    EXPECT_EQ(b - a, result_diff_ba);
    EXPECT_EQ(a - c, result_diff_ac);
    EXPECT_EQ(c - a, result_diff_ca);
    EXPECT_EQ(b - d, result_diff_bd);
    EXPECT_EQ(d - b, result_diff_db);

    EXPECT_EQ(a + b, result_sum_ab);
    EXPECT_EQ(b + a, result_sum_ba);
    EXPECT_EQ(a + c, result_sum_ac);
    EXPECT_EQ(c + a, result_sum_ca);
    EXPECT_EQ(b + d, result_sum_bd);
    EXPECT_EQ(d + b, result_sum_db);

    auto f2 = BigInt{ "123456781234567891234879169467981276392189732178937891237928173981239812219873218973"_s };
    auto g2 = BigInt{ "12837127389712389123891738127317892312987389217"_s };
    auto div_result = BigInt{ "9617165701222654353349541990196129976"_s };

    EXPECT_EQ(div_result, f2 / g2);
}
TEST(ARLibTests, HashFuncTests) {
    {
        // CRC32
        auto zerolength = CRC32::calculate(""_s);
        EXPECT_EQ(zerolength, 0x00);
        auto ff   = CRC32::calculate(Array{ 0xFF_u8 });
        auto zero = CRC32::calculate(Array{ 0x00_u8 });
        EXPECT_EQ(ff, 0xFF000000);
        EXPECT_EQ(zero, 0xD202EF8D);
        auto some_str = CRC32::calculate("123456789"_s);
        EXPECT_EQ(some_str, 0xCBF43926);
    }
    {
        // MD5
        auto zerolength          = MD5::calculate(""_s);
        auto zerolength_expected = "D41D8CD98F00B204E9800998ECF8427E"_s;
        EXPECT_EQ(PrintInfo{ zerolength }.repr(), zerolength_expected);

        auto some_str          = MD5::calculate("The quick brown fox jumps over the lazy dog"_s);
        auto some_str_expected = "9E107D9D372BB6826BD81D3542A419D6"_s;
        EXPECT_EQ(PrintInfo{ some_str }.repr(), some_str_expected);

        auto some_other_str          = MD5::calculate("The quick brown fox jumps over the lazy dog."_s);
        auto some_other_str_expected = "E4D909C290D0FB1CA068FFADDF22CBD0"_s;
        EXPECT_EQ(PrintInfo{ some_other_str }.repr(), some_other_str_expected);
    }
    {
        // SHA1
        auto zerolength          = SHA1::calculate(""_s);
        auto zerolength_expected = "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709"_s;
        EXPECT_EQ(PrintInfo{ zerolength }.repr(), zerolength_expected);

        auto some_str          = SHA1::calculate("Cantami o diva del pelide Achille l'ira funesta"_s);
        auto some_str_expected = "1F8A690B7366A2323E2D5B045120DA7E93896F47"_s;
        EXPECT_EQ(PrintInfo{ some_str }.repr(), some_str_expected);

        auto some_other_str          = SHA1::calculate("Contami o diva del pelide Achille l'ira funesta"_s);
        auto some_other_str_expected = "E5F08D98BF18385E2F26B904CAD23C734D530FFB"_s;
        EXPECT_EQ(PrintInfo{ some_other_str }.repr(), some_other_str_expected);
    }
    {
        // SHA256
        auto zerolength          = SHA256::calculate(""_s);
        auto zerolength_expected = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"_s;
        EXPECT_EQ(PrintInfo{ zerolength }.repr(), zerolength_expected);

        auto some_str          = SHA256::calculate("Cantami o diva del pelide Achille l'ira funesta"_s);
        auto some_str_expected = "BABA6AB2A80F6C3079EC5891EAD5C497306FCD31B0472A627F3BDB3BB9C93F5C"_s;
        EXPECT_EQ(PrintInfo{ some_str }.repr(), some_str_expected);

        auto some_other_str          = SHA256::calculate("Contami o diva del pelide Achille l'ira funesta"_s);
        auto some_other_str_expected = "DAC3D4D2A80F322FD8909F432C5DCCA9DB13F23ACFF49269B77FF87B2FB7C976"_s;
        EXPECT_EQ(PrintInfo{ some_other_str }.repr(), some_other_str_expected);
    }
}
TEST(ARLibTests, StrStrTests) {
    auto str = "hello world";
    auto a   = "hello world";
    auto b   = "hello worlda";
    auto c   = "";
    auto d   = " wor";
    EXPECT_EQ(ARLib::strstr(str, a), str);
    EXPECT_EQ(ARLib::strstr(str, b), nullptr);
    EXPECT_EQ(ARLib::strstr(str, c), str);
    EXPECT_EQ(ARLib::strstr(str, d), str + 5);
}
#ifdef STRINGLITERAL_AVAILABLE
TEST(ARLibTests, StringLiteralTests) {
    constexpr static StringLiteral l{ "hello  world my name is" };
    constexpr char c = l[4];
    static_assert(c == 'o');
    constexpr auto sz = l.size();
    static_assert(sz == 23);
    constexpr auto occ = l.count("l");
    static_assert(occ == 3);
    constexpr auto splits = l.split<l.count("  ")>("  ");
    constexpr auto idx    = l.index_of("rld");
    static_assert(idx == 9);
    static_assert(splits.size() == 2);
    static_assert(splits[0] == "hello");
    static_assert(splits[1] == "world my name is");
}
#endif
TEST(ARLibTests, PriorityQueueTests) {
    struct TestQueueItem {
        String it;
        ENABLE_QUERY_ITEM(it)
    };
    auto queue_priority = [](const auto& left, const auto& right) {
        if (left.it.size() > right.it.size())
            return greater;
        else if (left.it.size() < right.it.size())
            return less;
        else
            return equal;
    };

    PriorityQueue<String, TestQueueItem> queue{ queue_priority };
    PriorityQueue<String> queue2{};
    queue.push(TestQueueItem{ "123"_s });
    queue.push(TestQueueItem{ "1234"_s });
    queue2.push("hello"_s, 10);
    queue2.push("world"_s, 20);
    EXPECT_EQ(queue2.pop(), "world"_s);
    EXPECT_EQ(queue2.pop(), "hello"_s);
    EXPECT_EQ(queue2.size(), 0);
    EXPECT_EQ(queue.pop(), "1234"_s);
    EXPECT_EQ(queue.pop(), "123"_s);
    EXPECT_EQ(queue.size(), 0);
}
TEST(ARLibTests, MatrixTests) {
    FixedMatrix2D<10> fmat{};
    auto [l, h] = fmat.shape();
    for (size_t i = 0; i < l; i++) {
        for (size_t j = 0; j < h; j++) { fmat[{ i, j }] = static_cast<double>(i * l + j); }
    }
    Matrix2D mat{ fmat };
    EXPECT_EQ(mat, fmat);
    EXPECT_EQ(mat.det(), fmat.det());
    EXPECT_EQ(mat.rank(), fmat.rank());
    EXPECT_EQ(mat.rank(), 2);
    auto mat_inv  = mat.inv();
    auto fmat_inv = fmat.inv();
    EXPECT_EQ(mat_inv, fmat_inv);
    EXPECT_EQ(mat_inv.det(), fmat_inv.det());
    mat -= 1.0;
    fmat *= 2.0;
    double val = mat[{ 5_sz, 5_sz }];
    EXPECT_EQ(val, 5.0 + 5.0 * 10.0 - 1.0);
}
TEST(ARLibTests, ArgParserTests) {
    const char* argv[]{ "ARLibPlayground.exe", "-b", "-t", "help.txt", "-n", "10", "-un", "200", "-v", "3.5" };
    ArgParser parser{ sizeof_array(argv), argv };
    parser.add_version(1, 0);
    parser.allow_unmatched(0);
    parser.add_usage_string("ARLibPlayground <options>");
    String t{};
    int n{};
    unsigned int un{};
    double v{};
    parser.add_option("-b", "testing boolean option", NoValueTag{});
    parser.add_option("-t", "test string", "testing string option", t);
    parser.add_option("-n", "test int", "testing int option", n);
    parser.add_option("-un", "test uint", "testing uint option", un);
    parser.add_option("-v", "test double", "testing double option", v);
    auto result = parser.parse();
    EXPECT_FALSE(result.is_error());
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(parser.get<bool>("-b").ok_value(), true);
    EXPECT_EQ(parser.get<int>("-n").ok_value(), 10);
    EXPECT_EQ(parser.get<unsigned int>("-un").ok_value(), 200);
    EXPECT_EQ(t, "help.txt");
    EXPECT_EQ(n, 10);
    EXPECT_EQ(un, 200);
    EXPECT_EQ(v, 3.5);

    const char* argv2[]{ "ARLibPlayground.exe", "aasdf" };
    ArgParser parser2{ sizeof_array(argv2), argv2 };
    parser.add_version(1, 0);
    parser.allow_unmatched(0);
    auto result2 = parser2.parse();
    EXPECT_TRUE(result2.is_error());
    EXPECT_FALSE(result2.is_ok());
    result2.ignore_error();
}
TEST(ARLibTests, PrintfWideString) {
    char buffer[1024]{};
    auto expected = "hello à è §©┘ world hello à è §©┘ world"_sv;
    int ret       = ARLib::sprintf(buffer, "%S %s", L"hello à è §©┘ world", "hello à è §©┘ world");
    buffer[ret]   = '\0';
    EXPECT_EQ(StringView{ buffer }, expected);
}
TEST(ARLibTests, PrintfNonNullString) {
    char buffer[1024]{};
    constexpr int expected_len = 23;
    constexpr StringView expected{ "hello world hello world" };
    String a{ "hello world my name is alessio" };
    WString aw{ L"hello world my name is alessio" };
    StringView b   = a.view().substringview_fromlen(0, ARLib::strlen("hello world"));
    WStringView bw = aw.view().substringview_fromlen(0, wstrlen(L"hello world"));
    int ret        = ARLib::snprintf(buffer, sizeof(buffer), "%.*s %.*S", b.size(), b.data(), bw.size(), bw.data());
    EXPECT_EQ(StringView{ buffer }, expected);
    EXPECT_EQ(expected_len, ret);
}
TEST(ARLibTests, PrintfTestGeneric) {
    // the expected values come from std printf implementation
    constexpr StringView expected_from_misc{
        "Hello World +0X1.90000P+5 +1234.1234  % my name is alessio 0120 0X7FFFFFFFFFFFFFFF %\n"
    };
    constexpr StringView expected_from_fill_pre{ "0X0C 000C" };
    constexpr int expected_pn           = 37;
    constexpr int expected_bsz_misc     = 85;
    constexpr int expected_bsz_fill_pre = 9;
    char buffer[1024]{};
    double val              = 1234.1234;
    const char* s           = "my name is alessio";
    int hex                 = 0x50;
    int arlib_pn            = 0;
    ARLib::int64_t int64val = NumberTraits<ARLib::int64_t>::max;
    int bsz                 = ARLib::sprintf(
    buffer, "Hello World %+.5A %+10.4f %n %% %s %#02o %#02llX %%\n", 50.0, val, &arlib_pn, s, hex, int64val
    );
    EXPECT_EQ(expected_bsz_misc, bsz);
    buffer[bsz] = '\0';
    EXPECT_EQ(StringView{ buffer }, expected_from_misc);
    EXPECT_EQ(arlib_pn, expected_pn);

    bsz = ARLib::sprintf(buffer, "%#04X %04X", 12, 12);
    EXPECT_EQ(expected_bsz_fill_pre, bsz);
    buffer[bsz] = '\0';
    EXPECT_EQ(StringView{ buffer }, expected_from_fill_pre);
}
TEST(ARLibTests, PrintfTestDoubleOnly) {
    // the expected values come from std printf implementation
    constexpr StringView expected_from_double{ "25.650000000000 25.650000 25.65 25.65 2.565e+01 2.565000E+01\n" };
    constexpr StringView expected_from_width_prec{ "+000000.00000000 +0.00000000           +0.00000000" };
    constexpr int expected_bsz_double     = 61;
    constexpr int expected_bsz_width_prec = 50;
    char buffer[1024]{};
    double v = 25.65;
    int bsz  = ARLib::sprintf(buffer, "%.12f %F %g %G %.3e %E\n", v, v, v, v, v, v);
    EXPECT_EQ(expected_bsz_double, bsz);
    buffer[bsz] = '\0';
    EXPECT_EQ(StringView{ buffer }, expected_from_double);
    bsz = ARLib::sprintf(buffer, "%+016.8Lf %-+16.8Lf %+16.8Lf", 0.0L, 0.0L, 0.0L);
    EXPECT_EQ(expected_bsz_width_prec, bsz);
    buffer[bsz] = '\0';
    EXPECT_EQ(StringView{ buffer }, expected_from_width_prec);
}
TEST(ARLibTests, PrintfTestStringOnly) {
    // the expected values come from std printf implementation
    constexpr StringView expected_from_leftaligned{ "'hello               ' 10" };
    constexpr StringView expected_from_zerofill_win{ "'000000000000000hello' 10" };
    constexpr StringView expected_from_zerofill_lin{ "'               hello' 10" };
    char buffer[1024]{};
    int bsz = ARLib::sprintf(buffer, "'%-20.5s' %d", "hello world", 10);
    EXPECT_EQ(bsz, expected_from_leftaligned.size());
    EXPECT_EQ(StringView{ buffer }, expected_from_leftaligned);
    bsz = ARLib::sprintf(buffer, "'%020.5s' %d", "hello world", 10);
    if constexpr (windows_build) {
        EXPECT_EQ(bsz, expected_from_zerofill_win.size());
        EXPECT_EQ(StringView{ buffer }, expected_from_zerofill_win);
    } else {
        EXPECT_EQ(bsz, expected_from_zerofill_lin.size());
        EXPECT_EQ(StringView{ buffer }, expected_from_zerofill_lin);
    }

    constexpr StringView expected_from_leftalignedw{ "'hello ù            ' 10" };
    constexpr StringView expected_from_zerofillw_win{ "'00000000000hello ù ' 10" };
    constexpr StringView expected_from_zerofillw_lin{ "'            hello ù' 10" };
    bsz = ARLib::sprintf(buffer, "'%-20.8S' %d", L"hello ù world", 10);
    EXPECT_EQ(bsz, expected_from_leftalignedw.size());
    EXPECT_EQ(StringView{ buffer }, expected_from_leftalignedw);
    bsz = ARLib::sprintf(buffer, "'%020.8S' %d", L"hello ù world", 10);
    if constexpr (windows_build) {
        EXPECT_EQ(bsz, expected_from_zerofillw_win.size());
        EXPECT_EQ(StringView{ buffer }, expected_from_zerofillw_win);
    } else {
        EXPECT_EQ(bsz, expected_from_zerofillw_lin.size());
        EXPECT_EQ(StringView{ buffer }, expected_from_zerofillw_lin);
    }
}
// path tests
TEST(ARLibTests, PathTestsRemoveFileSpec) {
    Path p1{ R"(C:\Users\user\folder\file.txt)" };
    Path p2{ R"(C:\Users\user\folder)" };
    Path p3{ R"(C:\Users\user\folder\)" };
    auto p1s = p1.remove_filespec();
    auto p2s = p2.remove_filespec();
    auto p3s = p3.remove_filespec();
    EXPECT_EQ(p1s, R"(C:\Users\user\folder)"_p);
    EXPECT_EQ(p2s, R"(C:\Users\user)"_p);
    EXPECT_EQ(p3s, R"(C:\Users\user\folder)"_p);
}
TEST(ARLibTests, PathTestsNormalize) {
    Path p1{ R"(C:/Users\user/folder\file.txt)" };
    Path p2{ R"(C:\\Users\/\user\\folder\\/\file.txt)" };
    EXPECT_EQ(p1, p2);
}
TEST(ARLibTests, PathTestsParentPath) {
    if constexpr (windows_build) {
        Path p1{ R"(C:\Users\user\folder\file.txt)" };
        Path p2{ R"(C:\Users\user\folder)" };
        Path p3{ R"(Users\\user\\folder\\file.txt)" };
        Path p4{ R"(C:\)" };
        Path p5{ R"(Users)" };
        EXPECT_EQ(p1.parent_path(), R"(C:\Users\user\folder)"_p);
        EXPECT_EQ(p2.parent_path(), R"(C:\Users\user)"_p);
        EXPECT_EQ(p3.parent_path(), R"(Users\user\folder)"_p);
        EXPECT_EQ(p4.parent_path(), R"(C:\)"_p);
        EXPECT_EQ(p5.parent_path(), ""_p);
    } else {
        Path p1{ R"(/Users/user/folder/file.txt)" };
        Path p2{ R"(/Users/user/folder)" };
        Path p3{ R"(Users/user/folder/file.txt)" };
        Path p4{ R"(/)" };
        Path p5{ R"(Users)" };
        EXPECT_EQ(p1.parent_path(), "/Users/user/folder"_p);
        EXPECT_EQ(p2.parent_path(), "/Users/user"_p);
        EXPECT_EQ(p3.parent_path(), "Users/user/folder"_p);
        EXPECT_EQ(p4.parent_path(), "/"_p);
        EXPECT_EQ(p5.parent_path(), ""_p);
    }
}
TEST(ARLibTests, PathTestsAbsolutePath) {
    if constexpr (windows_build) {
        Path p1{ R"(C:\Users\user\folder\file.txt)" };
        Path p2{ R"(C:\Users\user\folder)" };
        Path p3{ R"(Users\\user\\folder\\file.txt)" };
        Path p4{ R"(C:\)" };
        Path p5{ R"(Users)" };
        EXPECT_TRUE(p1.is_absolute());
        EXPECT_TRUE(p2.is_absolute());
        EXPECT_FALSE(p3.is_absolute());
        EXPECT_TRUE(p4.is_absolute());
        EXPECT_FALSE(p5.is_absolute());
    } else {
        Path p1{ R"(/Users/user/folder/file.txt)" };
        Path p2{ R"(/Users/user/folder)" };
        Path p3{ R"(Users/user/folder/file.txt)" };
        Path p4{ R"(/)" };
        Path p5{ R"(Users)" };
        EXPECT_TRUE(p1.is_absolute());
        EXPECT_TRUE(p2.is_absolute());
        EXPECT_FALSE(p3.is_absolute());
        EXPECT_TRUE(p4.is_absolute());
        EXPECT_FALSE(p5.is_absolute());
    }
}
TEST(ARLibTests, PathTestsFilenameAndExtension) {
    if constexpr (windows_build) {
        Path p1{ R"(C:\Users\user\folder\file.txt)" };
        Path p2{ R"(C:\Users\user\folder)" };
        Path p3{ R"(Users\\user\\folder\)" };
        EXPECT_EQ(p1.filename(), "file.txt"_p);
        EXPECT_EQ(p1.extension(), ".txt"_p);
        EXPECT_EQ(p2.filename(), "folder"_p);
        EXPECT_EQ(p2.extension(), ""_p);
        EXPECT_EQ(p3.filename(), ""_p);
        EXPECT_EQ(p2.extension(), ""_p);
    } else {
        Path p1{ R"(/Users/user/folder/file.txt)" };
        Path p2{ R"(/Users/user/folder)" };
        Path p3{ R"(Users/user/folder/)" };
        EXPECT_EQ(p1.filename(), "file.txt"_p);
        EXPECT_EQ(p1.extension(), ".txt"_p);
        EXPECT_EQ(p2.filename(), "folder"_p);
        EXPECT_EQ(p2.extension(), ""_p);
        EXPECT_EQ(p3.filename(), ""_p);
        EXPECT_EQ(p2.extension(), ""_p);
    }
}
TEST(ARLibTests, PathTestsConcatenation) {
    if constexpr (windows_build) {
        Path p1{ R"(C:\Users\user\folder\child\file.txt)" };
        Path p2{ R"(C:\Users\user\folder\child2\file.txt)" };

        Path p3{ R"(C:\Users\user\folder\child\file.txt)" };
        Path p4{ R"(child2\file.txt)" };

        Path p5{ R"(C:\Users\user\folder\child)" };
        Path p6{ R"(folder\child2\file.txt)" };

        EXPECT_EQ(p1 / p2, R"(C:\Users\user\folder\child2\file.txt)"_p);
        EXPECT_EQ(p2 / p1, R"(C:\Users\user\folder\child\file.txt)"_p);

        EXPECT_EQ(p3 / p4, R"(C:\Users\user\folder\child\file.txt\child2\file.txt)"_p);
        EXPECT_EQ(p4 / p3, R"(C:\Users\user\folder\child\file.txt)"_p);

        EXPECT_EQ(p5 / p6, R"(C:\Users\user\folder\child\folder\child2\file.txt)"_p);
        EXPECT_EQ(p6 / p5, R"(C:\Users\user\folder\child)"_p);
    } else {
        Path p1{ R"(/Users/user/folder/child/file.txt)" };
        Path p2{ R"(/Users/user/folder/child2/file.txt)" };

        Path p3{ R"(/Users/user/folder/child/file.txt)" };
        Path p4{ R"(child2/file.txt)" };

        Path p5{ R"(/Users/user/folder/child)" };
        Path p6{ R"(folder/child2/file.txt)" };

        EXPECT_EQ(p1 / p2, R"(/Users/user/folder/child2/file.txt)"_p);
        EXPECT_EQ(p2 / p1, R"(/Users/user/folder/child/file.txt)"_p);

        EXPECT_EQ(p3 / p4, R"(/Users/user/folder/child/file.txt/child2/file.txt)"_p);
        EXPECT_EQ(p4 / p3, R"(/Users/user/folder/child/file.txt)"_p);

        EXPECT_EQ(p5 / p6, R"(/Users/user/folder/child/folder/child2/file.txt)"_p);
        EXPECT_EQ(p6 / p5, R"(/Users/user/folder/child)"_p);
    }
}
TEST(ARLibTests, SortingTest) {
    Array strings{
        "XZk6g68IJe"_sv, "xLX1I3D9Ju"_sv, "c8iChK6P5U"_sv, "ke1fjfqM4h"_sv, "C14RwSqFaK"_sv, "ZYApUXmw8i"_sv,
        "NSsOfFyYQw"_sv, "2embvf20ZJ"_sv, "QXreMhn9Rk"_sv, "OyTfRkPoWP"_sv, "kHXnimdjGb"_sv, "mtylTdDHs8"_sv,
        "HjDNsHZJ0O"_sv, "o0MmzO0OQ7"_sv, "C81lYiEoPK"_sv, "fxpSWLS0in"_sv, "QWLODBo9lj"_sv, "8N0MIpeP8j"_sv,
        "3xbvOimAaK"_sv, "wj41wl0ifb"_sv, "vjHFCWCKU1"_sv, "wQHSdcKmrn"_sv, "PXTMNP75hq"_sv, "L7TuxocgEG"_sv,
        "g7bQbXwCwG"_sv, "OTgkPNbdRG"_sv, "GXVpodrMcv"_sv, "uXsiZQv2KK"_sv, "144IRet2MM"_sv, "eQWNvRkaFh"_sv,
        "PDggPLAcQD"_sv, "WV4gj6eVx3"_sv, "H2az78ED8Q"_sv, "G3BKZOi5AP"_sv, "00eUWw5keT"_sv, "EekC8u3Fgp"_sv,
        "zBdebgoSjW"_sv, "NpetUuoijR"_sv, "hdNt1J3d35"_sv, "Kyi8aIHEeo"_sv, "dR0WL76qoO"_sv, "ggrxoFagtP"_sv,
        "qDJN7Clkcd"_sv, "8xPcyvzstU"_sv, "Fp0V3JBAQ1"_sv, "CAAIyvZJ2l"_sv, "jHFIKEqoDP"_sv, "0ePRmgMGJj"_sv,
        "UlCy28HaHT"_sv, "ULgHyVJVX6"_sv, "cG624ueEoO"_sv, "A1hg0xHLfQ"_sv, "C2kkYZkTtF"_sv, "1yUtwevLgb"_sv,
        "za9G6Ry7RA"_sv, "Kkfk6ZTfse"_sv, "20y9tEFtv7"_sv, "jbg7UzBTE1"_sv, "qjlGhPxKn0"_sv, "PgNHVLj5GD"_sv,
        "2I6GpgwhEj"_sv, "4hVs495EX4"_sv, "XdxSo7jBgU"_sv, "inlBP3ELRI"_sv, "h3mUk3Cw33"_sv, "4rHb12uhoa"_sv,
        "DrycQ6SL94"_sv, "GglNpPbzWw"_sv, "J9gtEsYZ48"_sv, "MpXA02CaUr"_sv, "XwTX7hCsA0"_sv, "p7USMCQ140"_sv,
        "yk14lFbf6S"_sv, "OxPYxbX4Up"_sv, "RRkhyOXK6j"_sv, "6guDMubTGf"_sv, "xdgN9VZ5xJ"_sv, "sZVJ6V5bzs"_sv,
        "mlBklidD2m"_sv, "vTPq38FlDd"_sv, "KC7cKE88kp"_sv, "CDtuCg6FoP"_sv, "pJUrm9kB7t"_sv, "l493YSmNkc"_sv,
        "oTI3T4gRRt"_sv, "qZLEQQYxor"_sv, "sRnv5wXIqn"_sv, "VTZRkswfh7"_sv, "q69Vq4wpIK"_sv, "SWQ6dU4wLs"_sv,
        "nHTVA6jzsS"_sv, "2phWwNfCHp"_sv, "HQ6oNmPko5"_sv, "LgUFsTN0eC"_sv, "hlDB7lRGJ0"_sv, "8kGMMJV7PR"_sv,
        "M798JiFBuA"_sv, "Sq51RKKFUZ"_sv, "6Lg6W9hZ9X"_sv, "HA741LK6Ai"_sv,
    };
    const Array expected{
        "00eUWw5keT"_sv, "0ePRmgMGJj"_sv, "144IRet2MM"_sv, "1yUtwevLgb"_sv, "20y9tEFtv7"_sv, "2I6GpgwhEj"_sv,
        "2embvf20ZJ"_sv, "2phWwNfCHp"_sv, "3xbvOimAaK"_sv, "4hVs495EX4"_sv, "4rHb12uhoa"_sv, "6Lg6W9hZ9X"_sv,
        "6guDMubTGf"_sv, "8N0MIpeP8j"_sv, "8kGMMJV7PR"_sv, "8xPcyvzstU"_sv, "A1hg0xHLfQ"_sv, "C14RwSqFaK"_sv,
        "C2kkYZkTtF"_sv, "C81lYiEoPK"_sv, "CAAIyvZJ2l"_sv, "CDtuCg6FoP"_sv, "DrycQ6SL94"_sv, "EekC8u3Fgp"_sv,
        "Fp0V3JBAQ1"_sv, "G3BKZOi5AP"_sv, "GXVpodrMcv"_sv, "GglNpPbzWw"_sv, "H2az78ED8Q"_sv, "HA741LK6Ai"_sv,
        "HQ6oNmPko5"_sv, "HjDNsHZJ0O"_sv, "J9gtEsYZ48"_sv, "KC7cKE88kp"_sv, "Kkfk6ZTfse"_sv, "Kyi8aIHEeo"_sv,
        "L7TuxocgEG"_sv, "LgUFsTN0eC"_sv, "M798JiFBuA"_sv, "MpXA02CaUr"_sv, "NSsOfFyYQw"_sv, "NpetUuoijR"_sv,
        "OTgkPNbdRG"_sv, "OxPYxbX4Up"_sv, "OyTfRkPoWP"_sv, "PDggPLAcQD"_sv, "PXTMNP75hq"_sv, "PgNHVLj5GD"_sv,
        "QWLODBo9lj"_sv, "QXreMhn9Rk"_sv, "RRkhyOXK6j"_sv, "SWQ6dU4wLs"_sv, "Sq51RKKFUZ"_sv, "ULgHyVJVX6"_sv,
        "UlCy28HaHT"_sv, "VTZRkswfh7"_sv, "WV4gj6eVx3"_sv, "XZk6g68IJe"_sv, "XdxSo7jBgU"_sv, "XwTX7hCsA0"_sv,
        "ZYApUXmw8i"_sv, "c8iChK6P5U"_sv, "cG624ueEoO"_sv, "dR0WL76qoO"_sv, "eQWNvRkaFh"_sv, "fxpSWLS0in"_sv,
        "g7bQbXwCwG"_sv, "ggrxoFagtP"_sv, "h3mUk3Cw33"_sv, "hdNt1J3d35"_sv, "hlDB7lRGJ0"_sv, "inlBP3ELRI"_sv,
        "jHFIKEqoDP"_sv, "jbg7UzBTE1"_sv, "kHXnimdjGb"_sv, "ke1fjfqM4h"_sv, "l493YSmNkc"_sv, "mlBklidD2m"_sv,
        "mtylTdDHs8"_sv, "nHTVA6jzsS"_sv, "o0MmzO0OQ7"_sv, "oTI3T4gRRt"_sv, "p7USMCQ140"_sv, "pJUrm9kB7t"_sv,
        "q69Vq4wpIK"_sv, "qDJN7Clkcd"_sv, "qZLEQQYxor"_sv, "qjlGhPxKn0"_sv, "sRnv5wXIqn"_sv, "sZVJ6V5bzs"_sv,
        "uXsiZQv2KK"_sv, "vTPq38FlDd"_sv, "vjHFCWCKU1"_sv, "wQHSdcKmrn"_sv, "wj41wl0ifb"_sv, "xLX1I3D9Ju"_sv,
        "xdgN9VZ5xJ"_sv, "yk14lFbf6S"_sv, "zBdebgoSjW"_sv, "za9G6Ry7RA"_sv
    };
    sort(strings);
    for (const auto& [g, e] : zip(strings, expected)) { EXPECT_EQ(g, e); }
}
TEST(ARLibTests, SpanTests) {
    Vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Array arr       = { "Hello"_sv, "World"_sv, "How"_sv, "Are"_sv, "You"_sv };

    auto span  = vec.span();
    auto span2 = span.subspan(2, 5);
    auto span3 = span.subspan(123123, 1234123);
    auto span4 = arr.span().subspan(1, 3);
    auto span5 = GenericView{ arr }.span().subspan(1, 3);
    EXPECT_EQ(span4, span5);
    EXPECT_EQ(span4.size(), 3);
    EXPECT_EQ(span3.size(), 0);
    EXPECT_TRUE(span3.empty());
    EXPECT_EQ(span.size(), vec.size());
    EXPECT_EQ(span2.size(), 5);
    EXPECT_EQ(span2[0], 3);
    EXPECT_EQ(span2[1], 4);
    EXPECT_EQ(span4[0], "World"_sv);
    EXPECT_EQ(span4[1], "How"_sv);
    EXPECT_EQ(span4[2], "Are"_sv);
}
TEST(ARLibTests, ProcessTests) {
    if constexpr (windows_build) {
        auto proc = Process{ "cmd"_sv };
        proc.with_args({ "/c"_sv, "echo"_sv, "Hello World"_sv }).set_pipe(HandleType::Output);
        auto err = proc.launch();
        EXPECT_TRUE(err.is_ok());
        auto ec = proc.wait_for_exit();
        EXPECT_TRUE(ec.is_ok());
        EXPECT_EQ(ec.to_ok(), 0);
        EXPECT_EQ(proc.output().string_view(), "Hello World\r\n"_sv);
        auto pipe   = Process{ "where" }.with_args({ "cmd"_sv }) | Process{ "findstr" }.with_args({ "System32"_sv });
        auto piperr = pipe.run();
        EXPECT_TRUE(piperr.is_ok());
        EXPECT_EQ(piperr.to_ok(), 0);
        EXPECT_EQ(pipe.output().string_view(), "C:\\Windows\\System32\\cmd.exe\r\n"_sv);
    } else {
        auto proc = Process{ "echo"_sv };
        proc.with_args({ "Hello World"_sv }).set_pipe(HandleType::Output);
        auto err = proc.launch();
        EXPECT_TRUE(err.is_ok());
        auto ec = proc.wait_for_exit();
        EXPECT_TRUE(ec.is_ok());
        EXPECT_EQ(ec.to_ok(), 0);
        EXPECT_EQ(proc.output().string_view(), "Hello World\n"_sv);
        auto pipe =
        Process{ "echo" }.with_args({ "hello\nworld\ntesting"_sv }) | Process{ "grep" }.with_args({ "world"_sv });
        auto piperr = pipe.run();
        EXPECT_TRUE(piperr.is_ok());
        EXPECT_EQ(piperr.to_ok(), 0);
        EXPECT_EQ(pipe.output().string_view(), "world\n"_sv);
    }
}
TEST(ARLibTests, AsyncTest) {
    auto long_running_task = [](StringView str, Pair<int, int> pair) {
        ThisThread::sleep(2'000'000);
        return str.extract_string() + IntToStr(pair.first()) + IntToStr(pair.second());
    };
    auto fut  = create_async_task(long_running_task, "Hello World"_sv, Pair{ 10, 20 });
    auto fut2 = create_async_task(&String::at, "Hello World"_s, 1_sz);
    auto fut3 = create_deferred_task(long_running_task, "Hello World"_sv, Pair{ 20, 30 });
    auto res3 = fut3.wait();
    auto res  = fut.wait();
    auto res2 = fut2.wait();
    EXPECT_EQ(res, "Hello World1020"_s);
    EXPECT_EQ(res2, 'e');
    EXPECT_EQ(res3, "Hello World2030"_s);
}
TEST(ARLibTests, FlatMapTest) {
    FlatMap<String, int> map{};
    auto val = map.insert("hello"_s, 10);
    auto ex  = map.insert("world"_s, 20);
    EXPECT_EQ(val, true);
    EXPECT_EQ(ex, true);
    auto r = map["hello"_s];
    EXPECT_EQ(r, 10);
    val = map.insert("hello"_s, 30);
    EXPECT_EQ(val, false);
    r = map["hello"_s];
    EXPECT_EQ(r, 30);
    EXPECT_EQ(map.size(), 2ull);
    auto res = map.remove("hello"_s);
    EXPECT_EQ(res, true);
    res = map.remove("hello"_s);
    EXPECT_EQ(res, false);
    EXPECT_EQ(map.size(), 1ull);
}
TEST(ARLibTests, FlatMapSetExtraTest) {
    constexpr static size_t n_of_strings = 1024;
    Set<String> strings{};    // using Set to make sure the strings are all unique
    FlatSet<String> set{};
    FlatMap<String, int> map{};

    auto generate_string = [](uint32_t max_len) {
        String s{};
        size_t len = Random::PCG::bounded_random_s(max_len);
        if (len == 0) { len = 1; }
        s.reserve(len);

        constexpr uint32_t start = ' ';
        constexpr uint32_t range = '}' - start;

        for (size_t i = 0; i < len; ++i) { s.append(static_cast<char>(Random::PCG::bounded_random_s(range) + start)); }
        return s;
    };
    auto fill_set = [&strings, &set]() {
        for (const auto& s : strings) { EXPECT_TRUE(set.insert(String{ s })); }
        EXPECT_EQ(set.size(), strings.size());
    };
    auto search_set = [&strings, &set]() {
        for (const auto& s : strings) { EXPECT_NE(set.find(s), set.end()); }
    };
    auto erase_set = [&strings, &set]() {
        for (const auto& s : strings) { EXPECT_TRUE(set.remove(s)); }
        EXPECT_EQ(set.size(), 0);
    };
    auto fill_map = [&strings, &map]() {
        for (const auto& [i, s] : enumerate(strings)) { EXPECT_TRUE(map.insert(String{ s }, static_cast<int>(i))); }
        EXPECT_EQ(map.size(), strings.size());
    };
    auto search_map = [&strings, &map]() {
        for (const auto& [i, s] : enumerate(strings)) {
            auto it = map.find(s);
            EXPECT_NE(it, map.end());
            EXPECT_EQ((*it).val(), i);
            EXPECT_EQ(map[s], i);
        }
    };
    auto erase_map = [&strings, &map]() {
        for (const auto& s : strings) { EXPECT_TRUE(map.remove(s)); }
        EXPECT_EQ(map.size(), 0);
    };
    strings.reserve(n_of_strings);

    auto fill_strings_thread = JThread{ [&](StopToken tok) {
        while (strings.size() < n_of_strings) {
            strings.insert(generate_string(32));
            if (tok.stop_requested()) return;
        }
    } };

    ThisThread::sleep(10'000);    // sleep for 10 ms and hope the strings vector is filled enough
    fill_strings_thread.request_stop();
    fill_strings_thread.join();

    fill_set();
    search_set();
    erase_set();
    fill_map();
    search_map();
    erase_map();
}