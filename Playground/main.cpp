#include "../PrintfImpl.h"
#include "../ArgParser.h"
#include "../CharConv.h"
#include "../Printer.h"
#include "../File.h"
#include "../JSONParser.h"
using namespace ARLib;
struct TestJson {
    int val;
    String str;
    static JSON::Parsed<TestJson> deserialize(StringView string) {
        auto obj = JSON::Parser::parse(string);
        if (obj.is_error()) return obj.to_error();
        auto jval = obj.to_ok();
        TestJson val{ .val = jval["val"_s].get<JSON::Type::JNumber>(),
                      .str = jval["str"_s].get<JSON::Type::JString>() };
        return val;
    }
    String serialize() const { 
        JSON::Object obj;
        obj.add("val"_s, JSON::Number{val});
        obj.add("str"_s, JSON::JString{ str });
        return JSON::dump_json(obj);
    }
};
int main(int argc, char** argv) {
    Result r{ 1 };
    Result<int> r2{ BacktraceError{ "hello"_s } };
    HARD_ASSERT(r.is_ok(), "1");
    HARD_ASSERT(!r.is_error(), "2");
    HARD_ASSERT(r.ok_value() == 1, "3");
    ArgParser parser{ argc, argv };
    String filename;
    parser.add_option("--file", "FILE", "File to pass", filename);
    if (auto res = parser.parse(); res.is_error()) { Printer::print("{}", res.to_error()); }
    if (parser.help_requested()) { parser.print_help(); }
    auto j = R"({"hello world": 1})"_json;
    auto obj = MUST(TestJson::deserialize(R"({"val": 10, "str": "Hello World!"})"_sv));
    Printer::print("{}", obj.serialize());
}