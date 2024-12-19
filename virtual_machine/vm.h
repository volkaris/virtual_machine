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

struct CallFrame;
class vm;

typedef void (*InstructionHandler)(vm*, CallFrame&, uint8_t*&);


static void handleHalt(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleConst(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleAdd(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleSub(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleMul(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleDiv(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleCompare(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJumpIfFalse(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJump(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleGetGlobal(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleSetGlobal(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleGetLocal(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleSetLocal(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleLogicalNot(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJumpIfFalseOrPop(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJumpIfTrueOrPop(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleDup(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleCall(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleReturn(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArray(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArrayGet(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArraySet(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleNil(vm* machine, CallFrame &frame, uint8_t *&ip);


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
};

struct CallFrame {
    CodeObject *co; // Указатель на CodeObject текущей функции
    uint8_t *ip; // Инструкционный указатель
    std::vector<EvaluationValue> locals; // Локальные переменные

    CallFrame(CodeObject *codeObject)
        : co(codeObject), ip(codeObject->code.data()) {

        int maxSlot = -1;
        for (const auto &pair: co->localNames) {
            if (pair.first > maxSlot) {
                maxSlot = pair.first;
            }
        }

        locals.resize(maxSlot + 1, NIL());
    }
};

class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<syntax::parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {}

    EvaluationValue exec(const std::string &program) {
        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);

        // Инициализация главного вызова (main)
        callStack.emplace_back(co);

        CallFrame &currentFrame = callStack.back();
        currentFrame.ip = currentFrame.co->code.data();
        sp = stack.begin();


        /*_bytecodeGenerator->disassembleBytecode();*/

        return evalExp();
    }


    EvaluationValue evalExp() {
        while (!callStack.empty()) {
            CallFrame &currentFrame = callStack.back();
            uint8_t *&ip = currentFrame.ip;

            uint8_t op_code = *ip++;
            handlers[op_code](this, currentFrame, ip);

            if (callStack.empty()) {
                break;
            }
        }

        if (sp == stack.begin()) {
            return NIL();
        }
        return pop();
    }



    bool isTruth(const EvaluationValue &value) {
        //  приведение к bool:
        // false, nil -> false
        // число 0 -> false, иначе true
        // пустая строка -> false, иначе true
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
            throw std::runtime_error("стек переполнен");
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
            throw std::runtime_error("стек пуст");
        }
        sp--;
        return *sp;
    }

    EvaluationValue peek() {
        if (sp == stack.begin()) {
            throw std::runtime_error("стек пуст");
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

    std::shared_ptr<Global> global;

    std::array<EvaluationValue, STACK_LIMIT> stack;
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp = stack.begin();

    std::unique_ptr<syntax::parser> _parser;

    CodeObject *co;

    std::vector<CallFrame> callStack;

    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;
};

// Проверка, чтобы не было нулевых обработчиков
static void verifyHandlers() {
    for (int i = 0; i <= 0xFF; i++) {
        if (handlers[i] == nullptr) {
            handlers[i] = [](vm*, CallFrame&, uint8_t *&) {
                throw std::runtime_error("No handler implemented for this opcode");
            };
        }
    }
}

static bool handlersInitialized = [](){
    verifyHandlers();
    return true;
}();



static void handleHalt(vm* machine, CallFrame &frame, uint8_t *&ip) {
    // Завершение выполнения: вернуть верх стека или NIL
    EvaluationValue result = IS_NIL(machine->peek()) ? NIL() : machine->pop();
    machine->callStack.clear();
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
        throw std::runtime_error("Деление на ноль");
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
                throw std::runtime_error("неизвестная операция сравнения");
        }
        machine->push(BOOLEAN(res));
    };

    if (IS_NUMBER(left) && IS_NUMBER(right)) {
        compare_values(AS_NUMBER(left), AS_NUMBER(right));
    } else if (IS_STRING(left) && IS_STRING(right)) {
        compare_values(AS_CPP_STRING(left), AS_CPP_STRING(right));
    } else {
        throw std::runtime_error("ошибка типов в сравнении");
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


static void handleJumpIfFalseOrPop(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t address = (ip[0] << 8) | ip[1];
    ip += 2;
    auto value = machine->peek();
    if (!machine->isTruth(value)) {
        machine->pop();
        ip = &frame.co->code[address];
    } else {
        machine->pop();
    }
}

static void handleJumpIfTrueOrPop(vm* machine, CallFrame &frame, uint8_t *&ip) {
    uint16_t address = (ip[0] << 8) | ip[1];
    ip += 2;
    auto value = machine->peek();
    if (machine->isTruth(value)) {
        machine->pop();
        ip = &frame.co->code[address];
    } else {
        machine->pop();
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
        throw std::runtime_error("попытка вызова не функции");
    }

    CodeObject* functionCo = AS_CODE(funcVal);

    CallFrame newFrame(functionCo);

    size_t paramIndex = 0;
    for (int slot = 0; slot < static_cast<int>(functionCo->localNames.size()); ++slot) {
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
        machine->push(returnValue);
        return;
    }

    machine->push(returnValue);
}

static void handleArray(vm* machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(ALLOC_ARRAY());
}

static void handleArrayGet(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("попытка индексации не по массиву");
    }
    ArrayObject* array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("индекс массива должен быть числом");
    }

    size_t index = (size_t)AS_NUMBER(indexVal);
    if (index >= array->elements.size()) {
        throw std::runtime_error("индекс больше размера массива");
    }

    machine->push(array->elements[index]);
}

static void handleArraySet(vm* machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->pop();
    EvaluationValue indexVal = machine->pop();
    EvaluationValue arrayVal = machine->pop();

    if (!IS_ARRAY(arrayVal)) {
        throw std::runtime_error("попытка индексации не по массиву");
    }
    ArrayObject* array = AS_ARRAY(arrayVal);

    if (!IS_NUMBER(indexVal)) {
        throw std::runtime_error("индекс массива должен быть числом");
    }

    const auto index = static_cast<size_t>(AS_NUMBER(indexVal));
    if (index >= array->elements.size()) {
        array->elements.resize(index + 1, NIL());
    }
    array->elements[index] = value;
}

static void handleNil(vm* machine, CallFrame &frame, uint8_t *&ip) {
    machine->push(NIL());
}
