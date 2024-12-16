#pragma once
#include <vector>
#include <string>
#include <array>
#include "OpCode.h"
#include <stdexcept>
#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"
#include "parser.h"

#define STACK_LIMIT 512
#define MAX_FRAMES 64

struct CallFrame {
    CodeObject* co;
    uint8_t* ip;
    EvaluationValue* locals;
};

class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<syntax::parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        locals.resize(1024);
    }

    EvaluationValue exec(const std::string &program) {
        std::shared_ptr<Exp> ast = _parser->parse(program);
        CodeObject* mainCo = _bytecodeGenerator->compile(*ast);

        globalFunctions = _bytecodeGenerator->getAllFunctions();

        ip = &mainCo->code[0];
        co = mainCo;
        sp = stack.begin();

        _bytecodeGenerator->disassembleBytecode();

        // Initialize call frame for main
        frames[0].co = mainCo;
        frames[0].ip = ip;
        frames[0].locals = &(*sp); // start of stack
        frameCount = 1;

        return run();
    }

private:
    std::shared_ptr<Global> global;
    std::unique_ptr<syntax::parser> _parser;
    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;

    std::vector<EvaluationValue> locals;
    std::array<EvaluationValue, STACK_LIMIT> stack;
    std::vector<CodeObject*> globalFunctions;

    // VM registers
    uint8_t* ip;
    CodeObject* co;
    std::array<CallFrame, MAX_FRAMES> frames;
    int frameCount = 0;

    // stack pointer
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp = stack.begin();

    inline uint8_t READ_BYTE() { return *ip++; }
    inline uint16_t READ_SHORT() {
        uint16_t high = READ_BYTE();
        uint16_t low = READ_BYTE();
        return (high << 8) | low;
    }

    inline EvaluationValue GET_CONST() {
        return co->constants[READ_BYTE()];
    }

    void push(const EvaluationValue &value) {
        if (static_cast<size_t>(sp - stack.begin()) == STACK_LIMIT) {
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

    EvaluationValue peek() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
        }
        return *(sp - 1);
    }

    bool isTruth(const EvaluationValue &value) {
        if (IS_BOOL(value)) return AS_BOOL(value);
        if (IS_NIL(value)) return false;
        if (IS_NUMBER(value)) return AS_NUMBER(value) != 0;
        if (IS_STRING(value)) return !AS_CPP_STRING(value).empty();
        return false; // fallback
    }

    template<typename T>
    void compare_values(const T &left, const T &right, uint8_t compare_op) {
        bool res = false;
        switch (compare_op) {
            case 0: res = left < right; break;
            case 1: res = left > right; break;
            case 2: res = left == right; break;
            case 3: res = left >= right; break;
            case 4: res = left <= right; break;
            case 5: res = left != right; break;
            default:
                throw std::runtime_error("Unknown compare operation.");
        }
        push(BOOLEAN(res));
    }

    EvaluationValue run() {
        for (;;) {
            auto op_code = READ_BYTE();
            switch (op_code) {
                case OP_HALT:
                    return pop();

                case OP_CONST: {
                    push(GET_CONST());
                    break;
                }

                case OP_ADD: {
                    auto right = pop();
                    auto left = pop();
                    if (IS_NUMBER(left) && IS_NUMBER(right)) {
                        push(NUMBER(AS_NUMBER(left) + AS_NUMBER(right)));
                    } else if (IS_STRING(left) && IS_STRING(right)) {
                        push(ALLOC_STRING(AS_CPP_STRING(left) + AS_CPP_STRING(right)));
                    } else {
                        throw std::runtime_error("Type error in ADD.");
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
                    double d = AS_NUMBER(right);
                    if (d == 0) throw std::runtime_error("Division by zero");
                    push(NUMBER(AS_NUMBER(left) / d));
                    break;
                }

                case OP_COMPARE: {
                    uint8_t compareOp = READ_BYTE();
                    auto right = pop();
                    auto left = pop();
                    if (IS_NUMBER(left) && IS_NUMBER(right)) {
                        compare_values(AS_NUMBER(left), AS_NUMBER(right), compareOp);
                    } else if (IS_STRING(left) && IS_STRING(right)) {
                        compare_values(AS_CPP_STRING(left), AS_CPP_STRING(right), compareOp);
                    } else {
                        throw std::runtime_error("Type error in COMPARE.");
                    }
                    break;
                }

                case OP_JUMP_IF_FALSE: {
                    uint16_t addr = READ_SHORT();
                    auto condition = pop();
                    if (!isTruth(condition)) {
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
                    uint8_t globalIndex = READ_BYTE();
                    push(global->get(globalIndex).value);
                    break;
                }

                case OP_SET_GLOBAL: {
                    uint8_t globalIndex = READ_BYTE();
                    global->set(globalIndex, pop());
                    break;
                }

                case OP_SET_LOCAL: {
                    uint8_t slot = READ_BYTE();
                    frames[frameCount-1].locals[slot] = pop();
                    break;
                }

                case OP_GET_LOCAL: {
                    uint8_t slot = READ_BYTE();
                    push(frames[frameCount-1].locals[slot]);
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
                    auto value = peek();
                    push(value);
                    break;
                }

                case OP_CALL: {
                    uint8_t funcIndex = READ_BYTE();
                    uint8_t argCount = READ_BYTE();

                    if (funcIndex >= globalFunctions.size()) {
                        throw std::runtime_error("Invalid function index in CALL.");
                    }
                    CodeObject* fco = globalFunctions[funcIndex];

                    if (frameCount == MAX_FRAMES) {
                        throw std::runtime_error("Too many nested function calls.");
                    }

                    // Arguments are on stack top
                    // The new frame's locals start where sp - argCount
                    EvaluationValue* base = &(*(sp - argCount));
                    // Push a frame
                    frames[frameCount].co = fco;
                    frames[frameCount].ip = &fco->code[0];
                    frames[frameCount].locals = base;

                    // Advance frameCount
                    frameCount++;

                    // Set co and ip
                    co = fco;
                    ip = frames[frameCount-1].ip;

                    break;
                }

                case OP_RETURN: {
                    // Return from current function
                    EvaluationValue returnValue = pop(); // function result
                    frameCount--;
                    if (frameCount < 0) {
                        throw std::runtime_error("Return with no call frame");
                    }

                    // Pop the arguments and locals
                    sp = frames[frameCount].locals; // restore stack
                    push(returnValue);

                    if (frameCount > 0) {
                        co = frames[frameCount-1].co;
                        ip = frames[frameCount-1].ip;
                    } else {
                        // Returned from main (top-level)
                        return returnValue;
                    }
                    break;
                }

                default:
                    throw std::runtime_error("Unknown opcode.");
            }
            // Update IP in frame
            frames[frameCount-1].ip = ip;
        }
    }
};
