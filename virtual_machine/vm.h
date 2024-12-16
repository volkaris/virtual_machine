#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <memory>

#include "parser.h"
#include "bytecodeGenerator.h"
#include "Global.h"
#include "EvaluationValue.h"
#include "OpCode.h"

#ifndef STACK_LIMIT
#define STACK_LIMIT 512
#endif

// Maximum number of frames for call stack
#ifndef MAX_FRAMES
#define MAX_FRAMES 1024
#endif

// A call frame represents one function invocation.
struct CallFrame {
    CodeObject* co;
    uint8_t* ip;
    EvaluationValue* bp; // base pointer
    int argCount;
};

class vm {
public:
    vm()
            : global(std::make_shared<Global>()),
              _parser(std::make_unique<syntax::parser>()),
              _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        setGlobalVariables();
        sp = stack.begin();
        fp = 0; // no frames yet
    }

    EvaluationValue exec(const std::string &program) {
        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);

        // Push the main code object and call it (like a main function)
        push(ALLOC_CODE("main"));
        callValue(peek(), 0);

        return run();
    }

private:
    std::shared_ptr<Global> global;
    std::unique_ptr<syntax::parser> _parser;
    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;

    // Current executing code object and instruction pointer
    CodeObject *co;
    uint8_t *ip;

    // Stack and stack pointer
    std::array<EvaluationValue, STACK_LIMIT> stack;
    EvaluationValue* sp;

    // Call frames
    CallFrame frames[MAX_FRAMES];
    int fp; // frame pointer (index of current frame)

    void setGlobalVariables() {
        // Initialize any global variables or constants here if needed
    }

    EvaluationValue run() {
        for (;;) {
            uint8_t op_code = READ_BYTE();
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
                    double denominator = AS_NUMBER(right);
                    if (denominator == 0) {
                        throw std::runtime_error("Division by zero");
                    }
                    push(NUMBER(AS_NUMBER(left) / denominator));
                    break;
                }

                case OP_COMPARE: {
                    auto compareOp = READ_BYTE();
                    auto right = pop();
                    auto left = pop();
                    compare_values(left, right, compareOp);
                    break;
                }

                case OP_JUMP_IF_FALSE: {
                    uint16_t addr = READ_SHORT();
                    EvaluationValue condition = pop();
                    if (IS_BOOL(condition) && !AS_BOOL(condition)) {
                        ip = &co->code[addr];
                    }
                    break;
                }

                case OP_JUMP: {
                    uint16_t addr = READ_SHORT();
                    ip = &co->code[addr];
                    break;
                }

                case OP_NIL:
                    push(NIL());
                    break;

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
                    currentFrame().bp[slot] = pop();
                    break;
                }

                case OP_GET_LOCAL: {
                    uint8_t slot = READ_BYTE();
                    push(currentFrame().bp[slot]);
                    break;
                }

                case OP_LOGICAL_NOT: {
                    auto operand = pop();
                    push(BOOLEAN(!isTruth(operand)));
                    break;
                }

                case OP_JUMP_IF_FALSE_OR_POP: {
                    uint16_t address = READ_SHORT();
                    auto value = peek();
                    if (!isTruth(value)) {
                        pop();
                        ip = &co->code[address];
                    } else {
                        pop();
                    }
                    break;
                }

                case OP_JUMP_IF_TRUE_OR_POP: {
                    uint16_t address = READ_SHORT();
                    auto value = peek();
                    if (isTruth(value)) {
                        pop();
                        ip = &co->code[address];
                    } else {
                        pop();
                    }
                    break;
                }

                case OP_DUP: {
                    if (sp == stack.begin()) {
                        throw std::runtime_error("Stack underflow: cannot DUP");
                    }
                    EvaluationValue val = peek();
                    push(val);
                    break;
                }

                case OP_CALL: {
                    uint8_t argCount = READ_BYTE();
                    auto callee = peek(argCount);
                    callValue(callee, argCount);
                    break;
                }

                case OP_RETURN: {
                    EvaluationValue result = pop();
                    popFrame();
                    push(result);
                    if (fp == 0) {
                        // Returned from top-level (main) code object
                        return pop();
                    }
                    break;
                }

                default:
                    throw std::runtime_error("Unknown opcode: " + std::to_string(op_code));
            }
        }
    }

    inline uint8_t READ_BYTE() {
        return *ip++;
    }

    inline uint16_t READ_SHORT() {
        uint16_t high = READ_BYTE();
        uint16_t low = READ_BYTE();
        return (high << 8) | low;
    }

    inline EvaluationValue GET_CONST() {
        return co->constants[READ_BYTE()];
    }

    void push(const EvaluationValue &value) {
        if ((sp - stack.begin()) == STACK_LIMIT) {
            throw std::runtime_error("Stack overflow.");
        }
        *sp = value;
        sp++;
    }

    EvaluationValue pop() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack underflow.");
        }
        --sp;
        return *sp;
    }

    EvaluationValue peek(int distance = 0) {
        return *(sp - 1 - distance);
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

    template<typename T>
    void doCompare(const T &left, const T &right, uint8_t compare_op) {
        bool res = false;
        switch (compare_op) {
            case 0: res = left < right; break;
            case 1: res = left > right; break;
            case 2: res = left == right; break;
            case 3: res = left >= right; break;
            case 4: res = left <= right; break;
            case 5: res = left != right; break;
            default:
                throw std::runtime_error("Unknown compare operation: " + std::to_string(compare_op));
        }
        push(BOOLEAN(res));
    }

    void compare_values(const EvaluationValue &leftVal, const EvaluationValue &rightVal, uint8_t compare_op) {
        if (IS_NUMBER(leftVal) && IS_NUMBER(rightVal)) {
            doCompare(AS_NUMBER(leftVal), AS_NUMBER(rightVal), compare_op);
        } else if (IS_STRING(leftVal) && IS_STRING(rightVal)) {
            doCompare(AS_CPP_STRING(leftVal), AS_CPP_STRING(rightVal), compare_op);
        } else {
            throw std::runtime_error("Type error in COMPARE operation.");
        }
    }

    CallFrame& currentFrame() {
        return frames[fp - 1];
    }

    void callValue(EvaluationValue fnValue, int argCount) {
        if (!IS_CODE(fnValue)) {
            throw std::runtime_error("Attempt to call a non-function value.");
        }

        CodeObject* fn = AS_CODE(fnValue);

        if (fp == MAX_FRAMES) {
            throw std::runtime_error("Stack overflow: too many nested calls.");
        }

        // Setup a new frame
        CallFrame &frame = frames[fp++];
        frame.co = fn;
        frame.ip = &fn->code[0];
        frame.argCount = argCount;

        // Arguments + function object are on the stack:
        // stack top: ... argN, argN-1, ..., arg1, fnValue
        // Base pointer is sp - argCount - 1 for the function object
        frame.bp = sp - argCount - 1;

        // Replace the function object position (bp[0]) with fnValue so that it's accessible as a local if needed.
        *frame.bp = fnValue;

        co = frame.co;
        ip = frame.ip;
    }

    void popFrame() {
        CallFrame &frame = frames[--fp];
        // result is on top of stack, pop function and arguments
        EvaluationValue result = pop();
        int totalToPop = frame.argCount; // arguments
        while (totalToPop-- > 0) {
            pop();
        }
        // pop the function object itself
        pop();
        push(result);

        if (fp > 0) {
            co = frames[fp - 1].co;
            ip = frames[fp - 1].ip;
        }
    }
};

