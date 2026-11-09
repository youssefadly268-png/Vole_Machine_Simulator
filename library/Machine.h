#pragma once

#include <string>
#include "CPU.h"
#include "Memory.h"
#include "Loader.h"
#include "Cell.h"

using namespace std;

class Machine {
private:
    CPU cpu = CPU();
    Memory memory = Memory();
    Loader loader;
    bool programLoaded = false;

public:
    Machine();

    void loadProgram(const string& fileName, int startAddress = 0);
    void loadProgram(const string& fileName, int loadAddress, int entryPoint);
    void runToHalt();
    void stepOnce();
    void reset();

    void editRegister(int index, const Cell& value);
    void editMemory(int address, const Cell& value);

    CPU& getCPU();
    const CPU& getCPU() const;
    Memory& getMemory();
    const Memory& getMemory() const;

    void setPc(int value);

    bool isProgramLoaded() const;
};

// The project description also refers to this class as VoleMachine.
using VoleMachine = Machine;
