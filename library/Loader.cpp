#include "Loader.h"
#include <fstream>
#include <stdexcept>
#include <cctype>

using namespace std;
bool Loader::isLineEmpty(const string& line) {
    if (line.empty()) return true;
    for (char c : line) {
        if (!isspace(static_cast<unsigned char>(c))) return false;
    }
    return true;
}
bool Loader::load(const string& fileName,Memory& memory,int startAddress)
{
    string line;
    string pcode;
    ifstream file(fileName);
    if (!file.is_open())
    {
        throw runtime_error("Cannot open program file");
    }
    if (startAddress < 0 || startAddress >= 256)
    {
        throw out_of_range("Invalid starting address");
    }
    int address = startAddress;
    while (getline(file,line)) {
        if (isLineEmpty(line) || line[0] == ';' ) {
            continue;
        }
        for (char c : line) {
            if (c == ' ')continue;
            if (c == ';')break;
            if ((c >= '0' && c <= '9') || ( c >= 'a' && c <= 'f') || ( c >= 'A' && c <= 'F'))pcode += c;
        }
    }
    for ( size_t i = 0; i + 1 < pcode.length(); i+=2 ) {
        if (address >= 256)
        {
            throw out_of_range("Program exceeds memory capacity");
        }
        Cell temp;
        temp.fromHex(pcode.substr(i, 2));
        memory[address] = temp;
        address++;
    }
    return true;
}