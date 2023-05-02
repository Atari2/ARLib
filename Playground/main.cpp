#include "Regex.hpp"
#include "Chrono.hpp"
#include "Random.hpp"
#include "Printer.hpp"
#include "File.hpp"

using namespace ARLib;
int main() {
    Regex re = R"(hello ([\s(\d)]*)? (\w .+) world)"_re;
    Vector<CommonTime> times;

    for (size_t i = 0; i < 20; ++i) {
        switch (i % 4) {
            case 0:
                times.push_back(CommonTime{ Seconds{ Random::PCG::random_s() } });
                break;
            case 1:
                times.push_back(CommonTime{ Millis{ Random::PCG::random_s() } });
                break;
            case 2:
                times.push_back(CommonTime{ Micros{ Random::PCG::random_s() } });
                break;
            case 3:
                times.push_back(CommonTime{ Nanos{ Random::PCG::random_s() } });
                break;
        }
    }
    Printer::print("Not sorted: ");
    for (auto& time : times) { Printer::print("{}", time.nanos()); }
    sort(times);
    Printer::print("Sorted: ");
    for (auto& time : times) { Printer::print("{}", time.nanos()); }
    {
        File f{ "test.txt"_p };
        MUST(f.open(OpenFileMode::Write));
        MUST(f.write("Hello world\n"_s));
    }
    Path p{ "test.txt" };
    NativeFileInfo info{ p };
    Printer::print(
    "File size: {}, full path: {}, last modification: {}", info.filesize(), info.path(),
    info.last_modification().to<Seconds>()
    );
}