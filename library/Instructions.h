#pragma once
#include <string>
#include <cstdint>
#include "Cell.h"
#include "Vole_Exception.h"

class Instructions {
private:
    uint8_t opcode = 0;
    uint8_t Main_Register = 0;
    uint8_t Register1 = 0;
    uint8_t Register2 = 0;
    uint8_t MemoryXY = 0;

public:
    Instructions();
    Instructions(uint8_t opcode, uint8_t Main_Register, uint8_t Register1, uint8_t Register2, uint8_t MemoryXY);
    
    static Instructions decode(Cell high, Cell low);

    uint8_t getOpcode() const;
    uint8_t getMain_Register() const;
    uint8_t getRegister1() const;
    uint8_t getRegister2() const;
    uint8_t getMemoryXY() const;

    bool isValid() const;
    bool isScreenStore() const; // For opcode 3 with FF to print to screen
};