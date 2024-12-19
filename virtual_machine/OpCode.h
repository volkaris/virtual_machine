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
constexpr auto OP_SET_GLOBAL = 0x0A;

constexpr auto OP_GET_LOCAL = 0x0B;
constexpr auto OP_SET_LOCAL = 0x0C;

constexpr auto OP_LOGICAL_NOT = 0X0D;


constexpr auto OP_JUMP_IF_FALSE_OR_POP = 0X0E;
constexpr auto OP_JUMP_IF_TRUE_OR_POP = 0X0F;

constexpr auto OP_DUP= 0X10;

constexpr auto OP_NIL = 0x11;

constexpr auto OP_CALL   = 0x12;
constexpr auto OP_RETURN = 0x13;

constexpr auto OP_ARRAY = 0x14;
constexpr auto OP_ARRAY_GET = 0x15;
constexpr auto OP_ARRAY_SET = 0x16;
constexpr auto OP_PRINT = 0x17;

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
        case OP_LOGICAL_NOT: return "LOGICAL_NOT";
        case OP_JUMP_IF_FALSE_OR_POP: return "JUMP_IF_FALSE_OR_POP";
        case OP_JUMP_IF_TRUE_OR_POP: return "JUMP_IF_TRUE_OR_POP";
        case OP_DUP: return "DUP";
        case OP_CALL:   return "CALL";
        case OP_RETURN: return "RETURN";
        case OP_ARRAY: return "ARRAY";
        case OP_ARRAY_GET: return "ARRAY_GET";
        case OP_ARRAY_SET: return "ARRAY_SET";
        case OP_PRINT : return "OP_PRINT";
        default:
            throw std::runtime_error("Неизвестный байткод " + std::to_string(opcode));
    }
}
