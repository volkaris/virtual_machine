//
// Created by Ilya on 18.12.2024.
//

#

#include <stdexcept>

#include "EvaluationValue.h"
#include "vm.h"


InstructionHandler handlers[0xFF + 1];

// For any null handlers, we should ensure they throw an error if reached:
static void verifyHandlers() {
    for (int i = 0; i <= 0xFF; i++) {
        if (handlers[i] == nullptr) {
            handlers[i] = [](vm*, CallFrame&, const std::vector<uint8_t>&) {
                throw std::runtime_error("No handler implemented for this opcode");
            };
        }
    }
}

// Call this before running evalExp() to ensure no null handlers remain
// In this example, we do it right after defining them
static bool handlersInitialized = [](){
    verifyHandlers();
    return true;
}();


// ====================== Handler Implementations ======================

static void handleHalt(vm* machine, CallFrame &frame, uint8_t *&ip) {
    // Return top of stack or NIL if empty, and end execution
    EvaluationValue result = IS_NIL(machine->peek()) ? NIL() : machine->pop();
    // Clear call stack to end execution
    machine->callStack.clear();
    // Push result back if you want final result:
    machine->push(result);
}

static void handleConst(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t constIndex = *ip++;
    machine->push(frame.co->constants[constIndex]);
}

static void handleAdd(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    if (IS_NUMBER(left) && IS_NUMBER(right)) {
        machine->push(NUMBER(AS_NUMBER(left) + AS_NUMBER(right)));
    } else if (IS_STRING(left) && IS_STRING(right)) {
        machine->push(ALLOC_STRING(AS_CPP_STRING(left) + AS_CPP_STRING(right)));
    } else {
        throw std::runtime_error("Type error in ADD operation.");
    }
}

static void handleSub(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    machine->push(NUMBER(AS_NUMBER(left) - AS_NUMBER(right)));
}

static void handleMul(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    machine->push(NUMBER(AS_NUMBER(left) * AS_NUMBER(right)));
}

static void handleDiv(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    double casted_left = AS_NUMBER(left);
    double casted_right = AS_NUMBER(right);
    if (casted_right == 0) {
        throw std::runtime_error("Division by zero");
    }
    machine->push(NUMBER(casted_left / casted_right));
}

static void handleCompare(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto compareOp = *ip++;
    auto right = machine->pop();
    auto left = machine->pop();

    auto compare_values = [&](auto casted_left, auto casted_right) {
        bool res = false;
        switch (compareOp) {
            case 0: res = (casted_left < casted_right); break;
            case 1: res = (casted_left > casted_right); break;
            case 2: res = (casted_left == casted_right); break;
            case 3: res = (casted_left >= casted_right); break;
            case 4: res = (casted_left <= casted_right); break;
            case 5: res = (casted_left != casted_right); break;
            default:
                throw std::runtime_error("Unknown compare operation.");
        }
        machine->push(BOOLEAN(res));
    };

    if (IS_NUMBER(left) && IS_NUMBER(right)) {
        compare_values(AS_NUMBER(left), AS_NUMBER(right));
    } else if (IS_STRING(left) && IS_STRING(right)) {
        compare_values(AS_CPP_STRING(left), AS_CPP_STRING(right));
    } else {
        throw std::runtime_error("Type error in COMPARE operation.");
    }
}

static void handleJumpIfFalse(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t addr = (ip[0] << 8) | ip[1];
    ip += 2;
    EvaluationValue condition = machine->pop();
    if (IS_BOOL(condition) && !AS_BOOL(condition)) {
        ip = &frame.co->code[addr];
    }
}

static void handleJump(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t addr = (ip[0] << 8) | ip[1];
    ip += 2;
    ip = &frame.co->code[addr];
}

static void handleGetGlobal(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t globalIndex = *ip++;
    machine->push(machine->global->get(globalIndex).value);
}

static void handleSetGlobal(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t globalIndex = *ip++;
    EvaluationValue val = machine->pop();
    machine->global->set(globalIndex, val);
}

static void handleGetLocal(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t slot = *ip++;
    machine->push(frame.locals[slot]);
}

static void handleSetLocal(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t slot = *ip++;
    frame.locals[slot] = machine->pop();
}

static void handleLogicalNot(vm* machine, CallFrame &frame, uint8_t *&ip) {
    auto operand = machine->pop();
    bool result = !machine->isTruth(operand);
    machine->push(BOOLEAN(result));
}

static void handleLogicalAnd(vm* machine, CallFrame &frame, uint8_t *&ip) {
    // If desired, implement logical AND short-circuiting if needed
    // For now, throw if not implemented
    throw std::runtime_error("OP_LOGICAL_AND not implemented in handlers");
}

static void handleLogicalOr(vm* machine, CallFrame &frame, uint8_t *&ip) {
    // If desired, implement logical OR short-circuiting if needed
    throw std::runtime_error("OP_LOGICAL_OR not implemented in handlers");
}

static void handleJumpIfFalseOrPop(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t address = (ip[0] << 8) | ip[1];
    ip += 2;
    auto value = machine->peek();
    if (!machine->isTruth(value)) {
        machine->pop(); // Remove the value
        ip = &frame.co->code[address];
    } else {
        machine->pop(); // Remove the value if not jumping
    }
}

static void handleJumpIfTrueOrPop(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t address = (ip[0] << 8) | ip[1];
    ip += 2;
    auto value = machine->peek();
    if (machine->isTruth(value)) {
        machine->pop(); // Remove the value
        ip = &frame.co->code[address];
    } else {
        machine->pop(); // Remove the value if not jumping
    }
}

static void handleDup(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->peek();
    machine->push(value);
}

static void handleCall(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t argCount = *ip++;

    std::vector<EvaluationValue> args(argCount);
    for (int i = argCount - 1; i >= 0; --i) {
        args[i] = machine->pop();
    }

    EvaluationValue funcVal = machine->pop();
    if (!IS_OBJECT(funcVal) || !IS_CODE(funcVal)) {
        throw std::runtime_error("Attempting to call a non-function.");
    }

    CodeObject* functionCo = AS_CODE(funcVal);

    // Create a new call frame for the function
    CallFrame newFrame(functionCo);

    // Assign arguments to the function's local variables
    size_t paramIndex = 0;
    for (int slot = 0; slot < (int)functionCo->localNames.size(); ++slot) {
        if (functionCo->localNames.find(slot) != functionCo->localNames.end()) {
            if (paramIndex < args.size()) {
                newFrame.locals[slot] = args[paramIndex++];
            } else {
                newFrame.locals[slot] = NIL();
            }
        }
    }

    machine->callStack.push_back(newFrame);
}

static void handleReturn(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue returnValue = machine->pop();
    machine->callStack.pop_back();

    if (machine->callStack.empty()) {
        // No more frames, this ends execution
        machine->push(returnValue);
        return;
    }

    // Push the return value onto the previous frame's stack
    machine->push(returnValue);
}

static void handleArray(vm* machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(ALLOC_ARRAY());
}

static void handleArrayGet(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("Attempting to index a non-array.");
    }
    ArrayObject* array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("Array index must be a number.");
    }

    size_t index = (size_t)AS_NUMBER(indexVal);
    if (index >= array->elements.size()) {
        throw std::runtime_error("Array index out of bounds.");
    }

    machine->push(array->elements[index]);
}

static void handleArraySet(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->pop();
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("Attempting to index a non-array.");
    }
    ArrayObject* array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("Array index must be a number.");
    }

    size_t index = (size_t)AS_NUMBER(indexVal);
    if (index >= array->elements.size()) {
        array->elements.resize(index + 1, NIL());
    }
    array->elements[index] = value;
}

static void handleNil(vm* machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(NIL());
}