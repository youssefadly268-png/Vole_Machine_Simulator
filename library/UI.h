#pragma once
#include <string>
#include "Cell.h"

using namespace std;

class Machine;

class ConsoleUI {
public:
    ConsoleUI() = default;

    void run(Machine& m);

private:
    int showMenu();
    void renderState(const Machine& m);
    Cell promptHex(const string& msg);
    int promptInt(const string& msg);
    void pcInput(Machine& machine);
};

// Typedef alias so both UI and ConsoleUI can be used interchangeably
using UI = ConsoleUI;
