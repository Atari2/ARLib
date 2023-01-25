#include "../Printer.h"
#include "../CharConv.h"
#include "../HashMap.h"
#include "../String.h"
#include "../Vector.h"
#include "../Enumerate.h"
#include "../JSONParser.h"
#include "../Array.h"
#include "../FileSystem.h"

using namespace ARLib;
int main() {
    Vector<Path> filepaths{};
    for (const auto& filedata : DirectoryIterate{ "jsons"_p, true }) {
        filepaths.append(filedata.path());
    }
    const Vector<String> filedata = filepaths.view()
                                    .filter([](const Path& name) { return name.string().ends_with(FsString{ ".json" }); })
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
}
