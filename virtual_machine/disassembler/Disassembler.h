#pragma once
#include <iostream>
#include <string>
#include "../OpCode.h"
#include "../EvaluationValue.h"

class Disassembler {
public:
    explicit Disassembler(const std::shared_ptr<Global> &global) : global(global) {}

    void disassemble(CodeObject* co) {
        std::cout << "== disassemble: " << co->name << " ==\n";
        size_t i = 0;
        while (i < co->code.size()) {
            i = disassembleInstruction(co, i);
        }
    }

private:
    std::shared_ptr<Global> global;

    size_t disassembleInstruction(CodeObject* co, size_t offset) {
        std::cout << offset << " ";
        uint8_t instruction = co->code[offset];
        switch (instruction) {
            case OP_CONST: {
                uint8_t constIndex = co->code[offset+1];
                std::cout << "OP_CONST " << evaValueToConstantString(co->constants[constIndex]) << "\n";
                return offset + 2;
            }
            case OP_ADD: std::cout << "OP_ADD\n"; return offset+1;
            case OP_SUB: std::cout << "OP_SUB\n"; return offset+1;
            case OP_MUL: std::cout << "OP_MUL\n"; return offset+1;
            case OP_DIV: std::cout << "OP_DIV\n"; return offset+1;
            case OP_COMPARE: {
                uint8_t cmpOp = co->code[offset+1];
                std::cout << "OP_COMPARE " << (int)cmpOp << "\n";
                return offset+2;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t addr = (co->code[offset+1] << 8) | co->code[offset+2];
                std::cout << "OP_JUMP_IF_FALSE " << addr << "\n";
                return offset+3;
            }
            case OP_JUMP: {
                uint16_t addr = (co->code[offset+1] << 8) | co->code[offset+2];
                std::cout << "OP_JUMP " << addr << "\n";
                return offset+3;
            }
            case OP_GET_GLOBAL: {
                uint8_t idx = co->code[offset+1];
                std::cout << "OP_GET_GLOBAL " << (int)idx << "\n";
                return offset+2;
            }
            case OP_SET_GLOBAL: {
                uint8_t idx = co->code[offset+1];
                std::cout << "OP_SET_GLOBAL " << (int)idx << "\n";
                return offset+2;
            }
            case OP_NIL: std::cout << "OP_NIL\n"; return offset+1;
            case OP_GET_LOCAL: {
                uint8_t slot = co->code[offset+1];
                std::cout << "OP_GET_LOCAL " << (int)slot << "\n";
                return offset+2;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = co->code[offset+1];
                std::cout << "OP_SET_LOCAL " << (int)slot << "\n";
                return offset+2;
            }
            case OP_LOGICAL_NOT: std::cout << "OP_LOGICAL_NOT\n"; return offset+1;
            case OP_JUMP_IF_FALSE_OR_POP: {
                uint16_t addr = (co->code[offset+1] << 8) | co->code[offset+2];
                std::cout << "OP_JUMP_IF_FALSE_OR_POP " << addr << "\n";
                return offset+3;
            }
            case OP_JUMP_IF_TRUE_OR_POP: {
                uint16_t addr = (co->code[offset+1] << 8) | co->code[offset+2];
                std::cout << "OP_JUMP_IF_TRUE_OR_POP " << addr << "\n";
                return offset+3;
            }
            case OP_DUP: std::cout << "OP_DUP\n"; return offset+1;
            case OP_HALT: std::cout << "OP_HALT\n"; return offset+1;

            case OP_CALL: {
                uint8_t funcIndex = co->code[offset+1];
                uint8_t argCount = co->code[offset+2];
                std::cout << "OP_CALL funcIndex:" << (int)funcIndex << " argCount:" << (int)argCount << "\n";
                return offset+3;
            }
            case OP_RETURN: {
                std::cout << "OP_RETURN\n";
                return offset+1;
            }

            default:
                std::cout << "Unknown opcode: " << (int)instruction << "\n";
                return offset+1;
        }
    }
};
