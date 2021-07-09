#include "Suite.h"

int main() {
    if (ARLib::run_all_tests()) return 0;
    return 1;
}