#pragma once

#include "Assertion.hpp"
namespace ARLib {
struct Unimplemented {
    Unimplemented(StringView message) { ASSERT_NOT_REACHED_FMT("Unimplemented: %s", message.data()); }
};
struct UnusedArgument {
    template <typename... Args>
    UnusedArgument([[maybe_unused]] Args&&... args) {
        // accepts anything
    }
    void remove_warning() const {}
};
}    // namespace ARLib