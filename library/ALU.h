#pragma once
#include "Cell.h"

using namespace std;

class ALU {
public:
    ALU() = default;

    Cell addInt(Cell a, Cell b) const;
    Cell addFloat(Cell a, Cell b) const;
    Cell bitAnd(Cell a, Cell b) const;
    Cell bitOr(Cell a, Cell b) const;
    Cell bitXor(Cell a, Cell b) const;
    Cell rotateRight(Cell a, int n) const;
};
