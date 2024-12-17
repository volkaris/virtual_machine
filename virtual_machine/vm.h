#pragma once
#include <vector>
#include <string>
#include "OpCode.h"
#include <array>
#include "parser.h"
using namespace syntax;

#define STACK_LIMIT 512


#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"


struct Frame {
    CodeObject* codeObject;                          // Указатель на код функции
    size_t ip;                                       // Индекс текущей инструкции
    std::vector<EvaluationValue> locals;             // Локальные переменные
    uint8_t* returnAddress;                          // Адрес возврата
};


class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        setGlobalVariables();
        locals.resize(1024);
    }

    EvaluationValue exec(const std::string &program) {
        std::shared_ptr<Exp> ast = _parser->parse(program);

        co = _bytecodeGenerator->compile(*ast);


        Frame mainFrame;
        mainFrame.codeObject = co;
        mainFrame.ip = 0;
        mainFrame.locals = {}; // Основные локальные переменные

        callStack.push_back(mainFrame);

        /*_bytecodeGenerator->disassembleBytecode();*/

        evalExp();
        return stack.back();
    }

    template<typename T>
    void compare_values(const T &casted_left, const T &casted_right, uint8_t compare_op) {
        bool res = false;
        switch (compare_op) {
            case 0:
                res = casted_left < casted_right;
                break;
            case 1:
                res = casted_left > casted_right;
                break;
            case 2:
                res = casted_left == casted_right;
                break;
            case 3:
                res = casted_left >= casted_right;
                break;
            case 4:
                res = casted_left <= casted_right;
                break;
            case 5:
                res = casted_left != casted_right;
                break;
            default: {
                throw std::runtime_error("Unknown compare operation." + std::to_string(compare_op));
            }
        }
        push(BOOLEAN(res));
    }

    EvaluationValue evalExp() {
        while (!callStack.empty()) {
            Frame& currentFrame = callStack.back();
            CodeObject* currentCode = currentFrame.codeObject;

            while (currentFrame.ip < currentCode->code.size()) {
                uint8_t op_code = currentCode->code[currentFrame.ip++];
                switch (op_code) {
                    case OP_HALT:
                        return pop();

                    case OP_CONST:
                        push(GET_CONST());
                    break;

                    case OP_ADD: {
                        auto right = pop();
                        auto left = pop();

                        if (IS_NUMBER(left) && IS_NUMBER(right)) {
                            push(NUMBER(AS_NUMBER(left) + AS_NUMBER(right)));
                        } else if (IS_STRING(left) && IS_STRING(right)) {
                            push(ALLOC_STRING(AS_CPP_STRING(left) + AS_CPP_STRING(right)));
                        } else {
                            throw std::runtime_error("Type error in ADD operation.");
                        }
                        break;
                    }

                    case OP_SUB: {
                        auto right = pop();
                        auto left = pop();
                        push(NUMBER(AS_NUMBER(left) - AS_NUMBER(right)));
                        break;
                    }

                    case OP_MUL: {
                        auto right = pop();
                        auto left = pop();
                        push(NUMBER(AS_NUMBER(left) * AS_NUMBER(right)));
                        break;
                    }

                    case OP_DIV: {
                        auto right = pop();
                        auto left = pop();

                        auto casted_left = AS_NUMBER(left);
                        auto casted_right = AS_NUMBER(right);
                        if (casted_right == 0) {
                            throw std::runtime_error("division by zero");
                        }
                        push(NUMBER(casted_left / casted_right));
                        break;
                    }

                    case OP_COMPARE: {
                        auto compareOp = READ_BYTE();

                        auto right = pop();
                        auto left = pop();

                        if (IS_NUMBER(left) && IS_NUMBER(right)) {
                            auto castedLeft = AS_NUMBER(left);
                            auto castedRight = AS_NUMBER(right);
                            compare_values(castedLeft, castedRight, compareOp);
                        } else if (IS_STRING(left) && IS_STRING(right)) {
                            auto castedLeft = AS_CPP_STRING(left);
                            auto castedRight = AS_CPP_STRING(right);
                            compare_values(castedLeft, castedRight, compareOp);
                        } else {
                            throw std::runtime_error("Type error in COMPARE operation.");
                        }
                        break;
                    }

                    case OP_JUMP_IF_FALSE: {
                        uint16_t addr = READ_SHORT();
                        EvaluationValue condition = pop();
                        if (IS_BOOL(condition) && !AS_BOOL(condition)) {
                            ip = &co->code[addr];
                        } /*else {
                            // Skip the jump address
                            // No action needed since ip already points to next instruction
                        }*/
                        break;
                    }

                    case OP_JUMP: {
                        uint16_t addr = READ_SHORT();
                        ip = &co->code[addr];
                        break;
                    }


                    case OP_NIL: {
                        push(NIL());
                        break;
                    }

                    case OP_GET_GLOBAL: {
                        auto globalIndex = READ_BYTE();
                        push(global->get(globalIndex).value);
                        break;
                    }

                    case OP_SET_GLOBAL: {
                        auto globalIndex = READ_BYTE();
                        global->set(globalIndex, pop());
                        break;
                    }

                    case OP_SET_LOCAL: {
                        uint8_t slot = READ_BYTE();
                        locals[slot] = pop();
                        break;
                    }

                    case OP_GET_LOCAL: {
                        uint8_t slot = READ_BYTE();
                        push(locals[slot]);
                        break;
                    }

                    case OP_LOGICAL_NOT: {
                        auto operand = pop();
                        bool result = !isTruth(operand);
                        push(BOOLEAN(result));
                        break;
                    }

                    case OP_JUMP_IF_FALSE_OR_POP: {
                        uint16_t address = READ_SHORT();

                        auto value = peek();

                        if (!isTruth(value)) {
                            pop(); // Remove the value
                            ip = &co->code[address];
                        } else {
                            pop(); // Remove the value
                        }
                        break;
                    }

                    case OP_JUMP_IF_TRUE_OR_POP: {
                        uint16_t address = READ_SHORT();
                        auto value = peek();

                        if (isTruth(value)) {
                            pop(); // Remove the value
                            ip = &co->code[address];
                        } else {
                            pop(); // Remove the value
                        }
                        break;
                    }

                    case OP_DUP: {
                        // Ensure there is at least one value on the stack to duplicate
                        if (sp == stack.begin()) {
                            throw std::runtime_error("Stack underflow: Cannot duplicate from an empty stack.");
                        }

                        EvaluationValue value = peek(); // Get the top value without removing it
                        push(value); // Push a copy onto the stack
                        break;
                    }


                    case OP_CALL: {
                        uint8_t argCount = currentCode->code[currentFrame.ip++];
                        EvaluationValue function = pop();
                        if (!IS_CODE(function)) {
                            throw std::runtime_error("Attempted to call a non-function.");
                        }
                        CodeObject* funcCo = AS_CODE(function);

                        // Проверка количества аргументов
                        if (argCount > funcCo->constants.size()) {
                            throw std::runtime_error("Incorrect number of arguments.");
                        }

                        // Создание нового фрейма для функции
                        Frame funcFrame;
                        funcFrame.codeObject = funcCo;
                        funcFrame.ip = 0;
                        funcFrame.locals.reserve(funcCo->localNames.size());

                        // Инициализация локальных переменных функцией
                        for (size_t i = 0; i < argCount; ++i) {
                            funcFrame.locals.push_back(pop());
                        }

                        callStack.push_back(funcFrame);
                        break;
                    }

                    case OP_RETURN: {
                        EvaluationValue returnValue = pop();
                        callStack.pop_back();
                        if (callStack.empty()) {
                            // Основной фрейм завершен
                            return returnValue;
                        } else {
                            // Вернуть значение в предыдущий фрейм
                            push(returnValue);
                        }
                        break;
                    }


                    default: {
                        throw std::runtime_error("Unknown opcode: " + std::to_string(op_code));
                    }
                }
            }
            callStack.pop_back();
        }
        throw std::runtime_error("No frame to execute.");
    }

private:
    void setGlobalVariables() {
        /*global->addConst("x", 10);*/
    }

    bool isTruth(const EvaluationValue &value) {
        if (IS_BOOL(value)) {
            return AS_BOOL(value);
        }

        if (IS_NIL(value)) {
            return false;
        }

        if (IS_NUMBER(value)) {
            return AS_NUMBER(value) != 0;
        }

        if (IS_STRING(value)) {
            return !AS_CPP_STRING(value).empty();
        }

        return false;
    }


    void push(const EvaluationValue &value) {
        if (static_cast<size_t>(sp - stack.begin()) == STACK_LIMIT) {
            throw std::runtime_error("Stack overflow.");
        }
        *sp = value;
        sp++;
    }

    uint16_t READ_SHORT() {
        uint16_t high = READ_BYTE();
        uint16_t low = READ_BYTE();
        return (high << 8) | low;
    }

    EvaluationValue pop() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
        }
        --sp;
        return *sp;
    }

    EvaluationValue peek() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
        }
        return *(sp - 1);
    }

    uint8_t READ_BYTE() {
        return *ip++;
    }

    EvaluationValue GET_CONST() {
        return co->constants[READ_BYTE()];
    }


    std::vector<EvaluationValue> locals;

    //Global object
    std::shared_ptr<Global> global;

    //Stack pointer
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp;

    //Instruction pointer (Program counter)
    uint8_t *ip;

    std::unique_ptr<parser> _parser;

    //Constant pool
    std::vector<EvaluationValue> constants;

    // Code object
    CodeObject *co;

    std::array<EvaluationValue, STACK_LIMIT> stack;
    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;

    std::vector<Frame> callStack;

};
