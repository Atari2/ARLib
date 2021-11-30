#include "Hash.h"

#include "Conversion.h"
#include "Vector.h"

namespace ARLib {
    uint32_t HashAlgorithm<HashType::CRC32>::calculate(GenericView<const uint8_t> data) {
        uint32_t crc32 = 0xFFFFFFFFu;
        for (size_t i = 0; i < data.size(); i++) {
            const uint32_t idx = data[i] ^ (crc32 & 0xFF);
            crc32 = (crc32 >> 8) ^ s_CRCTable[idx];
        }
        return ~crc32;
    }

    uint32_t HashAlgorithm<HashType::CRC32>::calculate(GenericView<const int8_t> data) {
        return calculate(
        GenericView{cast<const uint8_t*>(data.data()), cast<const uint8_t*>(data.data() + data.size())});
    }

    HashAlgorithm<HashType::MD5>::MD5Result HashAlgorithm<HashType::MD5>::calculate(GenericView<const uint8_t> data) {
        constexpr uint32_t s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                  5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                                  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
        constexpr uint32_t K[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
        uint32_t a0 = 0x67452301;
        uint32_t b0 = 0xefcdab89;
        uint32_t c0 = 0x98badcfe;
        uint32_t d0 = 0x10325476;
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
        uint8_t size_in_bits_u8[sizeof(size_t)]{0};
        size_t og_size_in_bits = (sz * 8_sz); /* % (1 << 64) */
        ARLib::memcpy(size_in_bits_u8, cast<const uint8_t*>(&og_size_in_bits), sizeof(size_t));
        for (auto val : size_in_bits_u8)
            glob.append(val);
        HARD_ASSERT(glob.size() % 64 == 0, "Message size in bits should be divisible by 512");
        // do algorithm here:
        // for each 512-bit chunk do
        constexpr size_t CHUNK_SIZE = 512;
        constexpr size_t BITS_PER_BYTE = 8;

        for (size_t chunk_num = 0; chunk_num < glob.size(); chunk_num += CHUNK_SIZE / BITS_PER_BYTE) {
            uint32_t words[16]{0};
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

    HashAlgorithm<HashType::MD5>::MD5Result HashAlgorithm<HashType::MD5>::calculate(GenericView<const int8_t> data) {
        return calculate(
        GenericView{cast<const uint8_t*>(data.data()), cast<const uint8_t*>(data.data() + data.size())});
    }

} // namespace ARLib