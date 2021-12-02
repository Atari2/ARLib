#include "Suite.h"
#include <gtest/gtest.h>

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
    auto str = Optional<String>{}.value_or("Hello world"_s);
    auto str2 = Optional<String>{"hello cpp"_s}.value_or("shouldn't get returned"_s);
    EXPECT_EQ(str, "Hello world"_s);
    EXPECT_EQ(str2, "hello cpp"_s);
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
    Tuple<int, String> tup4{110, "hello"_s};

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
    PartialFunction func3{decl2, 10};
    PartialFunction func1{test_partial_func, 10, "hello"_s};
    PartialFunction func2{decl, 10, "hello"_s};
    auto res1 = func1(Tuple<String, int>{"world"_s, 10});
    auto res2 = func2(Tuple<String, int>{"world"_s, 10});
    EXPECT_EQ(res1, res2);
    EXPECT_EQ(res1, 30ull);
    EXPECT_EQ(res2, 30ull);
    EXPECT_EQ(res1, test_partial_func(10, "hello"_s, Tuple<String, int>{"world"_s, 10}));
    EXPECT_EQ(res2, decl(10, "hello"_s, Tuple<String, int>{"world"_s, 10}));
    EXPECT_EQ(func3(10), 20);
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
    auto res = avg(vec);
    auto res2 = avg(vec.begin(), vec.end());
    EXPECT_EQ(res, res2);
    EXPECT_EQ(res, 49.5);
    EXPECT_EQ(res2, 49.5);
    EXPECT_EQ(avg(vec), avg(vec.begin(), vec.end()));
    auto res_rounded = avg<decltype(vec), true>(vec);
    auto res2_rounded = avg<decltype(vec.begin()), true>(vec.begin(), vec.end());
    EXPECT_EQ(res_rounded, res2_rounded);
    EXPECT_EQ(res_rounded, 49);
    EXPECT_EQ(res2_rounded, 49);
}

TEST(ARLibTests, ArrayTest) {
    Array arr{"hello"_s, "world"_s, "testing"_s};
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
    StringView view{str};
    EXPECT_EQ(str, view);
    EXPECT_EQ(sub.index_of('w'), 0ull);
    String repls = str.replace("l", "foo");
    EXPECT_EQ(repls, "hefoofooo worfood"_s);
    EXPECT_EQ(repls.replace("foo", "te"), "heteteo worted"_s);
}

TEST(ARLibTests, StringViewTests) {
    constexpr StringView view = "hello world"_sv;
    auto vec = view.split();
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "hello"_sv);
    EXPECT_EQ(vec[1], "world"_sv);
    constexpr char c = view[7];
    static_assert(c == 'o'); // this needs to compile
    static_assert(view == "hello world"_sv);
    static_assert(view == "hello world");
}

TEST(ARLibTests, FormatTest) {
    Vector<double> vec{1.0, 2.0, 3.0};
    Map<String, int> map{};
    map.add("Hello"_s, 1);
    map.add("World"_s, 2);
    auto ret = Printer::format("My name is {{}}, I'm {} years old, vector: {}, map: {}", 22, vec, map);
    EXPECT_EQ(ret,
              "My name is {}, I'm 22 years old, vector: [1.000000, 2.000000, 3.000000], map: { Hello: 1, World: 2 }"_s);
    auto ret2 = Printer::format("{{}}");
    EXPECT_EQ(ret2, "{}"_s);
    auto ret3 = Printer::format("{}", nullptr);
    EXPECT_EQ(ret3, "nullptr"_s);
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

TEST(ARLibTests, SSOVectorTests) {
    SSOVector vec{"hello"_s, "world"_s};
    EXPECT_TRUE(vec.is_in_situ());
    auto sz = vec.size();
    for (size_t i = 0; i < vec.sso() - sz; i++) {
        vec.push_back("a"_s);
    }
    EXPECT_TRUE(vec.is_in_situ());
    vec.push_back("b"_s);
    EXPECT_FALSE(vec.is_in_situ());
    EXPECT_EQ(vec[0], "hello"_s);
    SSOVector<int, 25> sso1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    SSOVector<int, 100> sso2{};
    sso2 = sso1;
    EXPECT_EQ(sso1.size(), sso2.size());
    for (size_t i = 0; i < sso1.size(); i++)
        EXPECT_EQ(sso1[i], sso2[i]);
}

TEST(ARLibTests, LinkedSetTests) {
    LinkedSet lset{"hello"_s, "hello"_s};
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
    LinkedList list{1, 2, 3, 4};
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
    LinkedList list2{"hello"_s};
    EXPECT_EQ(list2.pop(), "hello"_s);
}

TEST(ARLibTests, GenericViewTests) {
    Vector<int> vec{};
    vec.fill_pattern([](int i) { return ++i; }, 1000);
    Vector veccp{vec};
    IteratorView view{vec};
    IteratorView view2{veccp};
    String form_filtered{
    R"(["112", "128", "144", "160", "176", "192", "208", "224", "240", "256", "272", "288", "304", "320", "336", "352", "368", "384", "400", "416", "432", "448", "464", "480", "496", "512", "528", "544", "560", "576", "592", "608", "624", "640", "656", "672", "688", "704", "720", "736", "752", "768", "784", "800", "816", "832", "848", "864", "880", "896", "912", "928", "944", "960", "976", "992"])"_s};
    view.transform([](int a) { return a * 2; });
    for (auto [index, item] : Enumerate{vec}) {
        EXPECT_EQ(item, index * 2);
    }
    auto vec2 = view.map([](int) { return 0; });
    auto vec3 = view.transform_map<Vector<String>>([](int a) { return IntToStr(a); });
    for (const auto& [index, item] : Enumerate{vec3}) {
        EXPECT_EQ(item, IntToStr(index * 2));
    }
    for (auto item : vec2) {
        EXPECT_EQ(item, 0);
    }
    auto lam = [](int c) {
        return IntToStr(c * 4);
    };
    auto filtered = view2.map_view([](int a) { return a * 2; })
                    .map_view([](int b) { return b * 2; })
                    .map_view<decltype(lam), Vector<String>>(lam)
                    .filter([](const String& str) { return str.size() == 3; })
                    .collect<Vector<String>>();
    EXPECT_EQ(filtered.size(), 56ull);
    EXPECT_EQ(Printer::format("{}", filtered), form_filtered);
}

TEST(ARLibTests, StringTest2) {
    String str{"ciao come ciao io ciao sono ciao pippo"};
    String str2{"ciao ciao"};
    EXPECT_EQ(str2.last_index_not_of("ciao", str2.size() - 1), 4ull);
    auto ret = str.split("ciao");
    auto retv = str.split_view("ciao");
    Vector vec{""_s, " come "_s, " io "_s, " sono "_s, " pippo"_s};
    EXPECT_EQ(ret, vec);
    EXPECT_EQ(retv, vec);
    EXPECT_EQ(str.last_index_of_any("po"), str.size() - 1);
    str2 += 'c';
    EXPECT_EQ(str2, "ciao ciaoc"_s);
}

TEST(ARLibTests, PartialFuncTest2) {
    auto func = [](int a, int b) {
        return a + b;
    };
    PartialFunction partial{func, 10};
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

    Function<int(int, int)> fn{func};
    Function<bool(void)> fn2{TestStruct::ret};
    Function<bool(TestStruct*, bool)> fn3{&TestStruct::other_ret};
    EXPECT_EQ(fn(10, 10), 20);
    EXPECT_EQ(fn2(), true);
    EXPECT_EQ(fn3(&st, false), false);
}

TEST(ARLibTests, MoreFormatTests) {
    auto map_print = R"([{ hello: 10, cap: 10, world: 20 }, {}, {}])"_s;
    auto vec_of_vecs_print = R"([["hello", "world"], ["name"], ["cap"], [], [], [], [], [], [], []])"_s;
    auto mat_print =
    "[\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n\t[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]\n]"_s;
    auto row_print = R"([0, 0, 0, 0, 0, 0, 0, 0, 0, 0])"_s;
    int mat[10][10]{};
    decltype(auto) val = mat[0];
    Map<String, int> map[3]{{{"hello"_s, 10}, {"cap"_s, 10}, {"world"_s, 20}}};
    Vector<String> vec[10] = {Vector{"hello"_s, "world"_s}, Vector{"name"_s}, Vector{"cap"_s}};
    EXPECT_EQ(Printer::format("{}", map), map_print);
    EXPECT_EQ(Printer::format("{}", vec), vec_of_vecs_print);
    EXPECT_EQ(Printer::format("{}", val), row_print);
    EXPECT_EQ(Printer::format("{}", mat), mat_print);
    auto data = vec[0][0].data();
    EXPECT_EQ(Printer::format("{}", data), "hello"_s);
}

TEST(ARLibTests, ContainerAlgoTest) {
    Vector<int> vec1{1, 2, 3, 4, 5, 6};
    Vector<String> vec2{"hello"_s, "world"_s, "why"_s};
    HashMap<String, Vector<int>> map{{"hello"_s, Vector<int>{1, 2, 3, 4, 5}}, {"world"_s, Vector<int>{6, 7, 8, 9, 10}}};
    EXPECT_EQ(all_of(vec1, [](int a) { return a < 7; }), true);
    EXPECT_EQ(any_of(vec1, [](int a) { return a == 3; }), true);
    EXPECT_EQ(exactly_n(
              vec1, [](int a) { return a < 4; }, 3),
              true);
    EXPECT_EQ(all_of(vec2, [](const String& str) { return str == "hello"_s; }), false);
    EXPECT_EQ(any_of(vec2, [](const String& str) { return str == "hello"_s; }), true);
    EXPECT_EQ(exactly_n(
              vec2, [](const String& str) { return str == "hello"_s; }, 1),
              true);
    EXPECT_EQ(all_of(map,
                     [](const auto& entry) {
                         auto b = entry.key() == "hello"_s || entry.key() == "world"_s;
                         return b && all_of(entry.value(), [](int a) { return a < 11; });
                     }),
              true);
}

#ifndef DISABLE_THREADING

TEST(ARLibTests, ThreadingTests) {
    auto func = [](int val, String help) {
        EXPECT_EQ(val, 30);
        EXPECT_EQ(help, "hello world"_s);
    };

    Thread t{};
    EXPECT_FALSE(t.joinable());
    t = move(Thread{func, 30, "hello world"_s});
    EXPECT_TRUE(t.joinable());
    t.join();
    EXPECT_FALSE(t.joinable());
    Mutex m1{};
    Mutex m2{};
    Mutex m3{};
    { ScopedLock lock{m1, m2, m3}; }
    RecursiveMutex m4{};
    { UniqueLock ll{m4}; }
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
    for (int i = 0; i < 100; i++) {
        s.append('c');
    }
    auto new_now = Clock::now();
    EXPECT_GT(new_now, now);
    auto diff = Clock::diff(now, new_now);
    EXPECT_GT(diff, 0);
}

TEST(ARLibTests, JSONTest) {
    auto maybe_obj = JSON::Parser::parse(R"({"hello world": 10, "array": [1, 2, 3, 4]})"_sv);
    EXPECT_TRUE(maybe_obj.is_ok());
    auto obj = maybe_obj.to_ok();
    Vector vec{1, 2, 3, 4};
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
    auto a = BigInt{"12389123908"_s};
    auto b = BigInt{"-983458171238123"_s};
    auto c = BigInt{"875679183741987"_s};
    auto d = BigInt{"-1238767812763"_s};

    auto f = BigInt{1234};
    auto g = BigInt{4321};
    auto h = BigInt{-1234};
    auto i = BigInt{-4321};

    f -= g;
    h -= i;

    EXPECT_EQ(f, -3087);
    EXPECT_EQ(h, 3087);

    auto result_diff_ab = BigInt{"983470560362031"_s};
    auto result_diff_ba = BigInt{"-983470560362031"_s};
    auto result_diff_ac = BigInt{"-875666794618079"_s};
    auto result_diff_ca = BigInt{"875666794618079"_s};
    auto result_diff_bd = BigInt{"-982219403425360"_s};
    auto result_diff_db = BigInt{"982219403425360"_s};

    auto result_sum_ab = BigInt{"-983445782114215"_s};
    auto result_sum_ba = BigInt{"-983445782114215"_s};
    auto result_sum_ac = BigInt{"875691572865895"_s};
    auto result_sum_ca = BigInt{"875691572865895"_s};
    auto result_sum_bd = BigInt{"-984696939050886"_s};
    auto result_sum_db = BigInt{"-984696939050886"_s};

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

    auto f2 = BigInt{"123456781234567891234879169467981276392189732178937891237928173981239812219873218973"_s};
    auto g2 = BigInt{"12837127389712389123891738127317892312987389217"_s};
    auto div_result = BigInt{"9617165701222654353349541990196129976"_s};

    EXPECT_EQ(div_result, f2 / g2);
}

TEST(ARLibTests, HashFuncTests) {
    {
        // CRC32
        auto zerolength = CRC32::calculate(""_s);
        EXPECT_EQ(zerolength, 0x00);
        auto ff = CRC32::calculate(Array{0xFF_u8});
        auto zero = CRC32::calculate(Array{0x00_u8});
        EXPECT_EQ(ff, 0xFF000000);
        EXPECT_EQ(zero, 0xD202EF8D);
        auto some_str = CRC32::calculate("123456789"_s);
        EXPECT_EQ(some_str, 0xCBF43926);
    }
    {
        // MD5
        auto zerolength = MD5::calculate(""_s);
        auto zerolength_expected = "D41D8CD98F00B204E9800998ECF8427E"_s;
        EXPECT_EQ(PrintInfo{zerolength}.repr(), zerolength_expected);

        auto some_str = MD5::calculate("The quick brown fox jumps over the lazy dog"_s);
        auto some_str_expected = "9E107D9D372BB6826BD81D3542A419D6"_s;
        EXPECT_EQ(PrintInfo{some_str}.repr(), some_str_expected);

        auto some_other_str = MD5::calculate("The quick brown fox jumps over the lazy dog."_s);
        auto some_other_str_expected = "E4D909C290D0FB1CA068FFADDF22CBD0"_s;
        EXPECT_EQ(PrintInfo{some_other_str}.repr(), some_other_str_expected);
    }
    {
        // SHA1
        auto zerolength = SHA1::calculate(""_s);
        auto zerolength_expected = "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709"_s;
        EXPECT_EQ(PrintInfo{zerolength}.repr(), zerolength_expected);

        auto some_str = SHA1::calculate("Cantami o diva del pelide Achille l'ira funesta"_s);
        auto some_str_expected = "1F8A690B7366A2323E2D5B045120DA7E93896F47"_s;
        EXPECT_EQ(PrintInfo{some_str}.repr(), some_str_expected);

        auto some_other_str = SHA1::calculate("Contami o diva del pelide Achille l'ira funesta"_s);
        auto some_other_str_expected = "E5F08D98BF18385E2F26B904CAD23C734D530FFB"_s;
        EXPECT_EQ(PrintInfo{some_other_str}.repr(), some_other_str_expected);
    }
    {
        // SHA256
        auto zerolength = SHA256::calculate(""_s);
        auto zerolength_expected = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"_s;
        EXPECT_EQ(PrintInfo{zerolength}.repr(), zerolength_expected);

        auto some_str = SHA256::calculate("Cantami o diva del pelide Achille l'ira funesta"_s);
        auto some_str_expected = "BABA6AB2A80F6C3079EC5891EAD5C497306FCD31B0472A627F3BDB3BB9C93F5C"_s;
        EXPECT_EQ(PrintInfo{some_str}.repr(), some_str_expected);

        auto some_other_str = SHA256::calculate("Contami o diva del pelide Achille l'ira funesta"_s);
        auto some_other_str_expected = "DAC3D4D2A80F322FD8909F432C5DCCA9DB13F23ACFF49269B77FF87B2FB7C976"_s;
        EXPECT_EQ(PrintInfo{some_other_str}.repr(), some_other_str_expected);
    }
}