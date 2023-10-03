#include <stdint.h>

#include "common_audio/signal_processing/include/spl_inl.h"

// Table used by WebRtcSpl_CountLeadingZeros32_NotBuiltin. For each uint32_t n
// that's a sequence of 0 bits followed by a sequence of 1 bits, the entry at
// index (n * 0x8c0b2891) >> 26 in this table gives the number of zero bits in
// n.
const int8_t kWebRtcSpl_CountLeadingZeros32_Table[64] = {
    32, 8,  17, -1, -1, 14, -1, -1, -1, 20, -1, -1, -1, 28, -1, 18,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  26, 25, 24,
    4,  11, 23, 31, 3,  7,  10, 16, 22, 30, -1, -1, 2,  6,  13, 9,
    -1, 15, -1, 21, -1, 29, 19, -1, -1, -1, -1, -1, 1,  27, 5,  12,
};