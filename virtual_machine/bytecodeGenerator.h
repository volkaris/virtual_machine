#pragma once

#include <unordered_map>
#include "parser.h"
#include "EvaluationValue.h"
#include "OpCode.h"
#include "Global.h"
#include "disassembler/Disassembler.h"

class bytecodeGenerator {
public :
    explicit bytecodeGenerator(const std::shared_ptr<Global> &global)
            : global(global), disassembler(std::make_unique<Disassembler>(global)), co(nullptr) {
    }

    CodeObject *compile(const Exp &exp) {
        co = AS_CODE(ALLOC_CODE("main"));


        localCount = 0;

        generate(exp);


        emit(OP_HALT);


        // optimizeBytecode(co);

        return co;
    }

    void generate(const Exp &exp) {
        switch (exp.type) {
            case ExpType::NUMBER: {
                emit(OP_CONST);
                emit(numericConstIdx(exp.number));
                break;
            }

            case ExpType::STRING: {
                emit(OP_CONST);
                emit(stringConstIdx(exp.string));
                break;
            }

            case ExpType::SYMBOL: {

                if (exp.string == "true" || exp.string == "false") {
                    emit(OP_CONST);
                    emit(booleanConstIdx(exp.string == "true"));
                    break;
                }

                int slot = -1;

                // Поиск переменной во внутренних областях видимости (от внутренней к внешней)
                for (auto scopeIt = scopeStack.rbegin(); scopeIt != scopeStack.rend(); ++scopeIt) {
                    auto &scope = *scopeIt;
                    if (scope.find(exp.string) != scope.end()) {
                        slot = scope[exp.string];
                        emit(OP_GET_LOCAL);
                        emit(slot);
                        break;
                    }
                }

                if (slot == -1) {
                    if (global->exists(exp.string)) {
                        emit(OP_GET_GLOBAL);
                        emit(global->getGlobalIndex(exp.string));
                    } else {
                        throw std::runtime_error("Undefined variable: " + exp.string);
                    }
                }

                break;
            }

            case ExpType::UNARY_EXP: {
                generate(*exp.operand);

                if (exp.unaryOp == "!") {
                    emit(OP_LOGICAL_NOT);
                } else {
                    throw std::runtime_error("Неизвестный оператор в бинарном выражении");
                }
                break;
            }

            case ExpType::BINARY_EXP: {
                // Логические операции && и || с коротким замыканием
                if (exp.op == "&&") {
                    // Для &&: если левая часть ложна, правая не вычисляется
                    generate(*exp.left);

                    emit(OP_DUP);
                    emit(OP_JUMP_IF_FALSE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0); // Заглушка адреса

                    generate(*exp.right);


                    size_t afterRight = co->code.size();
                    patchAddress(jumpAddr, afterRight);
                } else if (exp.op == "||") {
                    // Для ||: если левая часть истинна, правая не вычисляется
                    generate(*exp.left);

                    emit(OP_DUP);
                    emit(OP_JUMP_IF_TRUE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    generate(*exp.right);


                    size_t afterRight = co->code.size();
                    patchAddress(jumpAddr, afterRight);
                } else {

                    generate(*exp.left);
                    generate(*exp.right);

                    if (exp.op == "+") {
                        emit(OP_ADD);
                    } else if (exp.op == "-") {
                        emit(OP_SUB);
                    } else if (exp.op == "*") {
                        emit(OP_MUL);
                    } else if (exp.op == "/") {
                        emit(OP_DIV);
                    } else if (compareOperator.count(exp.op) != 0) {
                        emit(OP_COMPARE);
                        emit(compareOperator[exp.op]);
                    } else {
                        throw std::runtime_error("Неизвестный оператор в бинарном выражении");
                    }
                }
                break;
            }

            case ExpType::IF_EXP: {

                generate(*exp.condition);


                emit(OP_JUMP_IF_FALSE);
                size_t jumpIfFalseAddr = co->code.size();
                emit16(0);

                generate(*exp.thenBranch);

                if (exp.elseBranch != nullptr) {
                    // Если есть else-ветка, вставляем OP_JUMP для пропуска else при истинном условии
                    emit(OP_JUMP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    // Обратная замена адреса на начало else
                    uint16_t elseBranchAddr = co->code.size();
                    patchAddress(jumpIfFalseAddr, elseBranchAddr);

                    generate(*exp.elseBranch);

                    // Обратная замена адреса после else
                    uint16_t afterElseAddr = co->code.size();
                    patchAddress(jumpAddr, afterElseAddr);
                } else {
                    // Если нет else-ветки, пропускаем место для nil
                    emit(OP_JUMP);
                    size_t jumpOverNilAddr = co->code.size();
                    emit16(0);

                    // Обратная замена адреса на место вставки NIL
                    uint16_t nilAddr = co->code.size();
                    patchAddress(jumpIfFalseAddr, nilAddr);

                    // Вставка NIL при ложном условии
                    emit(OP_NIL);

                    // Обратная замена адреса прыжка, чтобы пропустить NIL при истинном условии
                    uint16_t afterNilAddr = co->code.size();
                    patchAddress(jumpOverNilAddr, afterNilAddr);
                }

                break;
            }

            case ExpType::VAR_DECLARATION: {

                generate(*exp.varValue);

                auto &currentScope = scopeStack.back();

                if (currentScope.find(exp.varName) != currentScope.end()) {
                    throw std::runtime_error("Variable " + exp.varName + " already exists.");
                }

                currentScope[exp.varName] = localCount;
                co->localNames[localCount] = exp.varName;
                localCount++;

                emit(OP_SET_LOCAL);
                emit(currentScope[exp.varName]);
                break;
            }

            case ExpType::BLOCK: {

                scopeStack.emplace_back();

                for (const auto &stmt: exp.statements) {
                    generate(*stmt);
                }


                scopeStack.pop_back();
                break;
            }

            case ExpType::ASSIGNMENT: {

                if (exp.arrayValue != nullptr) {
                    // Присваивание элементу массива: arr[j] = arr[j+1]
                    Exp arrayNameExp(exp.varName);
                    generate(arrayNameExp);

                    generate(*exp.varValue);
                    generate(*exp.arrayValue);

                    emit(OP_ARRAY_SET);
                } else {
                    generate(*exp.varValue);

                    int slot = -1;
                    // Поиск переменной в областях видимости
                    for (auto scopeIt = scopeStack.rbegin(); scopeIt != scopeStack.rend(); ++scopeIt) {
                        auto &scope = *scopeIt;
                        if (scope.find(exp.varName) != scope.end()) {
                            slot = scope[exp.varName];
                            emit(OP_SET_LOCAL);
                            emit(slot);
                            break;
                        }
                    }

                    if (slot == -1) {
                        // Проверка глобальных
                        if (global->exists(exp.varName)) {
                            emit(OP_SET_GLOBAL);
                            emit(global->getGlobalIndex(exp.varName));
                        } else {
                            throw std::runtime_error("Неизвестная переменная " + exp.varName);
                        }
                    }
                }

                break;
            }

            case ExpType::WHILE_EXP : {
                size_t loopStart = co->code.size();


                generate(*exp.condition);

                // Вставка OP_JUMP_IF_FALSE для выхода из цикла
                emit(OP_JUMP_IF_FALSE);
                size_t exitJumpAddr = co->code.size();
                emit16(0);


                generate(*exp.whileBody);

                // Прыжок в начало цикла
                emit(OP_JUMP);
                emit16((uint16_t) loopStart);

                // Обратная замена адреса выхода из цикла
                size_t loopEnd = co->code.size();
                patchAddress(exitJumpAddr, loopEnd);

                break;
            }

            case ExpType::FOR_EXP : {
                // Инициализация (если есть)
                if (exp.forInit != nullptr) {
                    generate(*exp.forInit);
                }

                size_t loopStart = co->code.size();

                // Условие (если есть)
                if (exp.forCondition != nullptr) {
                    generate(*exp.forCondition);

                    // Если условие ложно - выход
                    emit(OP_JUMP_IF_FALSE);
                    size_t exitJumpAddr = co->code.size();
                    emit16(0);

                    // Тело цикла
                    generate(*exp.forBody);

                    // Обновление (если есть)
                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }

                    // Прыжок в начало
                    emit(OP_JUMP);
                    emit16((uint16_t) loopStart);

                    // Обратная замена адреса выхода
                    size_t loopEnd = co->code.size();
                    patchAddress(exitJumpAddr, loopEnd);
                } else {
                    // Бесконечный цикл (нет условия)
                    generate(*exp.forBody);

                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }

                    emit(OP_JUMP);
                    emit16(static_cast<uint16_t>(loopStart));
                }
                break;
            }

            case ExpType::FUNCTION_DECLARATION: {

                // Создание нового CodeObject для функции
                std::string functionName = exp.funcName;
                std::vector<std::string> params = exp.funcParams;
                std::shared_ptr<Exp> body = exp.funcBody;

                CodeObject *functionCo = AS_CODE(ALLOC_CODE(functionName));


                size_t functionConstIdx = co->constants.size();
                co->constants.emplace_back(ALLOC_CODE_OBJECT(functionCo));

                // Вставка OP_CONST с индексом функции
                emit(OP_CONST);
                emit((uint8_t) functionConstIdx);


                if (!global->exists(functionName)) {
                    global->define(functionName);
                }
                int globalIdx = global->getGlobalIndex(functionName);

                // OP_SET_GLOBAL для присвоения имени функции
                emit(OP_SET_GLOBAL);
                emit((uint8_t) globalIdx);

                // Переключение на functionCo
                CodeObject *previousCo = co;
                co = functionCo;

                // Новая область видимости для функции
                scopeStack.emplace_back();
                localCount = 0;

                // Параметры как локальные переменные
                for (const auto &param: params) {
                    scopeStack.back()[param] = localCount;
                    co->localNames[localCount] = param;
                    localCount++;
                }

                // Генерация тела функции
                generate(*body);

                // Убедиться, что функция заканчивается return
                emit(OP_RETURN);

                // Восстановление предыдущего CodeObject и области
                co = previousCo;
                scopeStack.pop_back();

                break;
            }

            case ExpType::FUNCTION_CALL: {

                std::string functionName = exp.funcName;
                int functionIdx = global->getGlobalIndex(functionName);
                if (functionIdx == -1) {
                    throw std::runtime_error("Undefined function: " + functionName);
                }

                // Загрузка функции
                emit(OP_GET_GLOBAL);
                emit((uint8_t) functionIdx);

                // Генерация аргументов
                for (const auto &arg: exp.callArguments) {
                    generate(*arg);
                }


                emit(OP_CALL);
                emit((uint8_t) exp.callArguments.size());

                break;
            }

            case ExpType::RETURN_STATEMENT: {
                if (exp.returnValue != nullptr) {
                    generate(*exp.returnValue);
                } else {
                    // Если нет значения, вернуть NIL
                    emit(OP_NIL);
                }

                emit(OP_RETURN);
                break;
            }

            case ExpType::ARRAY_LITERAL: {

                emit(OP_ARRAY);


                for (size_t i = 0; i < exp.callArguments.size(); ++i) {
                    emit(OP_DUP);

                    emit(OP_CONST);
                    emit(numericConstIdx((double) i));

                    generate(*exp.callArguments[i]);

                    emit(OP_ARRAY_SET);
                }
                break;
            }

            case ExpType::ARRAY_ACCESS: {
                // Доступ к элементу массива: arr[i]
                Exp arrayNameExp(exp.varName);
                generate(arrayNameExp);
                generate(*exp.varValue);
                emit(OP_ARRAY_GET);
                break;
            }

                /*case ExpType::PRINT_STATEMENT: {

                    generate(*exp.varValue);


                    emit(OP_PRINT);
                    break;
                }*/



        }
    }

    void disassembleBytecode() { disassembler->disassemble(co); }

    CodeObject *optimizeBytecode(CodeObject *originalCo) {
        CodeObject *optimizedCo = new CodeObject(originalCo->name + "_optimized");
        optimizedCo->constants = originalCo->constants;
        optimizedCo->code = originalCo->code;
        optimizedCo->localNames = originalCo->localNames;

//        std::cout << "before optimization:\n";
//        disassembler->disassemble(originalCo);

        // Применяем оптимизации
        eliminateUnreachableCode(optimizedCo);
        eliminateRedundantLoadsAndStores(optimizedCo);

//        std::cout << "after optimization:\n";
//        disassembler->disassemble(optimizedCo);

        return optimizedCo;
    }


private:
    std::shared_ptr<Global> global;

    std::unique_ptr<Disassembler> disassembler;

    CodeObject *co;

    std::vector<std::unordered_map<std::string, int> > scopeStack;
    int localCount = 0;

    size_t getOffset() { return co->code.size(); }

    size_t numericConstIdx(double value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_NUMBER(co->constants[i]) && AS_NUMBER(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(NUMBER(value));
        return co->constants.size() - 1;
    }

    size_t booleanConstIdx(bool value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_BOOL(co->constants[i]) && AS_BOOL(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(BOOLEAN(value));
        return co->constants.size() - 1;
    }

    size_t stringConstIdx(const std::string &value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_STRING(co->constants[i]) && AS_CPP_STRING(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(ALLOC_STRING(value));
        return co->constants.size() - 1;
    }

    void emit(uint8_t code) {
        co->code.emplace_back(code);
    }

    void emit16(uint16_t value) {
        emit((uint8_t) ((value >> 8) & 0xFF));
        emit((uint8_t) (value & 0xFF));
    }

    void patchAddress(size_t addrPos, uint16_t value) {
        co->code[addrPos] = (uint8_t) ((value >> 8) & 0xFF);
        co->code[addrPos + 1] = (uint8_t) (value & 0xFF);
    }

    inline EvaluationValue ALLOC_CODE_OBJECT(CodeObject *codeObject) {
        EvaluationValue val;
        val.type = EvaluationValueType::OBJECT;
        val.value = (Object *) codeObject;
        return val;
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

    size_t numericConstIdxInFunction(CodeObject *co, double value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_NUMBER(co->constants[i]) && AS_NUMBER(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.push_back(NUMBER(value));
        return co->constants.size() - 1;
    }

    static std::map<std::string, uint8_t> compareOperator;
};

std::map<std::string, uint8_t> bytecodeGenerator::compareOperator = {
        {"<",  0},
        {">",  1},
        {"==", 2},
        {">=", 3},
        {"<=", 4},
        {"!=", 5},
};
