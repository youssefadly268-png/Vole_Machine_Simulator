#include <sstream>
#include <iomanip>
#include "CPU.h"
#include "Opcodes.h"
#include <string>

// Constructor
CPU::CPU() : pc(0), halted(false) {}

// Reset CPU state
void CPU::reset() {
    pc = 0;
    halted = false;
    ir = Instructions();
    regs.clear();
}

// 1. Fetch phase
void CPU::fetch(Memory& mem) {
    if (pc < 0 || pc > MEM_CELLS - 2) {
        halted = true;
        std::stringstream ss;
        ss << "PC out of memory bounds at address: 0x"
           << std::hex << std::uppercase << pc;
        throw VoleException(ss.str());
    }

    const Cell high = mem.readCell(pc);
    const Cell low  = mem.readCell(pc + 1);

    try {
        ir = Instructions::decode(high, low);
        pc += 2;
    } catch (const VoleException& ex) {
        halted = true;
        std::stringstream ss;
        ss << ex.what() << " at PC = 0x"
           << std::hex << std::uppercase << (pc < 16 ? "0" : "") << pc
           << " (Instruction: 0x" << high.toHex() << low.toHex() << ")";
        throw VoleException(ss.str());
    }
}

// 2. Execute phase
void CPU::execute(Memory& mem) {
    if (halted) return;

    const uint8_t op = ir.getOpcode();
    const uint8_t r  = ir.getMain_Register();
    const uint8_t s  = ir.getRegister1();
    const uint8_t t  = ir.getRegister2();
    const uint8_t xy = ir.getMemoryXY();

    switch (op) {
        case OP_LOAD_MEM:   // 1RXY: R <- mem[XY]
            regs.write(r, mem.readCell(xy));
            break;
        case OP_LOAD_VAL:   // 2RXY: R <- XY
            regs.write(r, Cell(xy));
            break;
        case OP_STORE:      // 3RXY: mem[XY] <- R
            mem.writeCell(xy, regs.read(r));
            break;
        case OP_MOVE:       // 40RS: R <- S
            regs.write(s, regs.read(t));
            break;
        case OP_ADD_INT:    // 5RST: R <- S + T (two's complement)
            regs.write(r, alu.addInt(regs.read(s), regs.read(t)));
            break;
        case OP_ADD_FLOAT:  // 6RST: R <- S + T (floating point)
            regs.write(r, alu.addFloat(regs.read(s), regs.read(t)));
            break;
        case OP_OR:         // 7RST: R <- S | T
            regs.write(r, alu.bitOr(regs.read(s), regs.read(t)));
            break;
        case OP_AND:        // 8RST: R <- S & T
            regs.write(r, alu.bitAnd(regs.read(s), regs.read(t)));
            break;
        case OP_XOR:        // 9RST: R <- S ^ T
            regs.write(r, alu.bitXor(regs.read(s), regs.read(t)));
            break;
        case OP_ROTATE:     // AR0X: rotate R right X times
            regs.write(r, alu.rotateRight(regs.read(r), t));
            break;
        case OP_JUMP:       // BRXY: if R == R0, pc <- XY
            if (regs.read(r) == regs.read(0)) {
                pc = xy;
            }
            break;
        case OP_HALT:       // C000
            halted = true;
            break;
        default:
            throw VoleException("Unhandled Opcode execution: " + std::to_string(op));
    }
}

// Single step: fetch + execute
void CPU::step(Memory& mem) {
    if (halted) return;
    fetch(mem);
    execute(mem);
}

// Getters & setters
int CPU::getPC() const { return pc; }

void CPU::setPC(int newPC) {
    if (newPC < 0 || newPC > MEM_CELLS - 2) {
        throw VoleException("Cannot set PC outside memory bounds: " + std::to_string(newPC));
    }
    pc = newPC;
}

Instructions CPU::getIR() const { return ir; }
bool CPU::isHalted() const { return halted; }
Register& CPU::getRegisters() { return regs; }
const Register& CPU::getRegisters() const { return regs; }
