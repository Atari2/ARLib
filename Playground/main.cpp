#include "Regex.hpp"
#include "Chrono.hpp"
#include "Random.hpp"
#include "Printer.hpp"
#include "File.hpp"
#include "FileSystem.hpp"

using namespace ARLib;
int main() {
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

    return 0;
}