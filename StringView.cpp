#include "StringView.h"

namespace ARLib {
    StringView operator""_sv(const char* source, size_t len) {
        return StringView{ source, len + 1 };
    }
}