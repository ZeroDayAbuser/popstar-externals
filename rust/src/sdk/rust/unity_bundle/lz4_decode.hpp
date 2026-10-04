#pragma once

#include <cstdint>
#include <cstring>

namespace lz4 {

inline int decompress(const uint8_t* src, int src_size,
                      uint8_t* dst, int dst_capacity) {
    if (src_size <= 0 || dst_capacity <= 0) return -1;

    const uint8_t* const s_end = src + src_size;
    uint8_t* const       d_end = dst + dst_capacity;
    const uint8_t*       sp    = src;
    uint8_t*             dp    = dst;

    while (sp < s_end) {
        const uint8_t token = *sp++;
        int literal_len = token >> 4;
        int match_len   = (token & 0x0F) + 4;

        if (literal_len == 15) {
            for (;;) {
                if (sp >= s_end) return -1;
                const uint8_t add = *sp++;
                literal_len += add;
                if (add != 255) break;
                if (literal_len > dst_capacity) return -1;
            }
        }

        // Copy literals.
        if (literal_len) {
            if (sp + literal_len > s_end) return -1;
            if (dp + literal_len > d_end) return -1;
            std::memcpy(dp, sp, literal_len);
            sp += literal_len;
            dp += literal_len;
        }

        if (sp >= s_end) break;

        if (sp + 2 > s_end) return -1;
        const uint16_t offset = (uint16_t)sp[0] | ((uint16_t)sp[1] << 8);
        sp += 2;
        if (offset == 0) return -1;

        if (match_len - 4 == 15) {
            for (;;) {
                if (sp >= s_end) return -1;
                const uint8_t add = *sp++;
                match_len += add;
                if (add != 255) break;
                if (match_len > dst_capacity) return -1;
            }
        }

        const uint8_t* match = dp - offset;
        if (match < dst) return -1;
        if (dp + match_len > d_end) return -1;

        for (int i = 0; i < match_len; ++i) dp[i] = match[i];
        dp += match_len;
    }

    return (int)(dp - dst);
}

} // namespace lz4
