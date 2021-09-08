#include "../Array.h"
#include "../CharConv.h"
#include "../Chrono.h"
#include "../HashTable.h"
#include "../Optional.h"
#include "../Pair.h"
#include "../Printer.h"
#include "../Result.h"
#include "../SortedVector.h"
#include "../String.h"
#include "../Threading.h"
#include "../Tuple.h"
#include "../Variant.h"
#include "../Vector.h"

using namespace ARLib;

bool func() {
    return true;
}

int main() {
    Pair<String, int> pair{"hello"_s, 10};
    Variant<String, int, Pair<String, int>, Vector<int>> variant{};
    Tuple<String, int, size_t, TimePoint> tup{"hello"_s, 10, 1000, Clock::now()};
    Array<int, 4> arr{1, 2, 3, 4};
    Printer::print("{}, {}, {}, {}", pair, variant, tup, arr);
    HashTable<UniqueLock<Mutex>> tbl{};
    Mutex mut{};
    Mutex mut2{};
    RecursiveMutex rmut{};

    tbl.insert(UniqueLock{mut});
    UniqueLock loc{mut, defer_lock};
    ScopedLock sloc{mut2, rmut};
    auto iter = tbl.find(loc);
    if (iter == tbl.tend()) {
        Printer::print("Lock not found\n");
    } else {
        Printer::print("Lock found: {}\n", (*iter));
    }
    Result<String, int> res{"Hello"_s};
    Result<String> res2 = Result<String>::from_error();
    Thread t{func};
    Optional<String> opt{};
    Optional<String> opt2{"hello world"_s};
    Printer::print("{}, {}, {}, {}, {}, {}", res, res2, opt, opt2, loc, t);
    Printer::print("{}", sloc);
    t.join();
    return 0;
}