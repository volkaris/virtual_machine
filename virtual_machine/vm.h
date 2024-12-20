#pragma once

#include <vector>
#include <string>
#include <memory>
#include <array>
#include "parser.h"
#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"

#define STACK_LIMIT 80000

struct CallFrame;
class vm;

typedef void (*InstructionHandler)(vm *, CallFrame &, uint8_t *&);

static void handleHalt(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleConst(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleAdd(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleSub(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleMul(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleDiv(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleCompare(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleJumpIfFalse(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleJump(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleGetGlobal(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleSetGlobal(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleGetLocal(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleSetLocal(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleLogicalNot(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleLogicalAnd(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleLogicalOr(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleJumpIfFalseOrPop(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleJumpIfTrueOrPop(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleDup(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleCall(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleReturn(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleArray(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleArrayGet(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleArraySet(vm *machine, CallFrame &frame, uint8_t *&ip);

static void handleNil(vm *machine, CallFrame &frame, uint8_t *&ip);

/*static void handlePrint(vm *machine, CallFrame &frame, uint8_t *&ip);*/

// Create the handlers table
// Make sure every opcode from your OpCode.h is assigned here in the correct order.
static InstructionHandler handlers[0xFF + 1] = {
    handleHalt,
    handleConst,
    handleAdd,
    handleSub,
    handleMul,
    handleDiv,
    handleCompare,
    handleJumpIfFalse,
    handleJump,
    handleGetGlobal,
    handleSetGlobal,
    handleGetLocal,
    handleSetLocal,
    handleLogicalNot,
    handleJumpIfFalseOrPop,
    handleJumpIfTrueOrPop,
    handleDup,
    handleNil,
    handleCall,
    handleReturn,
    handleArray,
    handleArrayGet,
    handleArraySet,
    /*handlePrint*/
};


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
        initializeBuiltins();
    }

    EvaluationValue exec(const std::string &program) {

        stack.clear();

        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);


        // Добавить глобальные функции как переменные
        for (const auto &builtin: global->builtinFunctions) {
            defineBuiltin(builtin.first);
        }

        // Initialize the main call frame
        callStack.emplace_back(co);

        // Initialize the instruction pointer in the main call frame
        CallFrame &currentFrame = callStack.back();
        currentFrame.ip = currentFrame.co->code.data();
        /*sp = stack.begin();*/

        // Optionally, disassemble bytecode for debugging
        /*_bytecodeGenerator->disassembleBytecode();*/

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
            uint8_t *&ip = currentFrame.ip;

            // Instead of switch:
            // auto op_code = READ_BYTE(ip);
            // switch(op_code) { ... }

            uint8_t op_code = *ip++;
            handlers[op_code](this, currentFrame, ip);

            // If callStack was cleared by handleHalt or OP_RETURN logic ended execution, break out
            if (callStack.empty()) {
                break;
            }
        }

        // After the main loop, return top of stack or NIL
        // This depends on your original design
        if (stack.empty()) {
            return NIL();
        }
        return pop();
    }


    void setGlobalVariables() {
        global->setGlobalVariables();
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
        stack.push_back(value);
    }

    uint16_t READ_SHORT(uint8_t *&ip) {
        uint16_t high = READ_BYTE(ip);
        uint16_t low = READ_BYTE(ip);
        return (high << 8) | low;
    }

    EvaluationValue pop() {
        if (stack.empty()) {
            throw std::runtime_error("Stack empty.");
        }
        EvaluationValue val = stack.back();
        stack.pop_back();
        return val;
    }

    EvaluationValue peek() {
        if (stack.empty()) {
            throw std::runtime_error("Stack empty.");
        }
        return stack.back();
    }

    uint8_t READ_BYTE(uint8_t *&ip) {
        return *ip++;
    }

    EvaluationValue GET_CONST(uint8_t *&ip, CodeObject *co) {
        return co->constants[READ_BYTE(ip)];
    }

    void initializeBuiltins() {
        for (const auto &[name, func]: global->builtinFunctions) {
            // Создание CodeObject для встроенной функции
            CodeObject *codeObj = new CodeObject(name);
            builtins[name] = codeObj;

            // Добавление в глобальные переменные
            int globalIdx = global->getGlobalIndex(name);
            if (globalIdx == -1) {
                global->define(name);
                globalIdx = global->getGlobalIndex(name);
            }
            global->set(globalIdx, ALLOC_CODE(name));
        }
    }

    void defineBuiltin(const std::string &name) {
        int globalIdx = global->getGlobalIndex(name);
        if (globalIdx == -1) {
            throw std::runtime_error("Не удалось определить встроенную функцию: " + name);
        }
        // Привязка CodeObject к встроенной функции
        CodeObject *codeObj = builtins[name];
        global->globals[globalIdx].value = ALLOC_CODE(name);
        AS_CODE(global->globals[globalIdx].value)->name = name;
        AS_CODE(global->globals[globalIdx].value)->constants.push_back(NUMBER(0)); // Placeholder
    }

    std::vector<EvaluationValue> locals;

    //Global object
    std::shared_ptr<Global> global;

    //Stack pointer
    /*std::array<EvaluationValue, STACK_LIMIT> stack;
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp = stack.begin();*/

    std::vector<EvaluationValue> stack;


    std::unique_ptr<syntax::parser> _parser;

    // Code object
    CodeObject *co;

    std::vector<CallFrame> callStack;

    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;

    std::unordered_map<std::string, CodeObject *> builtins;
};


// Define a function pointer type for handlers


// For any null handlers, we should ensure they throw an error if reached:
static void verifyHandlers() {
    for (int i = 0; i <= 0xFF; i++) {
        if (handlers[i] == nullptr) {
            handlers[i] = [](vm *, CallFrame &, uint8_t *&) {
                throw std::runtime_error("No handler implemented for this opcode");
            };
        }
    }
}

// Call this before running evalExp() to ensure no null handlers remain
// In this example, we do it right after defining them
static bool handlersInitialized = []() {
    verifyHandlers();
    return true;
}();


// ====================== Handler Implementations ======================

static void handleHalt(vm *machine, CallFrame &frame, uint8_t *&ip) {
    // Return top of stack or NIL if empty, and end execution
    EvaluationValue result = IS_NIL(machine->peek()) ? NIL() : machine->pop();
    // Clear call stack to end execution
    machine->callStack.clear();
    // Push result back if you want final result:
    machine->push(result);
}

static void handleConst(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t constIndex = *ip++;
    machine->push(frame.co->constants[constIndex]);
}

static void handleAdd(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleSub(vm *machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    machine->push(NUMBER(AS_NUMBER(left) - AS_NUMBER(right)));
}

static void handleMul(vm *machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    machine->push(NUMBER(AS_NUMBER(left) * AS_NUMBER(right)));
}

static void handleDiv(vm *machine, CallFrame &frame, uint8_t *&ip) {
    auto right = machine->pop();
    auto left = machine->pop();
    double casted_left = AS_NUMBER(left);
    double casted_right = AS_NUMBER(right);
    if (casted_right == 0) {
        throw std::runtime_error("Division by zero");
    }
    machine->push(NUMBER(casted_left / casted_right));
}

static void handleCompare(vm *machine, CallFrame &frame, uint8_t *&ip) {
    auto compareOp = *ip++;
    auto right = machine->pop();
    auto left = machine->pop();

    auto compare_values = [&](auto casted_left, auto casted_right) {
        bool res = false;
        switch (compareOp) {
            case 0: res = (casted_left < casted_right);
                break;
            case 1: res = (casted_left > casted_right);
                break;
            case 2: res = (casted_left == casted_right);
                break;
            case 3: res = (casted_left >= casted_right);
                break;
            case 4: res = (casted_left <= casted_right);
                break;
            case 5: res = (casted_left != casted_right);
                break;
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

static void handleJumpIfFalse(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t addr = (ip[0] << 8) | ip[1];
    ip += 2;
    EvaluationValue condition = machine->pop();
    if (IS_BOOL(condition) && !AS_BOOL(condition)) {
        ip = &frame.co->code[addr];
    }
}

static void handleJump(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t addr = (ip[0] << 8) | ip[1];
    ip += 2;
    ip = &frame.co->code[addr];
}

static void handleGetGlobal(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t globalIndex = *ip++;
    machine->push(machine->global->get(globalIndex).value);
}

static void handleSetGlobal(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t globalIndex = *ip++;
    EvaluationValue val = machine->pop();
    machine->global->set(globalIndex, val);
}

static void handleGetLocal(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t slot = *ip++;
    machine->push(frame.locals[slot]);
}

static void handleSetLocal(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t slot = *ip++;
    frame.locals[slot] = machine->pop();
}

static void handleLogicalNot(vm *machine, CallFrame &frame, uint8_t *&ip) {
    auto operand = machine->pop();
    bool result = !machine->isTruth(operand);
    machine->push(BOOLEAN(result));
}

static void handleLogicalAnd(vm *machine, CallFrame &frame, uint8_t *&ip) {
    // If desired, implement logical AND short-circuiting if needed
    // For now, throw if not implemented
    throw std::runtime_error("OP_LOGICAL_AND not implemented in handlers");
}

static void handleLogicalOr(vm *machine, CallFrame &frame, uint8_t *&ip) {
    // If desired, implement logical OR short-circuiting if needed
    throw std::runtime_error("OP_LOGICAL_OR not implemented in handlers");
}

static void handleJumpIfFalseOrPop(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleJumpIfTrueOrPop(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleDup(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->peek();
    machine->push(value);
}

static void handleCall(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t argCount = *ip++;

    std::vector<EvaluationValue> args(argCount);
    for (int i = argCount - 1; i >= 0; --i) {
        args[i] = machine->pop();
    }

    EvaluationValue funcVal = machine->pop();
    if (!IS_OBJECT(funcVal) || !IS_CODE(funcVal)) {
        throw std::runtime_error("Attempting to call a non-function.");
    }

    CodeObject *functionCo = AS_CODE(funcVal);

    // Проверка, является ли функция встроенной
    bool isBuiltin = false;
    std::string builtinName;
    for (const auto &[name, codeObj]: machine->builtins) {
        if (name == functionCo->name) {
            isBuiltin = true;
            builtinName = name;
            break;
        }
    }

    if (isBuiltin) {
        // Вызов встроенной функции через Global
        EvaluationValue result = machine->global->callBuiltin(builtinName, args);
        machine->push(result);
    } else {
        // Вызов пользовательской функции
        // Создание нового фрейма вызова
        CallFrame newFrame(functionCo);

        // Установка локальных переменных (параметров)
        for (size_t i = 0; i < args.size(); ++i) {
            int slot = i;
            if (slot >= functionCo->localNames.size()) {
                throw std::runtime_error("Слишком много аргументов при вызове функции.");
            }
            newFrame.locals[slot] = args[i];
        }

        machine->callStack.push_back(newFrame);
    }
}

static void handleReturn(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleArray(vm *machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(ALLOC_ARRAY());
}

static void handleArrayGet(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("Attempting to index a non-array.");
    }
    ArrayObject *array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("Array index must be a number.");
    }

    size_t index = (size_t) AS_NUMBER(indexVal);
    if (index >= array->elements.size()) {
        throw std::runtime_error("Array index out of bounds.");
    }

    machine->push(array->elements[index]);
}

static void handleArraySet(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->pop();
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("Attempting to index a non-array.");
    }
    ArrayObject *array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("Array index must be a number.");
    }

    size_t index = (size_t) AS_NUMBER(indexVal);
    if (index >= array->elements.size()) {
        array->elements.resize(index + 1, NIL());
    }
    array->elements[index] = value;
}

static void handleNil(vm *machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(NIL());
}

/*static void handlePrint(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue val = machine->peek();

    std::string output = evaluationValueToConstantString(val);
    std::cout << output << std::endl;
}*/
