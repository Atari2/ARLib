#include "PrintfImpl.hpp"
#include "ArgParser.hpp"
#include "CharConv.hpp"
#include "Printer.hpp"
#include "File.hpp"
#include "JSONParser.hpp"
#include "FlatMap.hpp"
#include "FlatSet.hpp"
#include "Random.hpp"

using namespace ARLib;
static String generate_string(size_t max_len) {
    String s{};
    size_t len = Random::PCG::bounded_random_s(max_len);
    if (len == 0) { len = 1; }
    s.reserve(len);

    constexpr uint32_t start = ' ';
    constexpr uint32_t range = '}' - start;

    for (size_t i = 0; i < len; ++i) { s.append(static_cast<char>(Random::PCG::bounded_random_s(range) + start)); }
    return s;
}
constexpr static size_t n_of_strings = 400;
static Vector<String> strings{};
static void fill_vector() {
    strings.reserve(n_of_strings);
    for (size_t i = 0; i < n_of_strings; ++i) { strings.append(generate_string(32)); }
}
void fill_set(FlatSet<String>& set) {
    for (const auto& s : strings) { set.insert(String{ s }); }
}
void search_set(const FlatSet<String>& set) {
    for (const auto& s : strings) {
        Printer::print("len {} was {}", s.size(), set.find(s) != set.end() ? "found" : "not found");
    }
}
void erase_all(FlatSet<String>& set) {
    for (const auto& s : strings) {
        Printer::print("len {} was {}", s.size(), set.remove(s) ? "removed" : "not removed");
    }
}
void fill_map(FlatMap<String, int>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        set.insert(move(s), static_cast<int>(i));
    }
}
void search_map(const FlatMap<String, int>& set) {
    for (const auto& s : strings) {
        Printer::print("len {} was {}", s.size(), set.find(s) != set.end() ? "found" : "not found");
    }
}
void erase_map(FlatMap<String, int>& set) {
    for (const auto& s : strings) {
        Printer::print("len {} was {}", s.size(), set.remove(s) ? "removed" : "not removed");
    }
}
int main() {
    FlatSet<String> set{};
    fill_set(set);
    search_set(set);
    for (const auto& str : set) { Printer::print("{}", str); }
    erase_all(set);
    for (const auto& str : set) { Printer::print("{}", str); }

    FlatMap<StringView, int> map2{};
    map2.insert("hello"_sv, 1);
    map2.find("hello");

    FlatMap<String, int> map{};
    fill_map(map);
    search_map(map);
    for (const auto& [k, v] : map) { Printer::print("{} {}", k, v); }
    erase_map(map);
    for (const auto& [k, v] : map) { Printer::print("{} {}", k, v); }
    return 0;
}