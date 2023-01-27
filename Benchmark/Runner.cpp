#include "../Printer.h"
#include "../CharConv.h"
#include "../HashMap.h"
#include "../String.h"
#include "../Vector.h"
#include "../Enumerate.h"
#include "../JSONParser.h"
#include "../Array.h"
#include "../Chrono.h"
#include "../Assertion.h"
#include <benchmark/benchmark.h>
#include <inttypes.h>
#include <unordered_map>
#include <string_view>
#include <array>
using namespace std::string_view_literals;

using namespace ARLib;
static void BM_PrintfARLib(benchmark::State& state) {
    char buf[1024]{};
    for (auto _ : state) {
        double val              = 1234.1234;
        const char* s           = "my name is alessio";
        int hex                 = 0x50;
        ARLib::int64_t int64val = INT64_MAX;
        ARLib::sprintf(buf, "Hello World %+10.4f %% %s %#02o %#02I64X %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}
constexpr static const Array strings{
    "XZk6g68IJe"_sv, "xLX1I3D9Ju"_sv, "c8iChK6P5U"_sv, "ke1fjfqM4h"_sv, "C14RwSqFaK"_sv, "ZYApUXmw8i"_sv,
    "NSsOfFyYQw"_sv, "2embvf20ZJ"_sv, "QXreMhn9Rk"_sv, "OyTfRkPoWP"_sv, "kHXnimdjGb"_sv, "mtylTdDHs8"_sv,
    "HjDNsHZJ0O"_sv, "o0MmzO0OQ7"_sv, "C81lYiEoPK"_sv, "fxpSWLS0in"_sv, "QWLODBo9lj"_sv, "8N0MIpeP8j"_sv,
    "3xbvOimAaK"_sv, "wj41wl0ifb"_sv, "vjHFCWCKU1"_sv, "wQHSdcKmrn"_sv, "PXTMNP75hq"_sv, "L7TuxocgEG"_sv,
    "g7bQbXwCwG"_sv, "OTgkPNbdRG"_sv, "GXVpodrMcv"_sv, "uXsiZQv2KK"_sv, "144IRet2MM"_sv, "eQWNvRkaFh"_sv,
    "PDggPLAcQD"_sv, "WV4gj6eVx3"_sv, "H2az78ED8Q"_sv, "G3BKZOi5AP"_sv, "00eUWw5keT"_sv, "EekC8u3Fgp"_sv,
    "zBdebgoSjW"_sv, "NpetUuoijR"_sv, "hdNt1J3d35"_sv, "Kyi8aIHEeo"_sv, "dR0WL76qoO"_sv, "ggrxoFagtP"_sv,
    "qDJN7Clkcd"_sv, "8xPcyvzstU"_sv, "Fp0V3JBAQ1"_sv, "CAAIyvZJ2l"_sv, "jHFIKEqoDP"_sv, "0ePRmgMGJj"_sv,
    "UlCy28HaHT"_sv, "ULgHyVJVX6"_sv, "cG624ueEoO"_sv, "A1hg0xHLfQ"_sv, "C2kkYZkTtF"_sv, "1yUtwevLgb"_sv,
    "za9G6Ry7RA"_sv, "Kkfk6ZTfse"_sv, "20y9tEFtv7"_sv, "jbg7UzBTE1"_sv, "qjlGhPxKn0"_sv, "PgNHVLj5GD"_sv,
    "2I6GpgwhEj"_sv, "4hVs495EX4"_sv, "XdxSo7jBgU"_sv, "inlBP3ELRI"_sv, "h3mUk3Cw33"_sv, "4rHb12uhoa"_sv,
    "DrycQ6SL94"_sv, "GglNpPbzWw"_sv, "J9gtEsYZ48"_sv, "MpXA02CaUr"_sv, "XwTX7hCsA0"_sv, "p7USMCQ140"_sv,
    "yk14lFbf6S"_sv, "OxPYxbX4Up"_sv, "RRkhyOXK6j"_sv, "6guDMubTGf"_sv, "xdgN9VZ5xJ"_sv, "sZVJ6V5bzs"_sv,
    "mlBklidD2m"_sv, "vTPq38FlDd"_sv, "KC7cKE88kp"_sv, "CDtuCg6FoP"_sv, "pJUrm9kB7t"_sv, "l493YSmNkc"_sv,
    "oTI3T4gRRt"_sv, "qZLEQQYxor"_sv, "sRnv5wXIqn"_sv, "VTZRkswfh7"_sv, "q69Vq4wpIK"_sv, "SWQ6dU4wLs"_sv,
    "nHTVA6jzsS"_sv, "2phWwNfCHp"_sv, "HQ6oNmPko5"_sv, "LgUFsTN0eC"_sv, "hlDB7lRGJ0"_sv, "8kGMMJV7PR"_sv,
    "M798JiFBuA"_sv, "Sq51RKKFUZ"_sv, "6Lg6W9hZ9X"_sv, "HA741LK6Ai"_sv,
};
constexpr static const Array stdstrings{
    "XZk6g68IJe"sv, "xLX1I3D9Ju"sv, "c8iChK6P5U"sv, "ke1fjfqM4h"sv, "C14RwSqFaK"sv, "ZYApUXmw8i"sv,
    "NSsOfFyYQw"sv, "2embvf20ZJ"sv, "QXreMhn9Rk"sv, "OyTfRkPoWP"sv, "kHXnimdjGb"sv, "mtylTdDHs8"sv,
    "HjDNsHZJ0O"sv, "o0MmzO0OQ7"sv, "C81lYiEoPK"sv, "fxpSWLS0in"sv, "QWLODBo9lj"sv, "8N0MIpeP8j"sv,
    "3xbvOimAaK"sv, "wj41wl0ifb"sv, "vjHFCWCKU1"sv, "wQHSdcKmrn"sv, "PXTMNP75hq"sv, "L7TuxocgEG"sv,
    "g7bQbXwCwG"sv, "OTgkPNbdRG"sv, "GXVpodrMcv"sv, "uXsiZQv2KK"sv, "144IRet2MM"sv, "eQWNvRkaFh"sv,
    "PDggPLAcQD"sv, "WV4gj6eVx3"sv, "H2az78ED8Q"sv, "G3BKZOi5AP"sv, "00eUWw5keT"sv, "EekC8u3Fgp"sv,
    "zBdebgoSjW"sv, "NpetUuoijR"sv, "hdNt1J3d35"sv, "Kyi8aIHEeo"sv, "dR0WL76qoO"sv, "ggrxoFagtP"sv,
    "qDJN7Clkcd"sv, "8xPcyvzstU"sv, "Fp0V3JBAQ1"sv, "CAAIyvZJ2l"sv, "jHFIKEqoDP"sv, "0ePRmgMGJj"sv,
    "UlCy28HaHT"sv, "ULgHyVJVX6"sv, "cG624ueEoO"sv, "A1hg0xHLfQ"sv, "C2kkYZkTtF"sv, "1yUtwevLgb"sv,
    "za9G6Ry7RA"sv, "Kkfk6ZTfse"sv, "20y9tEFtv7"sv, "jbg7UzBTE1"sv, "qjlGhPxKn0"sv, "PgNHVLj5GD"sv,
    "2I6GpgwhEj"sv, "4hVs495EX4"sv, "XdxSo7jBgU"sv, "inlBP3ELRI"sv, "h3mUk3Cw33"sv, "4rHb12uhoa"sv,
    "DrycQ6SL94"sv, "GglNpPbzWw"sv, "J9gtEsYZ48"sv, "MpXA02CaUr"sv, "XwTX7hCsA0"sv, "p7USMCQ140"sv,
    "yk14lFbf6S"sv, "OxPYxbX4Up"sv, "RRkhyOXK6j"sv, "6guDMubTGf"sv, "xdgN9VZ5xJ"sv, "sZVJ6V5bzs"sv,
    "mlBklidD2m"sv, "vTPq38FlDd"sv, "KC7cKE88kp"sv, "CDtuCg6FoP"sv, "pJUrm9kB7t"sv, "l493YSmNkc"sv,
    "oTI3T4gRRt"sv, "qZLEQQYxor"sv, "sRnv5wXIqn"sv, "VTZRkswfh7"sv, "q69Vq4wpIK"sv, "SWQ6dU4wLs"sv,
    "nHTVA6jzsS"sv, "2phWwNfCHp"sv, "HQ6oNmPko5"sv, "LgUFsTN0eC"sv, "hlDB7lRGJ0"sv, "8kGMMJV7PR"sv,
    "M798JiFBuA"sv, "Sq51RKKFUZ"sv, "6Lg6W9hZ9X"sv, "HA741LK6Ai"sv,
};
static void BM_PrintfStd(benchmark::State& state) {
    char buf[1024]{};
    for (auto _ : state) {
        double val              = 1234.1234;
        const char* s           = "my name is alessio";
        int hex                 = 0x50;
        ARLib::int64_t int64val = INT64_MAX;
        ::sprintf(buf, "Hello World %+10.4f %% %s %#02o %#02" PRIX64 " %%\n", val, s, hex, int64val);
        benchmark::ClobberMemory();
    }
}

static void BM_StdUnorderedMap(benchmark::State& state) {
    std::unordered_map<StringView, int, Hash<StringView>> stdmap{};
    for (auto _ : state) {
        stdmap.clear();
        int i = 0;
        for (const auto& s : strings) {
            stdmap.insert({ s, ++i });
            if (auto it = stdmap.find(s); it != stdmap.end()) {
                if ((*it).second != i) { ASSERT_NOT_REACHED("Value is wrong"); }
            } else {
                ASSERT_NOT_REACHED("Value not found");
            }
        }
        if (stdmap.size() != strings.size()) { ASSERT_NOT_REACHED("Map size is wrong") }
        for (const auto& s : strings) {
            stdmap.erase(s);
        }
        if (stdmap.size() != 0) { ASSERT_NOT_REACHED("Map size is wrong") }
    }
}
static void BM_ARLibHashMap(benchmark::State& state) {
    HashMap<StringView, int> map{};
    for (auto _ : state) {
        map.clear();
        int i = 0;
        for (const auto& s : strings) {
            map.add(s, ++i);
            if (auto it = map.find(s); it != map.end()) {
                if ((*it).value() != i) { ASSERT_NOT_REACHED("Value is wrong"); }
            } else {
                ASSERT_NOT_REACHED("Value not found");
            }
        }
        if (map.size() != strings.size()) { ASSERT_NOT_REACHED("Map size is wrong") }
        for (const auto& s : strings) {
            map.remove(s);
        }
        if (map.size() != 0) { ASSERT_NOT_REACHED("Map size is wrong") }
    }
}
// BENCHMARK(BM_PrintfStd);
// BENCHMARK(BM_PrintfARLib);
BENCHMARK(BM_StdUnorderedMap);
BENCHMARK(BM_ARLibHashMap);
BENCHMARK_MAIN();
