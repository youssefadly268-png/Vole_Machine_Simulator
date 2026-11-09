#pragma once
#include "Cell.h"

class Memory {
    Cell cells[256] = {Cell()};
    std::string screen = "";
    public:
    Cell readCell(int index) const;
    void writeCell(int index,const Cell& cell);
    void clearScreen();
    std::string getScreen();
    void clear();
    Cell& operator[](int index);
};
