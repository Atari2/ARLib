#pragma once

#include "Types.hpp"
namespace ARLib {
namespace Random {
    // minimal PCG 32-bit implementation
    // based on https://github.com/imneme/pcg-c-basic
    // TODO: expand this to all of the implementations from https://github.com/imneme/pcg-cpp

    class PCG {
        uint64_t m_state;
        uint64_t m_inc;
        PCG(uint64_t state, uint64_t inc);

        public:
        static PCG create(uint64_t state, uint64_t inc);
        static PCG create();
        static PCG& static_state();
        const uint64_t& state() const { return m_state; }
        const uint64_t& inc() const { return m_inc; }
        void seed(uint64_t initstate, uint64_t initseq);
        static void seed_s(uint64_t seed, uint64_t seq);
        uint32_t random();
        static uint32_t random_s();
        uint32_t bounded_random(uint32_t bound);
        static uint32_t bounded_random_s(uint32_t bound);
    };
}    // namespace Random
}    // namespace ARLib
