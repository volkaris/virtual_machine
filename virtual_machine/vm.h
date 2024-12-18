#pragma once

#include <vector>
#include <string>
#include <memory>
#include <array>
#include "OpCode.h"
#include "parser.h"
#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"

#define STACK_LIMIT 512

struct CallFrame {
    CodeObject *co; // Pointer to the function's CodeObject
    uint8_t *ip; // Instruction pointer within the function's bytecode
    std::vector<EvaluationValue> locals; // Local variables for the function

    CallFrame(CodeObject *codeObject)
        : co(codeObject), ip(codeObject->code.data()) {
        // Determine the maximum slot index
        int maxSlot = -1;
        for (const auto &pair: co->localNames) {
            if (pair.first > maxSlot) {
                maxSlot = pair.first;
            }
        }
        // Initialize locals vector with NIL
        locals.resize(maxSlot + 1, NIL());
    }
};

class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<syntax::parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        setGlobalVariables();
    }

    EvaluationValue exec(const std::string &program) {

        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);

        // Initialize the main call frame
        callStack.emplace_back(co);

        // Initialize the instruction pointer in the main call frame
        CallFrame &currentFrame = callStack.back();
        currentFrame.ip = currentFrame.co->code.data();
        sp = stack.begin();

        // Optionally, disassemble bytecode for debugging
        // _bytecodeGenerator->disassembleBytecode();

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
        while (!callStack.empty()) {
            CallFrame &currentFrame = callStack.back();
            uint8_t *&ip = currentFrame.ip; // Reference to the frame's ip

            auto op_code = READ_BYTE(ip);
            switch (op_code) {
                case OP_HALT:
                    return pop();

                case OP_CONST:
                    push(GET_CONST(ip, currentFrame.co));
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

                    double casted_left = AS_NUMBER(left);
                    double casted_right = AS_NUMBER(right);
                    if (casted_right == 0) {
                        throw std::runtime_error("division by zero");
                    }
                    push(NUMBER(casted_left / casted_right));
                    break;
                }

                case OP_COMPARE: {
                    auto compareOp = READ_BYTE(ip);

                    auto right = pop();
                    auto left = pop();

                    if (IS_NUMBER(left) && IS_NUMBER(right)) {
                        double castedLeft = AS_NUMBER(left);
                        double castedRight = AS_NUMBER(right);
                        compare_values(castedLeft, castedRight, compareOp);
                    } else if (IS_STRING(left) && IS_STRING(right)) {
                        std::string castedLeft = AS_CPP_STRING(left);
                        std::string castedRight = AS_CPP_STRING(right);
                        compare_values(castedLeft, castedRight, compareOp);
                    } else {
                        throw std::runtime_error("Type error in COMPARE operation.");
                    }
                    break;
                }

                case OP_JUMP_IF_FALSE: {
                    uint16_t addr = READ_SHORT(ip);
                    EvaluationValue condition = pop();
                    if (IS_BOOL(condition) && !AS_BOOL(condition)) {
                        ip = &currentFrame.co->code[addr];
                    }
                    break;
                }

                case OP_JUMP: {
                    uint16_t addr = READ_SHORT(ip);
                    ip = &currentFrame.co->code[addr];
                    break;
                }

                case OP_NIL: {
                    push(NIL());
                    break;
                }

                case OP_GET_GLOBAL: {
                    auto globalIndex = READ_BYTE(ip);
                    push(global->get(globalIndex).value);
                    break;
                }

                case OP_SET_GLOBAL: {
                    auto globalIndex = READ_BYTE(ip);
                    global->set(globalIndex, pop());
                    break;
                }

                case OP_SET_LOCAL: {
                    uint8_t slot = READ_BYTE(ip);
                    currentFrame.locals[slot] = pop();
                    break;
                }

                case OP_GET_LOCAL: {
                    uint8_t slot = READ_BYTE(ip);
                    push(currentFrame.locals[slot]);
                    break;
                }

                case OP_LOGICAL_NOT: {
                    auto operand = pop();
                    bool result = !isTruth(operand);
                    push(BOOLEAN(result));
                    break;
                }

                case OP_JUMP_IF_FALSE_OR_POP: {
                    uint16_t address = READ_SHORT(ip);
                    auto value = peek();

                    if (!isTruth(value)) {
                        pop(); // Remove the value
                        ip = &currentFrame.co->code[address];
                    } else {
                        pop(); // Remove the value
                    }
                    break;
                }

                case OP_JUMP_IF_TRUE_OR_POP: {
                    uint16_t address = READ_SHORT(ip);
                    auto value = peek();

                    if (isTruth(value)) {
                        pop(); // Remove the value
                        ip = &currentFrame.co->code[address];
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
                    uint8_t argCount = READ_BYTE(ip);

                    // Pop arguments in reverse order to maintain correct order
                    std::vector<EvaluationValue> args(argCount);
                    for (int i = argCount - 1; i >= 0; --i) {
                        args[i] = pop();
                    }

                    // Pop the function object
                    EvaluationValue funcVal = pop();
                    if (!IS_OBJECT(funcVal) || !IS_CODE(funcVal)) {
                        throw std::runtime_error("Attempting to call a non-function.");
                    }

                    CodeObject *functionCo = AS_CODE(funcVal);

                    // Create a new call frame for the function
                    CallFrame newFrame(functionCo);

                    // Assign arguments to the function's local variables
                    size_t paramIndex = 0;
                    // Iterate over slot indices in ascending order
                    for (int slot = 0; slot < functionCo->localNames.size(); ++slot) {
                        auto it = functionCo->localNames.find(slot);
                        if (it != functionCo->localNames.end()) {
                            if (paramIndex < args.size()) {
                                newFrame.locals[slot] = args[paramIndex];
                                paramIndex++;
                            } else {
                                newFrame.locals[slot] = NIL();
                            }
                        }
                    }

                    // Push the new call frame onto the stack
                    callStack.emplace_back(newFrame);

                    break;
                }

                case OP_RETURN: {
                    // Pop the return value
                    EvaluationValue returnValue = pop();

                    // Pop the current call frame
                    callStack.pop_back();

                    if (callStack.empty()) {
                        // If no more call frames, halt execution
                        return returnValue;
                    }

                    // Push the return value onto the previous frame's stack
                    push(returnValue);

                    break;
                }


                case OP_ARRAY: {
                    // Create a new array and push it onto the stack
                    push(ALLOC_ARRAY());
                    break;
                }

                case OP_ARRAY_GET: {
                    // Pop the index and array from the stack
                    EvaluationValue indexVal = pop();
                    EvaluationValue arrayVal = pop();

                    // Validate that the popped value is an array
                    if (!IS_ARRAY(arrayVal)) {
                        throw std::runtime_error("Attempting to index a non-array.");
                    }

                    ArrayObject* array = AS_ARRAY(arrayVal);

                    // Ensure the index is a number
                    if (!IS_NUMBER(indexVal)) {
                        throw std::runtime_error("Array index must be a number.");
                    }

                    double indexDouble = AS_NUMBER(indexVal);
                    size_t index = static_cast<size_t>(indexDouble);

                    // Bounds checking
                    if (index >= array->elements.size()) {
                        throw std::runtime_error("Array index out of bounds.");
                    }

                    // Push the array element onto the stack
                    push(array->elements[index]);
                    break;
                }

                case OP_ARRAY_SET: {
                    // Pop the value, index, and array from the stack
                    EvaluationValue value = pop();
                    EvaluationValue indexVal = pop();
                    EvaluationValue arrayVal = pop();

                    // Validate that the popped value is an array
                    if (!IS_ARRAY(arrayVal)) {
                        throw std::runtime_error("Attempting to index a non-array.");
                    }

                    ArrayObject* array = AS_ARRAY(arrayVal);

                    // Ensure the index is a number
                    if (!IS_NUMBER(indexVal)) {
                        throw std::runtime_error("Array index must be a number.");
                    }

                    double indexDouble = AS_NUMBER(indexVal);
                    size_t index = static_cast<size_t>(indexDouble);

                    // Bounds checking and resizing if necessary
                    if (index >= array->elements.size()) {
                        // Resize the array to accommodate the new index
                        array->elements.resize(index + 1, NIL());
                    }

                    // Assign the value to the specified index
                    array->elements[index] = value;
                    break;
                }

                default: {
                    throw std::runtime_error("Unknown opcode: " + std::to_string(op_code));
                }
            }
            // No need to set currentFrame.ip = ip; since ip is a reference to currentFrame.ip
        }
        return NIL();
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

    uint16_t READ_SHORT(uint8_t *&ip) {
        uint16_t high = READ_BYTE(ip);
        uint16_t low = READ_BYTE(ip);
        return (high << 8) | low;
    }

    EvaluationValue pop() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
        }
        sp--;
        return *sp;
    }

    EvaluationValue peek() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
        }
        return *(sp - 1);
    }

    uint8_t READ_BYTE(uint8_t *&ip) {
        return *ip++;
    }

    EvaluationValue GET_CONST(uint8_t *&ip, CodeObject *co) {
        return co->constants[READ_BYTE(ip)];
    }


    std::vector<EvaluationValue> locals;

    //Global object
    std::shared_ptr<Global> global;

    //Stack pointer
    std::array<EvaluationValue, STACK_LIMIT> stack;
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp = stack.begin();

    std::unique_ptr<syntax::parser> _parser;

    // Code object
    CodeObject *co;

    std::vector<CallFrame> callStack;

    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;
};
