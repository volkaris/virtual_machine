#pragma once
#include <vector>
#include <string>
#include "OpCode.h"
#include <array>
#include "parser.h"
using namespace syntax;

#define STACK_LIMIT 51200


#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"


struct CallFrame {
    CodeObject *co;
    uint8_t *ip;
    EvaluationValue *bp; // base pointer for this function's stack frame
    int arity;
    std::vector<EvaluationValue> locals; // Per-frame locals
};


class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        setGlobalVariables();
        /*locals.resize(1024);*/
        frameCount = 0;
    }

    EvaluationValue exec(const std::string &program) {
        std::shared_ptr<Exp> ast = _parser->parse(program);

        co = _bytecodeGenerator->compile(*ast);

        ip = &co->code[0];
        sp = stack.begin();

        /*_bytecodeGenerator->disassembleBytecode();*/

        // Initialize the main frame (even if it's empty)
        frames[frameCount].co = co;
        frames[frameCount].ip = ip;
        frames[frameCount].bp = sp;
        frames[frameCount].arity = 0;
        frames[frameCount].locals.resize(0);
        frameCount++;

        return evalExp();
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
        for (;;) {
            auto op_code = READ_BYTE();
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
                    if (slot >= frames[frameCount - 1].locals.size()) {
                        throw std::runtime_error("Invalid local slot.");
                    }
                    frames[frameCount - 1].locals[slot] = pop();
                    break;
                }

                case OP_GET_LOCAL: {
                    uint8_t slot = READ_BYTE();
                    if (slot >= frames[frameCount - 1].locals.size()) {
                        throw std::runtime_error("Invalid local slot.");
                    }
                    push(frames[frameCount - 1].locals[slot]);
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

                // CHANGED: Handle calls
                case OP_CALL: {
                    uint8_t argCount = READ_BYTE();
                    // The function is on top of the stack (after pushing args)
                    EvaluationValue fnVal = pop();
                    if (!IS_CODE(fnVal)) {
                        throw std::runtime_error("Attempt to call non-function.");
                    }

                    CodeObject *fn = AS_CODE(fnVal);

                    // Arguments are currently on stack
                    pushFrame(fn, argCount);
                    break;
                }

                // CHANGED: Handle return
                case OP_RETURN: {
                    EvaluationValue returnValue = pop();

                    // Pop current call frame
                    EvaluationValue *bp = frames[frameCount - 1].bp;
                    // Remove local frame including arguments and function object itself
                    sp = bp;
                    popFrame();

                    // Push return value
                    push(returnValue);

                    if (frameCount == 0) {
                        // If we returned from the top-level
                        return returnValue;
                    }
                    break;
                }


                default: {
                    throw std::runtime_error("Unknown opcode: " + std::to_string(op_code));
                }
            }
        }
    }

private:
    CallFrame frames[64];
    int frameCount = 0;

    void pushFrame(CodeObject *fn, int argCount) {
        if (frameCount == 64) {
            throw std::runtime_error("Call stack overflow.");
        }

        // Check arity
        if (argCount != fn->arity) {
            throw std::runtime_error("Function argument count mismatch.");
        }

        frames[frameCount].co = fn;
        frames[frameCount].ip = &fn->code[0];
        frames[frameCount].bp = &*(sp - argCount); // Correct assignment
        frames[frameCount].arity = fn->arity;
        frames[frameCount].locals.resize(fn->arity);

        // Assign arguments to locals
        for (int i = 0; i < fn->arity; i++) {
            frames[frameCount].locals[i] = *(frames[frameCount].bp + i);
        }

        frameCount++;
        co = fn;
        ip = frames[frameCount - 1].ip;
    }

    void popFrame() {
        frameCount--;
        if (frameCount < 0) {
            throw std::runtime_error("Call stack underflow.");
        }

        if (frameCount == 0) {
            // Returning from main
            co = nullptr;
            ip = nullptr;
        } else {
            co = frames[frameCount - 1].co;
            ip = frames[frameCount - 1].ip;
        }
    }

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


    /*std::vector<EvaluationValue> locals;*/

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
};
