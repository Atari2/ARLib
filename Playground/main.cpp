#include "../PrintfImpl.h"
#include "../ArgParser.h"
#include "../CharConv.h"
#include "../Printer.h"
#include "../File.h"
#include "../JSONParser.h"
#include "../FlatMap.h"
#include "../FlatSet.h"
#include "../HashTable.h"
using namespace ARLib;

constexpr static size_t n_of_strings = 400;
void fill_set(FlatSet<String>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        set.insert(move(s));
    }
}
void search_set(const FlatSet<String>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        Printer::print("len {} was {}", s.size(), set.find(s) != set.end() ? "found" : "not found");
    }
}
void erase_all(FlatSet<String>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        Printer::print("len {} was {}", s.size(), set.remove(s) ? "removed" : "not removed");
    }
}

void fill_map(FlatMap<String, int>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        set.insert(move(s), i);
    }
}
void search_map(const FlatMap<String, int>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
        Printer::print("len {} was {}", s.size(), set.find(s) != set.end() ? "found" : "not found");
    }
}
void erase_map(FlatMap<String, int>& set) {
    for (size_t i = 1; i < n_of_strings; ++i) {
        String s{ i, 'a' };
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
    map.remove("aaaa"_s);
    for (const auto& [k, v] : map) { Printer::print("{} {}", k, v); }
    erase_map(map);
    for (const auto& [k, v] : map) { Printer::print("{} {}", k, v); }
    return 0;
}