#include <sstream>
#include <iomanip>
#include "Instructions.h"
#include <cstdint>
#include <string>

namespace {
constexpr uint8_t OP_MIN = 0x1;
constexpr uint8_t OP_STORE = 0x3;
constexpr uint8_t OP_MOVE = 0x4;
constexpr uint8_t OP_ROTATE = 0xA;
constexpr uint8_t OP_HALT = 0xC;

// Screen address for STORE (3R00 in the standard Vole spec).
// Change to 0xFF if your assignment defines it differently.
constexpr uint8_t SCREEN_ADDRESS = 0xFF;
}

// Default constructor
Instructions::Instructions()
    : opcode(0), Main_Register(0), Register1(0), Register2(0), MemoryXY(0) {}

// Parameterized constructor
Instructions::Instructions(uint8_t opcode, uint8_t Main_Register, uint8_t Register1,
                           uint8_t Register2, uint8_t MemoryXY)
    : opcode(opcode), Main_Register(Main_Register), Register1(Register1),
      Register2(Register2), MemoryXY(MemoryXY) {}

Instructions Instructions::decode(Cell high, Cell low) {
    const uint8_t opcode = high.highNibble();

    // Fail fast before doing any other work
    if (opcode < OP_MIN || opcode > OP_HALT) {
        std::stringstream ss;
        ss << "Invalid Opcode: Must be between 0x1 and 0xC (1 to 12). Found: 0x"
           << std::hex << std::uppercase << static_cast<int>(opcode)
           << " (decimal " << std::dec << static_cast<int>(opcode) << ")";
        throw VoleException(ss.str());
    }

    const uint8_t reg1 = low.highNibble();
    const uint8_t reg2 = low.lowNibble();

    const uint8_t memoryXY = low.getValue();

    Instructions inst(opcode, high.lowNibble(), reg1, reg2, memoryXY);

    if (!inst.isValid()) {
        throw VoleException("Invalid Instruction format or operands for Opcode " +
                            std::to_string(opcode));
    }

    return inst;
}

// Getters
uint8_t Instructions::getOpcode() const { return opcode; }
uint8_t Instructions::getMain_Register() const { return Main_Register; }
uint8_t Instructions::getRegister1() const { return Register1; }
uint8_t Instructions::getRegister2() const { return Register2; }
uint8_t Instructions::getMemoryXY() const { return MemoryXY; }

bool Instructions::isScreenStore() const {
    return opcode == OP_STORE && (MemoryXY == SCREEN_ADDRESS || MemoryXY == 0x00);
}

bool Instructions::isValid() const {
    // Nibbles are always 0-15, so no range check is needed.
    switch (opcode) {
        case OP_MOVE:   // 40RS
            return Main_Register == 0;
        case OP_ROTATE: // AR0X
            return Register1 == 0;
        case OP_HALT:   // C000
            return Main_Register == 0 && MemoryXY == 0x00;
        default:
            return true;
    }
}