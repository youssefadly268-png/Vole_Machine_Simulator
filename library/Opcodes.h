#pragma once
#include <cstdint>

const int MEM_CELLS = 256;
const uint8_t SCREEN_ADDRESS = 0xFF;

const uint8_t OP_LOAD_MEM   = 0x1;
const uint8_t OP_LOAD_VAL   = 0x2;
const uint8_t OP_STORE      = 0x3;
const uint8_t OP_MOVE       = 0x4;
const uint8_t OP_ADD_INT    = 0x5;
const uint8_t OP_ADD_FLOAT  = 0x6;
const uint8_t OP_OR         = 0x7;
const uint8_t OP_AND        = 0x8;
const uint8_t OP_XOR        = 0x9;
const uint8_t OP_ROTATE     = 0xA;
const uint8_t OP_JUMP       = 0xB;
const uint8_t OP_HALT       = 0xC;
