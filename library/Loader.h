#pragma once

#include <string>
#include "Memory.h"

class Loader
{
    bool isLineEmpty(const std::string& line);
public:
    bool load(const std::string& fileName,Memory& memory,int startAddress);
};