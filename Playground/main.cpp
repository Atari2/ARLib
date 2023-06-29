#include "String.hpp"
#include "Variant.hpp"
#include "Vector.hpp"
#include "Array.hpp"
#include "Printer.hpp"
#include "CharConv.hpp"
#include "JSONParser.hpp"
#include "FileSystem.hpp"
using namespace ARLib;
int main() {
    const auto path = windows_build ? R"(C:\Users\aless\Downloads\package)"_p : R"(/home/alessio/package)"_p;
    Timer c{};
    auto data = iterate_directory(path, DirIterType::Recursive)
                .iter()
                .filter([](const FileInfo& info) { return info.is_file() && info.path().extension() == ".json"_p; })
                .map([](const FileInfo& p) {
                    const auto str = MUST(File::read_all(p.path()));
                    auto res       = JSON::Parser::parse(str.view());
                    if (res.is_error()) {
                        auto err = res.to_error();
                        auto j   = MUST(JSON::Parser::parse(str.substringview(0, err.offset())));
                        return Pair{ FileInfo{ p }, j };
                    }
                    return Pair{ FileInfo{ p }, res.to_ok() };
                })
                .collect<Vector>();
    sort(data, [](const auto& lhs, const auto& rhs) { return rhs.first().filesize() <=> lhs.first().filesize(); });
    const auto total = data.iter().map([](const auto& p) { return p.first().filesize(); }).sum();
    Printer::print(
    "Parsed {} (total of {} GB) json files in {}", data.size(), (double)total / (double)(1_u64 << 30),
    c.elapsed().to_common().millis()
    );
    return 0;
}