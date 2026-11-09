#pragma once
#include "Cell.h"

class Register {
    Cell cells[16] = {Cell()};
    public:
    Cell read(int index) const;
    void write(int index,const Cell& cell);
    void clear();
    Cell& operator[](int index);
};