#include "Cell.h"
#include <sstream>
#include <iomanip>
using namespace std;

Cell::Cell() {
    value = 0;
}

Cell::Cell(uint8_t value) {
    this->value = value;
}

Cell Cell::fromHex(string str) {
    value = static_cast<uint8_t>(std::stoi(str, nullptr, 16));
    return *this;
}

string Cell::toHex() const {
    int val = static_cast<int>(this->value);
    stringstream s;
    s << hex << uppercase << setfill('0') << setw(2) << val;
    return s.str();
}

char Cell::toChar() const {
    return static_cast<char>(this->value);
}

uint8_t Cell::getValue() const {
    return value;
}

uint8_t Cell::highNibble() const {
    return (value >> 4);
}

uint8_t Cell::lowNibble() const {
    return (value & 0x0F);
}

void Cell::setValue(uint8_t value) {
    this->value = value;
}

bool Cell::isZero() const {
    return value == 0;
}

void Cell::zeroTheCell() {
    value = 0;
}

Cell& Cell::operator=(const Cell &cell) {
    if (this == &cell) {
        return *this;
    }
    this->value = cell.value;
    return *this;
}

Cell& Cell::operator=(uint8_t value) {
    this->value = value;
    return *this;
}

bool Cell::operator==(const Cell &cell) const {
    return this->value == cell.value;
}

bool Cell::operator!=(const Cell &cell) const {
    return this->value != cell.value;
}

bool Cell::operator<(const Cell &cell) const {
    return this->value < cell.value;
}

bool Cell::operator<=(const Cell &cell) const {
    return this->value <= cell.value;
}

bool Cell::operator>(const Cell &cell) const {
    return this->value > cell.value;
}

bool Cell::operator>=(const Cell &cell) const {
    return this->value >= cell.value;
}
