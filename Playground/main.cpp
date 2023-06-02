#include "NextVariant.hpp"
#include "String.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"

using namespace ARLib;
int main() {
    /*
    auto oldest_date = Date{ Instant::from_nanos(0_ns), Date::Timezone::No };
    Printer::print("{}", oldest_date);

    auto path = windows_build ? R"(C:\Users\aless\source\repos\ARLib\source)"_p : R"(/home/alessio/.vs/ARLib/source)"_p;

    auto cont = DirectoryIterate{ path, true };

    Vector<FileInfo> entries{};

    for (const auto& entry : cont) {
        entries.append(entry);
        if (entry.is_directory()) {
            Printer::print("{} is a directory", entry.path());
        } else {
            Printer::print("{} is a file", entry.path());
        }
    }

    sort(entries, [](const FileInfo& a, const FileInfo& b) { return a.last_modification() <=> b.last_modification(); });

    for (const auto& entry : entries) { Printer::print("Last modification at: {}", Date{ entry.last_modification() }); }
    */

    v2::Variant<int, double, Vector<int>, String, Pair<int, double>> t1{
        "Hello world this is a long string that should be freed"
    };
    auto s        = v2::VariantStorage<10, 8>{};
    const auto s2        = v2::VariantStorage<10, 8>{};
    using T = decltype(s.raw_memory());
    using T2 = decltype(s2.raw_memory());
    const auto& v = t1.get<String>();
    Printer::print("{}", v);
    t1 = 1;
    t1.set(Vector{ 1, 2, 3 });
    Printer::print("{}", t1.current_type());
    const auto& v2 = t1.get<Vector<int>>();
    t1.visit([](const auto& asdf) { Printer::print("{}", asdf); });
    /*
    constexpr Array sizes{ sizeof(int), sizeof(double), sizeof(Vector<int>), sizeof(String) };

    constexpr auto max_size = *max(sizes);

    constexpr auto total = sum(sizes, [](size_t a) { return a;  });

    Variant<int, double, Vector<int>, String> t2{};
    constexpr size_t t1size = sizeof(t1);
    constexpr size_t t2size = sizeof(t2);
    */
    return 0;
}