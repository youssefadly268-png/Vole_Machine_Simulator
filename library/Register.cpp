#include "Register.h"
#include <stdexcept>
using namespace std;

Cell Register::read(int index) const {
    if (index >= 16 || index < 0) {
        throw out_of_range("Index out of range");
    }
    return this->cells[index];
}

void Register::write(int index, const Cell& cell) {
    if (index >= 16 || index < 0) {
        throw out_of_range("Index out of range");
    }
    this->cells[index] = cell;
}

void Register::clear() {
    for (int i = 0; i < 16; i++) {
        this->cells[i].zeroTheCell();
    }
}

Cell& Register::operator[](int index) {
    if (index >= 16 || index < 0) {
        throw out_of_range("Index out of range");
    }
    return this->cells[index];
}
