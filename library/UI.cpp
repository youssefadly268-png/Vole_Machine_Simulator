#include "UI.h"
#include "Machine.h"
#include "CPU.h"
#include "Memory.h"
#include "Register.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <limits>
#include <cctype>

using namespace std;

int ConsoleUI::promptInt(const string& msg) {
    int val;
    while (true) {
        cout << msg;
        if (cin >> val) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        }
        cout << "Invalid input. Please enter an integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void ConsoleUI::pcInput(Machine& machine) {
    int pc;
    bool valid = false;
    while (!valid) {
        pc = promptInt("Enter [PC] address (0 - 254): \n");
        if (pc < 0 || pc > 254) continue;
        try {
            machine.setPc(pc);
            valid = true;
        } catch (const exception& ex) {
            cout << "Error Input PC: " << ex.what() << "\n";
        }
    }
}

Cell ConsoleUI::promptHex(const string& msg) {
    string input;
    while (true) {
        cout << msg;
        if (cin >> input) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            // Strip optional 0x or 0X prefix
            if (input.size() >= 2 && (input[0] == '0' && (input[1] == 'x' || input[1] == 'X'))) {
                input = input.substr(2);
            }

            // Pad single hex digit with leading zero (e.g. 'F' -> '0F')
            if (input.size() == 1) {
                input = "0" + input;
            }

            // Check if valid 2-digit hex string
            bool valid = (input.size() == 2);
            if (valid) {
                for (char c : input) {
                    if (!isxdigit(static_cast<unsigned char>(c))) {
                        valid = false;
                        break;
                    }
                }
            }

            if (valid) {
                try {
                    Cell cell;
                    cell.fromHex(input);
                    return cell;
                } catch (...) {
                    // Fall through to error prompt
                }
            }
        } else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        cout << "Invalid hexadecimal byte. Please enter a 1 or 2 digit hex value (00 - FF).\n";
    }
}

int ConsoleUI::showMenu() {
    cout << "\n========================================\n";
    cout << "           VOLE MACHINE SIMULATOR       \n";
    cout << "========================================\n";
    cout << " 1. Load Program from File\n";
    cout << " 2. Run Program to Halt\n";
    cout << " 3. Step Instruction(s)\n";
    cout << " 4. Display Machine State (CPU & Memory)\n";
    cout << " 5. Edit Register\n";
    cout << " 6. Edit Memory Cell\n";
    cout << " 7. Edit PC\n";
    cout << " 8. Reset Machine\n";
    cout << " 0. Exit\n";
    cout << "========================================\n";
    return promptInt("Choose an option: ");
}

void ConsoleUI::renderState(const Machine& m) {
    Machine& nonConstM = const_cast<Machine&>(m);
    CPU& cpu = const_cast<CPU&>(nonConstM.getCPU());
    Memory& mem = const_cast<Memory&>(nonConstM.getMemory());
    Register& regs = cpu.getRegisters();

    cout << "\n================ MACHINE STATE ================\n";

    // CPU State
    cout << "[CPU]\n";
    cout << "  PC     : 0x" << hex << uppercase << setfill('0')
         << setw(2) << cpu.getPC() << dec << "\n";
    cout << "  Status : " << (cpu.isHalted() ? "HALTED" : "RUNNING") << "\n";

    // Registers (R0 .. RF)
    cout << "\n[REGISTERS]\n";
    for (int i = 0; i < 16; ++i) {
        cout << "  R" << hex << uppercase << i << ": "
             << regs.read(i).toHex();
        if ((i + 1) % 4 == 0) {
            cout << "\n";
        } else {
            cout << "    ";
        }
    }
    cout << dec;

    // Screen Output (Memory address 0xFF and 0x00)
    cout << "\n[SCREEN OUTPUT]\n";
    Cell screenCellFF = mem.readCell(255);
    Cell screenCell00 = mem.readCell(0);
    uint8_t valFF = screenCellFF.getValue();
    uint8_t val00 = screenCell00.getValue();
    cout << "  Mem[0xFF]: 0x" << screenCellFF.toHex() << " (" << static_cast<int>(valFF) << ")";
    if (valFF >= 32 && valFF <= 126) cout << " ASCII: '" << static_cast<char>(valFF) << "'";
    cout << "\n";
    if (val00 != 0) {
        cout << "  Mem[0x00]: 0x" << screenCell00.toHex() << " (" << static_cast<int>(val00) << ")";
        if (val00 >= 32 && val00 <= 126) cout << " ASCII: '" << static_cast<char>(val00) << "'";
        cout << "\n";
    }

    // Memory (256 bytes formatted in 16x16 grid)
    cout << "\n[MEMORY (256 Bytes)]\n";
    cout << "      ";
    for (int col = 0; col < 16; ++col) {
        cout << hex << uppercase << setfill('0') << setw(2) << col << " ";
    }
    cout << "\n";

    for (int row = 0; row < 256; row += 16) {
        cout << "  " << hex << uppercase << setfill('0') << setw(2) << row << ": ";
        for (int col = 0; col < 16; ++col) {
            int addr = row + col;
            cout << mem.readCell(addr).toHex() << " ";
        }
        cout << "\n";
    }
    cout << dec;
    cout << "===============================================\n\n";
}

void ConsoleUI::run(Machine& m) {
    cout << "========================================\n";
    cout << "   Welcome to the Vole Machine Simulator\n";
    cout << "========================================\n";

    bool running = true;
    while (running) {
        int choice = showMenu();
        switch (choice) {
            case 1: { // Load Program from file
                m.reset();
                cout << "Enter program file path: ";
                string path;
                getline(cin, path);
                if (path.empty()) {
                    cin >> path;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                int start = promptInt("Enter starting memory address (0 - 255): ");
                int pc = promptInt("Enter program counter (0 - 254): ");
                try {
                    m.loadProgram(path, start, pc);
                    cout << "Program loaded successfully from: " << path << "\n";
                    renderState(m);
                } catch (const exception& ex) {
                    cout << "Error loading program: " << ex.what() << "\n";
                }
                break;
            }
            case 2: { // Run until halt
                try {
                    m.runToHalt();
                    cout << "Execution completed.\n";
                    renderState(m);
                } catch (const exception& ex) {
                    cout << "Execution error: " << ex.what() << "\n";
                }
                break;
            }
            case 3: { // Step instruction(s)
                int steps = promptInt("Enter number of instructions to step (default 1): ");
                if (steps <= 0) steps = 1;
                try {
                    for (int s = 0; s < steps; ++s) {
                        m.stepOnce();
                        if (m.getCPU().isHalted()) {
                            cout << "CPU halted during step " << (s + 1) << ".\n";
                            break;
                        }
                    }
                    renderState(m);
                } catch (const exception& ex) {
                    cout << "Execution error: " << ex.what() << "\n";
                }
                break;
            }
            case 4: { // Display state
                renderState(m);
                break;
            }
            case 5: { // Edit Register
                int regIdx = promptInt("Enter register index (0 to 15): ");
                if (regIdx < 0 || regIdx > 15) {
                    cout << "Invalid register index! Must be between 0 and 15.\n";
                    break;
                }
                Cell val = promptHex("Enter hex byte value (e.g. 1F): ");
                m.editRegister(regIdx, val);
                cout << "Register R" << hex << uppercase << regIdx
                     << " updated to " << val.toHex() << dec << ".\n";
                break;
            }
            case 6: { // Edit Memory Cell
                int addr = promptInt("Enter memory address (0 to 255): ");
                if (addr < 0 || addr > 255) {
                    cout << "Invalid memory address! Must be between 0 and 255.\n";
                    break;
                }
                Cell val = promptHex("Enter hex byte value (e.g. 1F): ");
                m.editMemory(addr, val);
                cout << "Memory address 0x" << hex << uppercase << setfill('0')
                     << setw(2) << addr << " updated to " << val.toHex() << dec << ".\n";
                break;
            }
            case 7: { // Edit PC
                pcInput(m);
                break;
            }
            case 8: { // Reset Machine
                m.reset();
                cout << "Machine has been reset.\n";
                renderState(m);
                break;
            }
            case 0: { // Exit
                running = false;
                cout << "Exiting Vole Simulator. Goodbye!\n";
                break;
            }
            default: {
                cout << "Invalid choice! Please choose an option from the menu.\n";
                break;
            }
        }
    }
}
