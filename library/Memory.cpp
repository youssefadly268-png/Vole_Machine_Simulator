#include "Memory.h"

#include <stdexcept>
using namespace std;

Cell Memory::readCell(int index) const {
    if (index > 255 || index < 0) {
        throw out_of_range("Invalid index");
    }
    return cells[index];
}
void Memory::writeCell(int index, const Cell& cell) {
    if (index > 255 || index < 0) {
        throw out_of_range("Invalid index");
    }
    cells[index] = cell;
}

void Memory::clearScreen() {
   this->cells[255] = 0;
    this->cells[0] = 0;
   this-> screen.clear();
}

std::string Memory::getScreen() {
    screen = cells[255].toHex();
    return this->screen;
}

void Memory::clear() {
    for (int i = 0; i < 256; i++) {
        cells[i].zeroTheCell();
    }
    this->clearScreen();
}
Cell& Memory::operator[](int index) {
    if (index > 255 || index < 0) {
        throw out_of_range("Invalid index");
    }
    return cells[index];
}