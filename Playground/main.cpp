#include "../CharConv.h"
#include "../JSONParser.h"
#include "../Printer.h"
using namespace ARLib;

class Test {
    public:
    int hello = 0;

    static JSON::ParseResultT<Test> deserialize(StringView view) { 
        auto obj = JSON::Parser::parse(view);
        if (obj.is_error()) return JSON::ParseResultT<Test>::from_error(obj.to_error());
        auto json = obj.to_ok();
        Test t{};
        t.hello = json["hello"_s].get<JSON::Type::JNumber>();
        return t;
    }
    String serialize() const { 
        JSON::Object obj{};
        obj.add("hello"_s, JSON::ValueObj::construct<JSON::Number>(hello));
        return JSON::dump_json(obj);
    }
};

int main() {
    auto res = Test::deserialize(R"({"hello": 123})"_sv);
    Printer::print("{}", res.to_ok().serialize());
    return 0;
}