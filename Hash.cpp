#include "Hash.h"

#include "Conversion.h"
#include "Vector.h"

#ifdef COMPILER_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#elif COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wsign-conversion"
#endif
namespace ARLib {
HashAlgorithm<HashType::MD5>::MD5Result HashAlgorithm<HashType::MD5>::calculate(ReadOnlyByteView data) {
    constexpr uint32_t s[] = { 7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 5,  9,  14, 20, 5,  9,
                               14, 20, 5,  9,  14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16, 23, 4,  11, 16, 23,
                               4,  11, 16, 23, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21 };
    constexpr uint32_t K[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613,
                               0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193,
                               0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d,
                               0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                               0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
                               0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
                               0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244,
                               0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                               0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb,
                               0xeb86d391 };
    uint32_t a0            = 0x67452301;
    uint32_t b0            = 0xefcdab89;
    uint32_t c0            = 0x98badcfe;
    uint32_t d0            = 0x10325476;
    Vector<uint8_t> glob{};

    auto leftrotate = [](auto a, auto b) {
        return (a << b) | (a >> ((sizeof(decltype(a)) * 8) - b));
    };

    const size_t sz = data.size();
    glob.set_size(sz);
    ARLib::memcpy(&glob.index_unchecked(0), data.data(), sz);
    // append a single "1" bit
    glob.append(0x80);
    size_t length_in_bits = glob.size() * 8;
    while ((length_in_bits % 512) != 448) {
        glob.append(0x00);
        length_in_bits += 8;
    }
    uint8_t size_in_bits_u8[sizeof(size_t)]{ 0 };
    size_t og_size_in_bits = (sz * 8_sz); /* % (1 << 64) */
    ARLib::memcpy(size_in_bits_u8, cast<const uint8_t*>(&og_size_in_bits), sizeof(size_t));
    for (auto val : size_in_bits_u8) glob.append(val);
    HARD_ASSERT(glob.size() % 64 == 0, "Message size in bits should be divisible by 512");
    // do algorithm here:
    // for each 512-bit chunk do
    constexpr size_t CHUNK_SIZE = 512;

    for (size_t chunk_num = 0; chunk_num < glob.size(); chunk_num += CHUNK_SIZE / BITS_PER_BYTE) {
        uint32_t words[16]{ 0 };
        ARLib::memcpy(words, &glob[chunk_num], sizeof(uint32_t) * 16);
        auto a = a0;
        auto b = b0;
        auto c = c0;
        auto d = d0;
        for (uint32_t j = 0; j < 64; j++) {
            uint32_t f = 0, g = 0;
            if (j < 16) {
                f = (b & c) | (~b & d);
                g = j;
            } else if (j < 32) {
                f = (d & b) | (~d & c);
                g = (5 * j + 1) % 16;
            } else if (j < 48) {
                f = b ^ c ^ d;
                g = (3 * j + 5) % 16;
            } else /* if (j >= 48 && j <= 63) */ {
                f = c ^ (b | ~d);
                g = (7 * j) % 16;
            }
            f = f + a + K[j] + words[g];
            a = d;
            d = c;
            c = b;
            b = b + leftrotate(f, s[j]);
        }
        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }
    MD5Result res{};
    constexpr size_t u32s = NumberTraits<uint32_t>::size;
    ARLib::memcpy(res.digest + (u32s * 0), &a0, u32s);
    ARLib::memcpy(res.digest + (u32s * 1), &b0, u32s);
    ARLib::memcpy(res.digest + (u32s * 2), &c0, u32s);
    ARLib::memcpy(res.digest + (u32s * 3), &d0, u32s);
    return res;
}
HashAlgorithm<HashType::MD5>::MD5Result HashAlgorithm<HashType::MD5>::calculate(ReadOnlyCharView data) {
    return calculate(ReadOnlyByteView{ cast<const uint8_t*>(data.data()),
                                       cast<const uint8_t*>(data.data() + data.size()) });
}
HashAlgorithm<HashType::SHA1>::SHA1Result HashAlgorithm<HashType::SHA1>::calculate(ReadOnlyByteView data) {
    using u32 = uint32_t;

    u32 h0 = 0x67452301;
    u32 h1 = 0xEFCDAB89;
    u32 h2 = 0x98BADCFE;
    u32 h3 = 0x10325476;
    u32 h4 = 0xC3D2E1F0;

    Vector<uint8_t> glob{};

    const size_t sz = data.size();
    glob.set_size(sz);
    ARLib::memcpy(&glob.index_unchecked(0), data.data(), sz);
    // append a single "1" bit
    glob.append(0x80);
    size_t length_in_bits = glob.size() * 8;
    while ((length_in_bits % 512) != 448) {
        glob.append(0x00);
        length_in_bits += 8;
    }
    uint8_t size_in_bits_u8[sizeof(size_t)]{ 0 };
    size_t og_size_in_bits = (sz * 8_sz); /* % (1 << 64) */
    auto* og_size_ptr      = cast<const uint8_t*>(&og_size_in_bits);
    for (int i = sizeof(size_t) - 1; i >= 0; i--) { size_in_bits_u8[sizeof(size_t) - i - 1] = og_size_ptr[i]; }

    for (auto val : size_in_bits_u8) glob.append(val);

    auto leftrotate = [](auto a, auto b) {
        return (a << b) | (a >> ((sizeof(decltype(a)) * 8) - b));
    };

    constexpr size_t CHUNK_SIZE_BITS = 512;
    constexpr size_t CHUNK_SIZE      = CHUNK_SIZE_BITS / BITS_PER_BYTE;

    auto chunk_to_u32 = [](const Vector<uint8_t>& full_msg, uint32_t(&block)[80], size_t idx) {
        auto* chunk = &full_msg[idx];
        for (size_t i = 0, j = 0; i < CHUNK_SIZE / sizeof(u32); i++, j += 4) {
            block[i] = ((chunk[j] << 24_u32) | (chunk[j + 1] << 16_u32) | (chunk[j + 2] << 8_u32) | chunk[j + 3_u32]);
        }
    };

    auto u32_to_bytes_be = [](uint8_t(&digest)[20], size_t cnt, u32 value) {
        digest[cnt]     = (value >> 24) & 0xFF;
        digest[cnt + 1] = (value >> 16) & 0xFF;
        digest[cnt + 2] = (value >> 8) & 0xFF;
        digest[cnt + 3] = value & 0xFF;
    };

    for (size_t chunk_num = 0; chunk_num < glob.size(); chunk_num += CHUNK_SIZE) {
        uint32_t w[80]{ 0 };
        chunk_to_u32(glob, w, chunk_num);

        for (size_t i = 16; i < 80; i++) { w[i] = leftrotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1); }
        auto a = h0;
        auto b = h1;
        auto c = h2;
        auto d = h3;
        auto e = h4;
        for (uint32_t j = 0; j < 80; j++) {
            u32 f = 0;
            u32 k = 0;
            if (j < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (j < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (j < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            u32 temp = leftrotate(a, 5) + f + e + k + w[j];
            e        = d;
            d        = c;
            c        = leftrotate(b, 30);
            b        = a;
            a        = temp;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }
    SHA1Result res{};
    constexpr size_t u32s = NumberTraits<uint32_t>::size;
    u32_to_bytes_be(res.digest, u32s * 0, h0);
    u32_to_bytes_be(res.digest, u32s * 1, h1);
    u32_to_bytes_be(res.digest, u32s * 2, h2);
    u32_to_bytes_be(res.digest, u32s * 3, h3);
    u32_to_bytes_be(res.digest, u32s * 4, h4);
    return res;
}
HashAlgorithm<HashType::SHA1>::SHA1Result HashAlgorithm<HashType::SHA1>::calculate(ReadOnlyCharView data) {
    return calculate(ReadOnlyByteView{ cast<const uint8_t*>(data.data()),
                                       cast<const uint8_t*>(data.data() + data.size()) });
}
HashAlgorithm<HashType::SHA256>::SHA256Result HashAlgorithm<HashType::SHA256>::calculate(ReadOnlyByteView data) {
    using u32 = uint32_t;
    u32 h0    = 0x6A09E667;
    u32 h1    = 0xBB67AE85;
    u32 h2    = 0x3C6EF372;
    u32 h3    = 0xA54FF53A;
    u32 h4    = 0x510E527F;
    u32 h5    = 0x9B05688C;
    u32 h6    = 0x1F83D9AB;
    u32 h7    = 0x5BE0CD19;

    constexpr u32 k[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
                            0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
                            0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
                            0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
                            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
                            0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
                            0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
                            0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
                            0xc67178f2 };

    Vector<uint8_t> glob{};

    const size_t sz = data.size();
    glob.set_size(sz);
    ARLib::memcpy(&glob.index_unchecked(0), data.data(), sz);
    // append a single "1" bit
    glob.append(0x80);
    size_t length_in_bits = glob.size() * 8;
    while ((length_in_bits % 512) != 448) {
        glob.append(0x00);
        length_in_bits += 8;
    }
    uint8_t size_in_bits_u8[sizeof(size_t)]{ 0 };
    size_t og_size_in_bits = (sz * 8_sz); /* % (1 << 64) */
    auto* og_size_ptr      = cast<const uint8_t*>(&og_size_in_bits);
    for (size_t i = sizeof(size_t) - 1;; i--) {
        size_in_bits_u8[sizeof(size_t) - i - 1_sz] = og_size_ptr[i];
        if (i == 0) break;
    }

    for (auto val : size_in_bits_u8) glob.append(val);

    // the commented out _u32 causes a linker error on clang??
    // I don't really understand this one since the same function is used like 4 lines after this
    // in the other lambda, probably a compiler error, too lazy to investing and/or report
    // easier to just comment out
    auto rightrotate = [](auto a, auto b) {
        return (a >> b) | (a << ((sizeof(decltype(a)) * 8 /* _u32 */) - b));
    };

    constexpr size_t CHUNK_SIZE_BITS = 512;
    constexpr size_t CHUNK_SIZE      = CHUNK_SIZE_BITS / BITS_PER_BYTE;

    auto chunk_to_u32 = [](const Vector<uint8_t>& full_msg, uint32_t(&block)[64], size_t idx) {
        auto* chunk = &full_msg[idx];
        for (size_t i = 0, j = 0; i < CHUNK_SIZE / sizeof(u32); i++, j += 4) {
            block[i] = ((chunk[j] << 24_u32) | (chunk[j + 1] << 16_u32) | (chunk[j + 2] << 8_u32) | chunk[j + 3_u32]);
        }
    };

    auto u32_to_bytes_be = [](uint8_t(&digest)[32], size_t cnt, u32 value) {
        digest[cnt]     = (value >> 24) & 0xFF;
        digest[cnt + 1] = (value >> 16) & 0xFF;
        digest[cnt + 2] = (value >> 8) & 0xFF;
        digest[cnt + 3] = value & 0xFF;
    };

    for (size_t chunk_num = 0; chunk_num < glob.size(); chunk_num += CHUNK_SIZE) {
        uint32_t w[64]{ 0 };
        chunk_to_u32(glob, w, chunk_num);

        for (size_t i = 16; i < 64; i++) {
            u32 s0 = rightrotate(w[i - 15], 7) ^ rightrotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            u32 s1 = rightrotate(w[i - 2], 17) ^ rightrotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i]   = w[i - 16] + s0 + w[i - 7] + s1;
        }
        auto a = h0;
        auto b = h1;
        auto c = h2;
        auto d = h3;
        auto e = h4;
        auto f = h5;
        auto g = h6;
        auto h = h7;

        for (uint32_t j = 0; j < 64; j++) {
            u32 s0  = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
            u32 maj = (a & b) ^ (a & c) ^ (b & c);
            u32 t2  = s0 + maj;
            u32 s1  = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
            u32 ch  = (e & f) ^ (~e & g);
            u32 t1  = h + s1 + ch + k[j] + w[j];
            h       = g;
            g       = f;
            f       = e;
            e       = d + t1;
            d       = c;
            c       = b;
            b       = a;
            a       = t1 + t2;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }
    SHA256Result res{};
    constexpr size_t u32s = NumberTraits<uint32_t>::size;
    u32_to_bytes_be(res.digest, u32s * 0, h0);
    u32_to_bytes_be(res.digest, u32s * 1, h1);
    u32_to_bytes_be(res.digest, u32s * 2, h2);
    u32_to_bytes_be(res.digest, u32s * 3, h3);
    u32_to_bytes_be(res.digest, u32s * 4, h4);
    u32_to_bytes_be(res.digest, u32s * 5, h5);
    u32_to_bytes_be(res.digest, u32s * 6, h6);
    u32_to_bytes_be(res.digest, u32s * 7, h7);
    return res;
}
HashAlgorithm<HashType::SHA256>::SHA256Result HashAlgorithm<HashType::SHA256>::calculate(ReadOnlyCharView data) {
    return calculate(ReadOnlyByteView{ cast<const uint8_t*>(data.data()),
                                       cast<const uint8_t*>(data.data() + data.size()) });
}
}    // namespace ARLib
#ifdef COMPILER_GCC
    #pragma GCC diagnostic pop
#elif COMPILER_CLANG
    #pragma clang diagnostic pop
#endif