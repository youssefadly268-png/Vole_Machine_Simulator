#include "ALU.h"
#include <algorithm>

using namespace std;

Cell ALU::addInt(Cell a, Cell b) const {
    uint8_t result = static_cast<uint8_t>(a.getValue() + b.getValue());
    return Cell(result);
}

Cell ALU::bitAnd(Cell a, Cell b) const {
    uint8_t result = static_cast<uint8_t>(a.getValue() & b.getValue());
    return Cell(result);
}

Cell ALU::bitOr(Cell a, Cell b) const {
    uint8_t result = static_cast<uint8_t>(a.getValue() | b.getValue());
    return Cell(result);
}

Cell ALU::bitXor(Cell a, Cell b) const {
    uint8_t result = static_cast<uint8_t>(a.getValue() ^ b.getValue());
    return Cell(result);
}

Cell ALU::rotateRight(Cell a, int n) const {
    n = (n % 8 + 8) % 8;
    if (n == 0) {
        return a;
    }
    uint8_t val = a.getValue();
    uint8_t result = static_cast<uint8_t>((val >> n) | ((val << (8 - n)) & 0xFF));
    return Cell(result);
}

Cell ALU::addFloat(Cell a, Cell b) const {
    // Brookshear 8-bit floating point format:
    // Bit 7: Sign (0 = positive, 1 = negative)
    // Bits 6-4: Exponent (3 bits in excess-4 notation)
    // Bits 3-0: Mantissa (4 bits, fraction 0.m1 m2 m3 m4)
    // Value = (-1)^s * (m / 16) * 2^(exp - 4)

    int mantA = a.getValue() & 0x0F;
    int mantB = b.getValue() & 0x0F;

    // A zero mantissa represents 0.0 in Brookshear representation
    if (mantA == 0 && mantB == 0) {
        return Cell(0);
    }
    if (mantA == 0) {
        return b;
    }
    if (mantB == 0) {
        return a;
    }

    int signA = (a.getValue() >> 7) & 0x01;
    int expA  = (a.getValue() >> 4) & 0x07;

    int signB = (b.getValue() >> 7) & 0x01;
    int expB  = (b.getValue() >> 4) & 0x07;

    // Ensure A has the greater or equal exponent
    if (expA < expB) {
        swap(signA, signB);
        swap(expA, expB);
        swap(mantA, mantB);
    }

    int diff = expA - expB;
    int commonExp = expA;

    // Shift mantissas into 16-bit space with headroom for alignment & carry
    int sigA = mantA << 8;
    int sigB = (diff >= 12) ? 0 : (mantB << 8) >> diff;

    int resSign = signA;
    int resSig = 0;

    if (signA == signB) {
        resSig = sigA + sigB;
    } else {
        if (sigA >= sigB) {
            resSig = sigA - sigB;
            resSign = signA;
        } else {
            resSig = sigB - sigA;
            resSign = signB;
        }
    }

    if (resSig == 0) {
        return Cell(0);
    }

    // Carry out: bit 12 set (>= 0x1000)
    while (resSig >= 0x1000) {
        resSig >>= 1;
        commonExp++;
    }

    // Exponent overflow check
    if (commonExp > 7) {
        // Exponent overflow: clamp to max magnitude with sign
        uint8_t overflowVal = static_cast<uint8_t>((resSign << 7) | (0x07 << 4) | 0x0F);
        return Cell(overflowVal);
    }

    // Normalize: leading bit of mantissa (bit 11) must be 1
    while (resSig < 0x800 && commonExp > 0) {
        resSig <<= 1;
        commonExp--;
    }

    // Underflow check: if cannot normalize within exponent range
    if (resSig < 0x800) {
        return Cell(0);
    }

    // Extract the top 4 bits of the normalized mantissa
    int finalMant = (resSig >> 8) & 0x0F;

    uint8_t resByte = static_cast<uint8_t>((resSign << 7) | ((commonExp & 0x07) << 4) | (finalMant & 0x0F));
    return Cell(resByte);
}
