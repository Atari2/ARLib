#include "Hash.h"

#include "Conversion.h"

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
        return calculate(GenericView{cast<const uint8_t*>(data.data()), cast<const uint8_t*>(data.data() + data.size())});
    }
} // namespace ARLib