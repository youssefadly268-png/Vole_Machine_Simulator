#pragma once
#include <cstdint>
#include <string>

class Cell {
    uint8_t value = 0;

public:
    Cell();
    Cell(uint8_t value);
    Cell(const Cell& cell) = default;
    Cell fromHex(std::string str);
    std::string toHex() const;
    char toChar() const;
    uint8_t getValue() const;
    uint8_t highNibble() const;
    uint8_t lowNibble() const;
    void setValue(uint8_t value);
    bool isZero() const;
    void zeroTheCell();
    Cell& operator=(const Cell& cell);
    Cell& operator=(uint8_t value);
    bool operator==(const Cell& cell) const;
    bool operator!=(const Cell& cell) const;
    bool operator<(const Cell& cell) const;
    bool operator>(const Cell& cell) const;
    bool operator<=(const Cell& cell) const;
    bool operator>=(const Cell& cell) const;
};
