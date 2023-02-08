#include "../Printer.h"
#include "../String.h"
#include "../Vector.h"
#include "../Enumerate.h"
#include "../JSONParser.h"
#include "../FileSystem.h"
#include "../Set.h"
#include "../List.h"

using namespace ARLib;
int main() {
    Vector<Path> filepaths{};
    for (const auto& filedata : DirectoryIterate{ "jsons"_p, true }) { filepaths.append(filedata.path()); }
    const Vector<String> filedata = filepaths.view()
                                    .filter([](const Path& name) { return name.extension() == ".json"_p; })
                                    .map([](const auto& name) {
                                        auto text = MUST(File::read_all(name));
                                        return text;
                                    })
                                    .collect<Vector<String>>();
    for (const auto& [data, file] : zip(filedata, filepaths)) {
        Printer::print("Parsing {}", file);
        auto res = JSON::Parser::parse(data.view());
        if (res.is_error()) { Printer::print("Error during parsing {}", res.to_error()); }
    }
    Printer::print("Globbed {} files", filepaths.size());
    Vector<String> v{ "1"_s, "2"_s, "3"_s };
    Set<String> s{ "Hello"_s, "world"_s, "foo"_s };
    LinkedList<int> l{ 2, 4, 5 };
    HashMap<String, int> m{
        {"foo"_s,  6},
        { "bar"_s, 7},
        { "biz"_s, 8},
    };
    const HashTable<String> tbl{};
    tbl.find("hello"_s);
    for (const auto& [t1, t2, t3, t4] : zip(v, s, l, m)) {
        const auto& [key, val] = t4;
        Printer::print("{}, {}, {}, {}, {}", t1, t2, t3, key, val);
    }
}