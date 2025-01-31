#pragma once
#include <iomanip>
#include <iostream>
#include "EvaluationValue.h"
#include "OpCode.h"

// Disassembler - дизассемблер байткода для отладки и отображения
class Disassembler {
public:
    explicit Disassembler(const std::shared_ptr<Global>& global) : global(global) {}

    void disassemble(CodeObject* co) {
        std::cout << "== Disassembly of " << co->name << " ==" << std::endl;
        size_t offset = 0;
        while (offset < co->code.size()) {
            offset = disassembleInstruction(co, offset);
        }
    }

private:
    size_t disassembleInstruction(CodeObject* co, size_t offset) {
        printf("%04zu ", offset);
        uint8_t opcode = co->code[offset];
        switch (opcode) {
            case OP_HALT:
                return simpleInstruction("OP_HALT", offset);
            case OP_CONST:
                return constantInstruction("OP_CONST", co, offset);
            case OP_ADD:
                return simpleInstruction("OP_ADD", offset);
            case OP_SUB:
                return simpleInstruction("OP_SUB", offset);
            case OP_MUL:
                return simpleInstruction("OP_MUL", offset);
            case OP_DIV:
                return simpleInstruction("OP_DIV", offset);
            case OP_COMPARE:
                return byteInstruction("OP_COMPARE", co, offset);
            case OP_JUMP_IF_FALSE:
                return jumpInstruction("OP_JUMP_IF_FALSE", 1, co, offset);
            case OP_JUMP:
                return jumpInstruction("OP_JUMP", 1, co, offset);
            case OP_GET_GLOBAL:
                return globalInstruction("OP_GET_GLOBAL", co, offset);
            case OP_SET_GLOBAL:
                return globalInstruction("OP_SET_GLOBAL", co, offset);
            case OP_GET_LOCAL:
                return byteInstruction("OP_GET_LOCAL", co, offset);
            case OP_SET_LOCAL:
                return byteInstruction("OP_SET_LOCAL", co, offset);
            case OP_NIL:
                return simpleInstruction("OP_NIL", offset);
            case OP_LOGICAL_NOT:
                return simpleInstruction("OP_LOGICAL_NOT", offset);
            case OP_JUMP_IF_FALSE_OR_POP:
                return jumpInstruction("OP_JUMP_IF_FALSE_OR_POP", 1, co, offset);
            case OP_JUMP_IF_TRUE_OR_POP:
                return jumpInstruction("OP_JUMP_IF_TRUE_OR_POP", 1, co, offset);
            case OP_DUP:
                return simpleInstruction("OP_DUP", offset);
            case OP_CALL:
                return callInstruction("OP_CALL", co, offset);
            case OP_RETURN:
                return simpleInstruction("OP_RETURN", offset);
            case OP_ARRAY:
                return simpleInstruction("OP_ARRAY", offset);
            case OP_ARRAY_GET:
                return simpleInstruction("OP_ARRAY_GET", offset);
            case OP_ARRAY_SET:
                return simpleInstruction("OP_ARRAY_SET", offset);
            default:
                throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }
    }

    size_t simpleInstruction(const std::string& name, size_t offset) {
        // Простая инструкция без аргументов
        printf("%-16s\n", name.c_str());
        return offset + 1;
    }

    size_t constantInstruction(const std::string& name, CodeObject* co, size_t offset) {
        // Инструкция с одним байтом аргумента - индекс константы
        uint8_t constantIndex = co->code[offset + 1];
        printf("%-16s %4d ", name.c_str(), constantIndex);
        std::cout << "; " << evaluationValueToConstantString(co->constants[constantIndex]) << std::endl;
        return offset + 2;
    }

    size_t byteInstruction(const std::string& name, CodeObject* co, size_t offset) {
        // Инструкция с одним байтовым аргументом (например локальный индекс)
        uint8_t slot = co->code[offset + 1];
        std::string varName = "<unknown>";
        if (co->localNames.find(slot) != co->localNames.end()) {
            varName = co->localNames[slot];
        }
        printf("%-16s %4d (%s)\n", name.c_str(), slot, varName.c_str());
        return offset + 2;
    }

    size_t jumpInstruction(const std::string& name, int sign, CodeObject* co, size_t offset) {
        uint16_t jump = (co->code[offset + 1] << 8) | co->code[offset + 2];
        printf("%-16s %4zu -> %zu\n", name.c_str(), offset, offset + 3 + sign * jump);
        return offset + 3;
    }

    size_t globalInstruction(const std::string& name, CodeObject* co, size_t offset) {
        // Глобальная переменная по индексу
        uint8_t globalIndex = co->code[offset + 1];
        const std::string& globalName = global->globals[globalIndex].name;
        printf("%-16s %4d (%s)\n", name.c_str(), globalIndex, globalName.c_str());
        return offset + 2;
    }

    size_t callInstruction(const std::string& name, CodeObject* co, size_t offset) {
        // Вызов функции с числом аргументов
        uint8_t argCount = co->code[offset + 1];
        printf("%-16s %4d args\n", name.c_str(), argCount);
        return offset + 2;
    }

    std::shared_ptr<Global> global;
};
