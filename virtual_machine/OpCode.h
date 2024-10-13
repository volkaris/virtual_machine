#pragma once

#include <cstdint>
#include "Logger.h"

constexpr auto OP_HALT = 0x00;
constexpr auto OP_CONST = 0x01;
constexpr auto OP_ADD = 0x02;
constexpr auto OP_SUB = 0x03;
constexpr auto OP_MUL = 0x04;
constexpr auto OP_DIV = 0x05;
constexpr auto OP_COMPARE = 0x06;


constexpr auto OP_JUMP_IF_FALSE = 0x07;
constexpr auto OP_JUMP = 0x08;

constexpr auto OP_GET_GLOBAL = 0x09;
constexpr auto OP_SET_GLOBAL = 0x10;

// i love kostili!
constexpr auto OP_NIL = 0x20;



#define OP_STR(op) \
    case OP_##op:  \
        return #op

std::string opcodeToString(uint8_t opcode) {
    switch (opcode) {
        OP_STR(HALT);
        OP_STR(CONST);
        OP_STR(ADD);
        OP_STR(SUB);
        OP_STR(MUL);
        OP_STR(DIV);
        OP_STR(COMPARE);
        OP_STR(JUMP_IF_FALSE);
        OP_STR(JUMP);
        OP_STR(GET_GLOBAL);
        OP_STR(SET_GLOBAL);
        default:
           DIE << "opcodeToString: unknown opcode: " << opcode;
    }
    return "Unknown";
}




