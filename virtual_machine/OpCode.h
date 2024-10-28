#pragma once

#include <cstdint>

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

constexpr auto OP_GET_LOCAL = 0x11;
constexpr auto OP_SET_LOCAL = 0x12;

constexpr auto OP_NIL = 0x20;


inline std::string opcodeToString(uint8_t opcode) {
    switch (opcode) {
        case OP_HALT: return "HALT";
        case OP_CONST: return "CONST";
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_MUL: return "MUL";
        case OP_DIV: return "DIV";
        case OP_COMPARE: return "COMPARE";
        case OP_JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OP_JUMP: return "JUMP";
        case OP_GET_GLOBAL: return "GET_GLOBAL";
        case OP_SET_GLOBAL: return "SET_GLOBAL";
        case OP_NIL: return "NIL";
        case OP_GET_LOCAL: return "GET_LOCAL";
        case OP_SET_LOCAL: return "SET_LOCAL";
        default:
           throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }
    return "Unknown";
}




