#include "Machine.h"
#include "Vole_Exception.h"
#include <string>

using namespace std;

namespace
{
    const int MAX_INSTRUCTIONS_PER_RUN = 1000000;
}

Machine::Machine() : programLoaded(false)
{
}

void Machine::setPc(int value) {
    cpu.setPC(value);
}

void Machine::loadProgram(const string& fileName, int startAddress)
{
    // By default, the load address and the first instruction address are equal.
    loadProgram(fileName, startAddress, startAddress);
}

void Machine::loadProgram(const string& fileName, int loadAddress, int entryPoint)
{
    // Validate entry point and load address
    if (loadAddress < 0 || loadAddress > 254)
    {
        throw VoleException("Program load address must be between 00 and FE");
    }
    if (entryPoint < 0 || entryPoint > 254)
    {
        throw VoleException("Program entry address must be between 00 and FE");
    }

    // Load into a temporary memory first. If loading fails, current machine state is unchanged.
    Memory newMemory;
    bool success = loader.load(fileName, newMemory, loadAddress);
    if (!success)
    {
        throw VoleException("Failed to load program from file: " + fileName);
    }

    memory = newMemory;
    cpu.reset();
    cpu.setPC(entryPoint);
    programLoaded = true;
}

void Machine::runToHalt()
{
    if (!programLoaded)
    {
        throw VoleException("No program has been loaded");
    }

    int executedInstructions = 0;

    while (!cpu.isHalted())
    {
        if (executedInstructions >= MAX_INSTRUCTIONS_PER_RUN)
        {
            throw VoleException("Execution stopped: instruction limit reached (possible infinite loop)");
        }

        cpu.step(memory);
        executedInstructions++;
    }
}

void Machine::stepOnce()
{
    if (!programLoaded)
    {
        throw VoleException("No program has been loaded");
    }

    if (!cpu.isHalted())
    {
        cpu.step(memory);
    }
}

void Machine::reset()
{
    memory.clear();
    cpu.reset();
    programLoaded = false;
}

void Machine::editRegister(int index, const Cell& value)
{
    if (index < 0 || index >= 16)
    {
        throw VoleException("Invalid register index: " + to_string(index) + " (must be 0 to 15)");
    }

    cpu.getRegisters().write(index, value);
}

void Machine::editMemory(int address, const Cell& value)
{
    if (address < 0 || address >= 256)
    {
        throw VoleException("Invalid memory address: " + to_string(address) + " (must be 0 to 255)");
    }

    memory.writeCell(address, value);
}

CPU& Machine::getCPU()
{
    return cpu;
}

const CPU& Machine::getCPU() const
{
    return cpu;
}

Memory& Machine::getMemory()
{
    return memory;
}

const Memory& Machine::getMemory() const
{
    return memory;
}

bool Machine::isProgramLoaded() const
{
    return programLoaded;
}
