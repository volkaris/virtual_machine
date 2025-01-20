#pragma once

#include <vector>
#include <string>
#include <memory>
#include "parser.h"
#include "EvaluationValue.h"
#include "bytecodeGenerator.h"
#include "Global.h"


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
    CodeObject *co; // Pointer to the function's CodeObject
    uint8_t *ip; // Instruction pointer within the function's bytecode
    std::vector<EvaluationValue> locals; // Local variables for the function

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
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)), disassembler(std::make_unique<Disassembler>(global)) {
        setGlobalVariables();
        initializeBuiltins();
    }

    EvaluationValue exec(const std::string &program) {
        stack.clear();

        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);


        for (const auto &builtin: global->builtinFunctions) {
            defineBuiltin(builtin.first);
        }


        callStack.emplace_back(co);


        CallFrame &currentFrame = callStack.back();
        currentFrame.ip = currentFrame.co->code.data();


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

    CodeObject *optimizeBytecode(CodeObject *originalCo) {
        CodeObject *optimizedCo = new CodeObject(originalCo->name + "_optimized");
        optimizedCo->constants = originalCo->constants;
        optimizedCo->code = originalCo->code;
        optimizedCo->localNames = originalCo->localNames;

        /*std::cout << "before optimization:\n";
        disassembler->disassemble(originalCo);*/


        eliminateUnreachableCode(optimizedCo);
        eliminateRedundantLoadsAndStores(optimizedCo);

        /*std::cout << "after optimization:\n";
        disassembler->disassemble(optimizedCo);*/

        return optimizedCo;
    }

    void eliminateUnreachableCode(CodeObject *co) {
        std::vector<uint8_t> &code = co->code;
        std::vector<uint8_t> optimizedCode;
        size_t i = 0;

        while (i < code.size()) {
            uint8_t opcode = code[i];
            optimizedCode.emplace_back(opcode);
            size_t instrLen = 1;

            // Определяем длину текущей инструкции
            switch (opcode) {
                case OP_JUMP:
                case OP_JUMP_IF_FALSE:
                case OP_JUMP_IF_FALSE_OR_POP:
                case OP_JUMP_IF_TRUE_OR_POP:
                    if (i + 2 >= code.size()) {
                        throw std::runtime_error("Invalid jump instruction length in eliminateUnreachableCode.");
                    }
                    instrLen = 3;
                    for (int j = 1; j < instrLen; ++j) {
                        optimizedCode.emplace_back(code[i + j]);
                    }
                    break;
                case OP_CONST:
                case OP_GET_LOCAL:
                case OP_SET_LOCAL:
                case OP_GET_GLOBAL:
                case OP_SET_GLOBAL:
                case OP_LOGICAL_NOT:
                case OP_DUP:
                case OP_CALL:
                    instrLen = 2;
                    if (i + 1 >= code.size()) {
                        throw std::runtime_error("Invalid instruction length in eliminateUnreachableCode.");
                    }
                    optimizedCode.emplace_back(code[i + 1]);
                    break;
                case OP_ADD:
                case OP_SUB:
                case OP_MUL:
                case OP_DIV:
                case OP_COMPARE:
                case OP_ARRAY:
                case OP_ARRAY_GET:
                case OP_ARRAY_SET:
                case OP_NIL:
                case OP_HALT:
                case OP_RETURN:
                    instrLen = 1;
                    break;
                default:
                    throw std::runtime_error(
                        "Unknown opcode encountered in eliminateUnreachableCode: " + std::to_string(opcode));
            }

            // Проверяем, является ли текущая инструкция завершением выполнения
            if (opcode == OP_RETURN || opcode == OP_HALT) {
                // Любой код после этой инструкции считается недостижимым
                break;
            }

            i += instrLen;
        }

        // Устанавливаем оптимизированный байткод
        co->code = optimizedCode;
    }


    void eliminateRedundantLoadsAndStores(CodeObject *co) {
        // удаление избыточных загрузок/сохранений
        std::vector<uint8_t> &code = co->code;
        bool changed = true;

        while (changed) {
            changed = false;
            for (size_t i = 0; i + 2 < code.size(); i++) {
                if (code[i] == OP_GET_LOCAL && code[i + 2] == OP_SET_LOCAL) {
                    uint8_t local1 = code[i + 1];
                    uint8_t local2 = code[i + 3];
                    if (local1 == local2) {
                        // Если сразу после GET_LOCAL x идёт SET_LOCAL x без использования значения,
                        // убираем GET_LOCAL
                        code.erase(code.begin() + i, code.begin() + i + 2);
                        changed = true;
                        break;
                    }
                }
            }
        }
    }


    std::vector<EvaluationValue> locals;


    std::shared_ptr<Global> global;


    std::vector<EvaluationValue> stack;

    std::unique_ptr<syntax::parser> _parser;

    CodeObject *co;

    std::vector<CallFrame> callStack;

    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;

    std::unordered_map<std::string, CodeObject *> builtins;

    std::unordered_map<CodeObject *, CodeObject *> jitCache;

    std::unique_ptr<Disassembler> disassembler;
};


static void handleHalt(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue result = IS_NIL(machine->peek()) ? NIL() : machine->pop();

    machine->callStack.clear();

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


static void handleJumpIfFalseOrPop(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleJumpIfTrueOrPop(vm *machine, CallFrame &frame, uint8_t *&ip) {
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

static void handleDup(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue value = machine->peek();
    machine->push(value);
}

static void handleCall(vm *machine, CallFrame &frame, uint8_t *&ip) {
    uint8_t argCount = *ip++;

    // 1) Считываем аргументы со стека
    std::vector<EvaluationValue> args(argCount);
    for (int i = argCount - 1; i >= 0; --i) {
        args[i] = machine->pop();
    }

    // 2) Снимаем со стека саму функцию
    EvaluationValue funcVal = machine->pop();
    if (!IS_OBJECT(funcVal) || !IS_CODE(funcVal)) {
        throw std::runtime_error("Attempting to call a non-function.");
    }
    CodeObject *originalFunctionCo = AS_CODE(funcVal);

    // 3) Проверяем, не является ли это встроенной функцией
    bool isBuiltin = false;
    std::string builtinName;
    for (const auto &[name, codeObj]: machine->builtins) {
        if (name == originalFunctionCo->name) {
            isBuiltin = true;
            builtinName = name;
            break;
        }
    }

    CodeObject *functionCo = originalFunctionCo;

    if (!isBuiltin) {
        // 4b) Если НЕ встроенная функция, проверяем, оптимизировалась ли она ранее
        auto it = machine->jitCache.find(originalFunctionCo);
        if (it != machine->jitCache.end()) {
            // Есть в кэше -> берем оптимизированный код
            functionCo = it->second;
        } else {
            // Нет в кэше -> оптимизируем и добавляем в кэш
            CodeObject *optimizedCo = machine->optimizeBytecode(originalFunctionCo);
            machine->jitCache[originalFunctionCo] = optimizedCo;
            functionCo = optimizedCo;
        }
    }

    if (isBuiltin) {
        // 4a) Если встроенная, вызываем её, никакой JIT-оптимизации не нужно
        EvaluationValue result = machine->global->callBuiltin(builtinName, args);
        machine->push(result);
    } else {
        // 5) Создаём фрейм вызова пользовательской функции
        CallFrame newFrame(functionCo);

        // 6) Установка локальных переменных (параметров)
        for (size_t i = 0; i < args.size(); ++i) {
            int slot = i;
            if (slot >= functionCo->localNames.size()) {
                throw std::runtime_error("Слишком много аргументов при вызове функции.");
            }
            newFrame.locals[slot] = args[i];
        }

        // 7) Добавляем фрейм в стек вызовов
        machine->callStack.push_back(newFrame);
    }
}


static void handleReturn(vm *machine, CallFrame &frame, uint8_t *&ip) {
    EvaluationValue returnValue = machine->pop();
    machine->callStack.pop_back();

    if (machine->callStack.empty()) {
        machine->push(returnValue);
        return;
    }


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
