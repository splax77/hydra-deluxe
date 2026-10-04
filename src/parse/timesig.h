// A time signature's denominator is stored as a power of two in both chart
// formats. One rule turns the exponent into the denominator, so neither
// parser shifts by an out-of-range amount.
#ifndef HYDRA_PARSE_TIMESIG_H
#define HYDRA_PARSE_TIMESIG_H

namespace hydra {

// 2 to the power `exponent`, or 0 when the exponent is below 0 or above 30
// (no int holds that power). The readers only know a line's exponent, not
// always its tick, so they store the 0 and the song parser's apply_timesig
// refuses it there, naming the tick. A signature whose top number is 0 is
// ignored before that, whatever its bottom number says.
inline int timesig_denominator(int exponent) {
    if (exponent < 0 || exponent > 30) return 0;
    return 1 << exponent;
}

}  // namespace hydra

#endif
