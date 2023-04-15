#include "PrintfImpl.hpp"
#include "ArgParser.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "File.hpp"
#include "JSONParser.hpp"
#include "FlatMap.hpp"
#include "FlatSet.hpp"
#include "Random.hpp"
#include "Set.hpp"
#include "Threading.hpp"
#include "Async.hpp"

#define EXPECT_TRUE(a)  HARD_ASSERT((static_cast<bool>(a) == true), "asdf");
#define EXPECT_EQ(a, b) HARD_ASSERT((a == b), "asdf")
#define EXPECT_NE(a, b) HARD_ASSERT((a != b), "asdf")

using namespace ARLib;
int main() {
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
    Atomic<bool> done{ false };
    auto fill_strings_thread = create_async_task([&]() {
        while (strings.size() < n_of_strings && !done) { strings.insert(generate_string(32)); }
        done = true;
    });

    if (fill_strings_thread.wait_for(10'000'000) == FutureStatus::Timeout) {
        Printer::print("Filling the set with strings timed out");
        // ask for the thread to stop and wait for it
        done = true;
        fill_strings_thread.wait();
    } else {
        Printer::print("Filling the set with strings succeeded");
    }

    fill_set();
    search_set();
    erase_set();
    fill_map();
    search_map();
    erase_map();
}