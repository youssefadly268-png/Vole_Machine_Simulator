#pragma once
#include <iostream>
#include "Instructions.h"
#include "Register.h"
#include "Memory.h"
#include "ALU.h"
#include "Vole_Exception.h"

class CPU {
    int pc = 0;
    Instructions ir = Instructions();
    bool halted = false;
    Register regs = Register();
    ALU alu = ALU();

public:
    CPU();
    void reset();
    void fetch(Memory& mem);
    void execute(Memory& mem);
    void step(Memory& mem);

    int getPC() const;
    void setPC(int newPC);
    Instructions getIR() const;
    bool isHalted() const;
    Register& getRegisters();
    const Register& getRegisters() const;
};
