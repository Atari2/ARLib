#include "Random.hpp"

#include "Chrono.hpp"

#ifdef COMPILER_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4146)
#endif
namespace ARLib {
namespace Random {
    PCG::PCG(uint64_t state, uint64_t inc) : m_state(state), m_inc(inc) {}
    PCG PCG::create(uint64_t initstate, uint64_t initseq) {
        PCG state{ 0, 0 };
        state.seed(initstate, initseq);
        return state;
    }
    PCG PCG::create() {
        PCG state{ 0, 0 };
        state.seed(
        static_cast<uint64_t>(DateClock::now().raw_value().value) ^ reinterpret_cast<uintptr_t>(&ARLib::DateClock::diff),
        reinterpret_cast<uintptr_t>(&PCG::bounded_random_s)
        );
        return state;
    }
    PCG& PCG::static_state() {
        static PCG state{ 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };
        return state;
    }
    void PCG::seed(uint64_t initstate, uint64_t initseq) {
        m_state = 0;
        m_inc   = (initseq << 1u) | 1u;
        random();
        m_state += initstate;
        random();
    }
    void PCG::seed_s(uint64_t seed, uint64_t seq) {
        static_state().seed(seed, seq);
    }
    uint32_t PCG::random() {
        uint64_t oldstate   = m_state;
        m_state             = oldstate * 6364136223846793005ULL + m_inc;
        uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot        = static_cast<uint32_t>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }
    uint32_t PCG::random_s() {
        return static_state().random();
    }
    uint32_t PCG::bounded_random(uint32_t bound) {
        uint32_t threshold = -bound % bound;
        for (;;) {
            uint32_t r = random();
            if (r >= threshold) return r % bound;
        }
    }
    uint32_t PCG::bounded_random_s(uint32_t bound) {
        return static_state().bounded_random(bound);
    }
}    // namespace Random
}    // namespace ARLib
#ifdef COMPILER_MSVC
    #pragma warning(pop)
#endif
