#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <stdexcept>
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
        functions.clear();
        functionIndexMap.clear();

        co = AS_CODE(ALLOC_CODE("main"));
        localCount = 0;
        scopeStack.clear();
        scopeStack.emplace_back();

        generate(exp);

        //forcefully stop the program
        emit(OP_HALT);

        // Register main as function 0 if needed
        // But main is just the top-level code object
        functions.push_back(co);

        return co;
    }

    void disassembleBytecode() { disassembler->disassemble(co); }

    // After compilation, you can access functions:
    std::vector<CodeObject*> getAllFunctions() {
        return functions;
    }

private:
    std::shared_ptr<Global> global;
    std::unique_ptr<Disassembler> disassembler;

    CodeObject *co;
    std::vector<std::unordered_map<std::string, int> > scopeStack;
    int localCount = 0;

    // For function handling
    std::unordered_map<std::string, int> functionIndexMap;
    std::vector<CodeObject*> functions;

    static std::map<std::string, uint8_t> compareOperator;

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

                {
                    int slot = -1;
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
                }
                break;
            }

            case ExpType::UNARY_EXP: {
                generate(*exp.operand);
                if (exp.unaryOp == "!") {
                    emit(OP_LOGICAL_NOT);
                } else {
                    throw std::runtime_error("Unknown unary operator.");
                }
                break;
            }

            case ExpType::BINARY_EXP: {
                if (exp.op == "&&") {
                    generate(*exp.left);
                    emit(OP_DUP);
                    emit(OP_JUMP_IF_FALSE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    generate(*exp.right);

                    size_t afterRight = co->code.size();
                    patchAddress(jumpAddr, (uint16_t)afterRight);
                } else if (exp.op == "||") {
                    generate(*exp.left);
                    emit(OP_DUP);
                    emit(OP_JUMP_IF_TRUE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    generate(*exp.right);

                    size_t afterRight = co->code.size();
                    patchAddress(jumpAddr, (uint16_t)afterRight);
                } else {
                    generate(*exp.left);
                    generate(*exp.right);

                    if (exp.op == "+") emit(OP_ADD);
                    else if (exp.op == "-") emit(OP_SUB);
                    else if (exp.op == "*") emit(OP_MUL);
                    else if (exp.op == "/") emit(OP_DIV);
                    else if (compareOperator.count(exp.op) != 0) {
                        emit(OP_COMPARE);
                        emit(compareOperator[exp.op]);
                    } else {
                        throw std::runtime_error("Unknown binary operator: " + exp.op);
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
                    emit(OP_JUMP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    uint16_t elseBranchAddr = (uint16_t)co->code.size();
                    patchAddress(jumpIfFalseAddr, elseBranchAddr);

                    generate(*exp.elseBranch);

                    uint16_t afterElseAddr = (uint16_t)co->code.size();
                    patchAddress(jumpAddr, afterElseAddr);
                } else {
                    emit(OP_JUMP);
                    size_t jumpOverNilAddr = co->code.size();
                    emit16(0);

                    uint16_t nilAddr = (uint16_t)co->code.size();
                    patchAddress(jumpIfFalseAddr, nilAddr);

                    emit(OP_NIL);

                    uint16_t afterNilAddr = (uint16_t)co->code.size();
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
                int oldLocalCount = localCount;
                for (auto &stmt: exp.statements) {
                    generate(*stmt);
                }
                scopeStack.pop_back();
                localCount = oldLocalCount;
                break;
            }

            case ExpType::ASSIGNMENT: {
                generate(*exp.varValue);
                {
                    int slot = -1;
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
                        if (global->exists(exp.varName)) {
                            emit(OP_SET_GLOBAL);
                            emit(global->getGlobalIndex(exp.varName));
                        } else {
                            throw std::runtime_error("Undefined variable: " + exp.varName);
                        }
                    }
                }
                break;
            }

            case ExpType::WHILE_EXP: {
                size_t loopStart = co->code.size();
                generate(*exp.condition);
                emit(OP_JUMP_IF_FALSE);
                size_t exitJumpAddr = co->code.size();
                emit16(0);

                generate(*exp.whileBody);

                emit(OP_JUMP);
                emit16((uint16_t)loopStart);

                size_t loopEnd = co->code.size();
                patchAddress(exitJumpAddr, (uint16_t)loopEnd);
                break;
            }

            case ExpType::FOR_EXP: {
                // init
                if (exp.forInit != nullptr) {
                    generate(*exp.forInit);
                }

                size_t loopStart = co->code.size();

                if (exp.forCondition != nullptr) {
                    generate(*exp.forCondition);
                    emit(OP_JUMP_IF_FALSE);
                    size_t exitJumpAddr = co->code.size();
                    emit16(0);

                    generate(*exp.forBody);

                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }

                    emit(OP_JUMP);
                    emit16((uint16_t)loopStart);

                    size_t loopEnd = co->code.size();
                    patchAddress(exitJumpAddr, (uint16_t)loopEnd);
                } else {
                    // infinite loop
                    generate(*exp.forBody);
                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }
                    emit(OP_JUMP);
                    emit16((uint16_t)loopStart);
                }
                break;
            }

            case ExpType::FUNCTION_DECLARATION: {
                // Create a new CodeObject for the function
                {
                    auto funcCoValue = ALLOC_CODE(exp.funcName);
                    CodeObject *funcCo = AS_CODE(funcCoValue);

                    // Save current co
                    CodeObject *oldCo = co;
                    int oldLocalCount = localCount;

                    co = funcCo;
                    localCount = 0;
                    scopeStack.emplace_back();

                    // Assign parameters as locals
                    for (auto &param : exp.funcParams) {
                        scopeStack.back()[param] = localCount;
                        co->localNames[localCount] = param;
                        localCount++;
                    }

                    // generate body
                    generate(*exp.funcBody);

                    // ensure return at end
                    emit(OP_NIL);
                    emit(OP_RETURN);

                    scopeStack.pop_back();
                    localCount = oldLocalCount;

                    co = oldCo;

                    // register function
                    int funcIdx = (int)functions.size();
                    functions.push_back(funcCo);
                    functionIndexMap[exp.funcName] = funcIdx;
                }
                break;
            }

            case ExpType::FUNCTION_CALL: {
                // push arguments
                for (auto &arg: exp.callArguments) {
                    generate(*arg);
                }

                auto it = functionIndexMap.find(exp.funcName);
                if (it == functionIndexMap.end()) {
                    throw std::runtime_error("Undefined function: " + exp.funcName);
                }
                int funcIdx = it->second;

                emit(OP_CALL);
                emit((uint8_t)funcIdx);
                emit((uint8_t)exp.callArguments.size());

                break;
            }
        }
    }

    size_t numericConstIdx(double value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_NUMBER(co->constants[i]) && AS_NUMBER(co->constants[i]) == value)
                return i;
        }
        co->constants.emplace_back(NUMBER(value));
        return co->constants.size() - 1;
    }

    size_t booleanConstIdx(bool value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_BOOL(co->constants[i]) && AS_BOOL(co->constants[i]) == value)
                return i;
        }
        co->constants.emplace_back(BOOLEAN(value));
        return co->constants.size() - 1;
    }

    size_t stringConstIdx(const std::string &value) {
        for (size_t i = 0; i < co->constants.size(); ++i) {
            if (IS_STRING(co->constants[i]) && AS_CPP_STRING(co->constants[i]) == value)
                return i;
        }
        co->constants.emplace_back(ALLOC_STRING(value));
        return co->constants.size() - 1;
    }

    void emit(uint8_t code) {
        co->code.emplace_back(code);
    }

    void emit16(uint16_t value) {
        emit((value >> 8) & 0xFF);
        emit(value & 0xFF);
    }

    void patchAddress(size_t addrPos, uint16_t value) {
        co->code[addrPos] = (value >> 8) & 0xFF;
        co->code[addrPos + 1] = value & 0xFF;
    }
};

std::map<std::string, uint8_t> bytecodeGenerator::compareOperator = {
        {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};
