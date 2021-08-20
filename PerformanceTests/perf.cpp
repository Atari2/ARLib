#include "../HashMap.h"
#include "../cstdio_compat.h"
#include "PerfTime.h"

using namespace ARLib;

int main() {
    PERF_SUITE_START(test)
    ADD_PERF_START(test, test_perf)
    Vector<String> strings{};
    for (size_t i = 0; i < 1000; i++)
        strings.append(String{i, 'a'});
    ADD_PERF_END
    ADD_PERF_START(test, test_perf_2)
    HashMap<String, size_t> map{};
    for (size_t i = 0; i < 1000; i++)
        map.add(String{i, 'a'}, i);
    ADD_PERF_END
    PERF_SUITE_END
}