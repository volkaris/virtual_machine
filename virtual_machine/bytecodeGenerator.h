//
// Created by Ilya on 06.10.2024.
//
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
        : global(global), disassembler(std::make_unique<Disassembler>(global)), co(nullptr), localCount(0) {
    }

    CodeObject *compile(const Exp &exp) {
        co = AS_CODE(ALLOC_CODE("main"));

        //todo do i actually need it?
        /*locals.clear();*/
        localCount = 0;


        generate(exp);

        //forcefully stop the program
        emit(OP_HALT);
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

                // Look through scopes from innermost to outermost
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
                    // Check global variables
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
                    //todo add support for unary minus and plus operators (++x, --x)
                    throw std::runtime_error("Unknown operator in unary expression.");
                }
                break;
            }


            case ExpType::BINARY_EXP: {

                if (exp.op == "&&") {
                    // Short-circuit evaluation for '&&'        //Short-circuit evaluation is when the second argument is not evaluated if the first argument is false.
                    generate(*exp.left);                        //for binary && ur wont evaluate the right  if the left is false

                    emit(OP_DUP); // Duplicate the value for checking
                    emit(OP_JUMP_IF_FALSE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0); // Placeholder for jump address

                    generate(*exp.right);

                    // Backpatch the jump address
                    size_t afterRight = co->code.size();
                    patchAddress(jumpAddr, afterRight);
                } else if (exp.op == "||") {                //for || ur wont evaluate the right  if the left is true
                    // Short-circuit evaluation for '||'
                    generate(*exp.left);

                    emit(OP_DUP); // Duplicate the value for checking
                    emit(OP_JUMP_IF_TRUE_OR_POP);
                    size_t jumpAddr = co->code.size();
                    emit16(0); // Placeholder for jump address

                    generate(*exp.right);

                    // Backpatch the jump address
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
                        throw std::runtime_error("Unknown operator in binary expression.");
                    }
                }
                break;
            }
            case ExpType::IF_EXP: {
                // Generate code for condition
                generate(*exp.condition);

                // Emit OP_JUMP_IF_FALSE with placeholder address
                emit(OP_JUMP_IF_FALSE);
                size_t jumpIfFalseAddr = co->code.size();

                emit16(0);

                generate(*exp.thenBranch);

                if (exp.elseBranch != nullptr) {
                    // Emit OP_JUMP to skip elseBranch
                    emit(OP_JUMP);
                    size_t jumpAddr = co->code.size();
                    emit16(0);

                    // Backpatch the jumpIfFalse address to point to elseBranch
                    uint16_t elseBranchAddr = co->code.size();
                    patchAddress(jumpIfFalseAddr, elseBranchAddr);

                    // Generate code for elseBranch
                    generate(*exp.elseBranch);

                    // Backpatch the jump address to point after elseBranch
                    uint16_t afterElseAddr = co->code.size();
                    patchAddress(jumpAddr, afterElseAddr);
                } else {
                    // Emit OP_JUMP to skip over OP_NIL when condition is true
                    emit(OP_JUMP);
                    size_t jumpOverNilAddr = co->code.size();
                    emit16(0);

                    // Backpatch the jumpIfFalse address to point to OP_NIL
                    uint16_t nilAddr = co->code.size();
                    patchAddress(jumpIfFalseAddr, nilAddr);

                    // Emit OP_NIL (executed when condition is false)
                    emit(OP_NIL);

                    // Backpatch the jump over OP_NIL to point after OP_NIL
                    uint16_t afterNilAddr = co->code.size();
                    patchAddress(jumpOverNilAddr, afterNilAddr);
                }

                break;
            }
            case ExpType::VAR_DECLARATION: {
                generate(*exp.varValue);

                if (co->name == "main") {
                    // Define the global variable before setting it
                    global->define(exp.varName);

                    // Emit OP_SET_GLOBAL for main scope variables
                    emit(OP_SET_GLOBAL);
                    int globalIdx = global->getGlobalIndex(exp.varName);
                    if (globalIdx == -1) {
                        throw std::runtime_error("Failed to define global variable: " + exp.varName);
                    }
                    emit(static_cast<uint8_t>(globalIdx));
                } else {
                    auto &currentScope = scopeStack.back();

                    if (currentScope.find(exp.varName) != currentScope.end()) {
                        throw std::runtime_error("Variable " + exp.varName + " already exists.");
                    }

                    currentScope[exp.varName] = localCount;
                    co->localNames[localCount] = exp.varName; // Store the variable name
                    localCount++;

                    // Emit OP_SET_LOCAL for function scope variables
                    emit(OP_SET_LOCAL);
                    emit(currentScope[exp.varName]);
                }
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
                generate(*exp.varValue);

                int slot = -1;

                // Look through scopes from innermost to outermost
                for (auto scopeIt = scopeStack.rbegin(); scopeIt != scopeStack.rend(); ++scopeIt) {
                    auto &scope = *scopeIt;
                    if (scope.find(exp.varName) != scope.end()) {
                        slot = scope[exp.varName];
                        break;
                    }
                }

                if (slot == -1) {
                    // Check global variables
                    if (global->exists(exp.varName)) {
                        emit(OP_SET_GLOBAL);
                        emit((uint8_t)global->getGlobalIndex(exp.varName));
                    } else {
                        throw std::runtime_error("Undefined variable: " + exp.varName);
                    }
                } else {
                    if (co->name == "main") {
                        // If in main, treat as global
                        emit(OP_SET_GLOBAL);
                        emit((uint8_t)global->getGlobalIndex(exp.varName));
                    } else {
                        // Else, treat as local
                        emit(OP_SET_LOCAL);
                        emit(slot);
                    }
                }

                break;
            }

            case ExpType::WHILE_EXP : {

                size_t loopStart = co->code.size();

                // Generate code for condition
                generate(*exp.condition);

                // Emit OP_JUMP_IF_FALSE with placeholder address
                emit(OP_JUMP_IF_FALSE);
                size_t exitJumpAddr = co->code.size();
                emit16(0); // Placeholder for exit jump

                // Generate code for loop body
                generate(*exp.whileBody);

                // Emit OP_JUMP to loop start
                emit(OP_JUMP);
                emit16(loopStart);

                // Backpatch the exit jump address to point to the code after the loop
                size_t loopEnd = co->code.size();
                patchAddress(exitJumpAddr, loopEnd);

                break;
            }
            case  ExpType::FOR_EXP : {
                // Generate initialization (if any)
                if (exp.forInit != nullptr) {
                    generate(*exp.forInit);
                }

                size_t loopStart = co->code.size();

                // Generate condition (if any)
                if (exp.forCondition != nullptr) {
                    generate(*exp.forCondition);

                    // Emit OP_JUMP_IF_FALSE with placeholder address
                    emit(OP_JUMP_IF_FALSE);
                    size_t exitJumpAddr = co->code.size();
                    emit16(0); // Placeholder for exit jump

                    // Generate loop body
                    generate(*exp.forBody);

                    // Generate update expression (if any)
                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }

                    // Emit OP_JUMP to loop start
                    emit(OP_JUMP);
                    emit16(loopStart);

                    // Backpatch the exit jump address to point to the code after the loop
                    size_t loopEnd = co->code.size();
                    patchAddress(exitJumpAddr, loopEnd);
                } else {
                    // Infinite loop (condition is empty)
                    // Emit loop body
                    generate(*exp.forBody);

                    // Generate update expression (if any)
                    if (exp.forUpdate != nullptr) {
                        generate(*exp.forUpdate);
                    }

                    // Emit OP_JUMP to loop start
                    emit(OP_JUMP);
                    emit16(loopStart);
                }
                break;
            }
            case ExpType::FUNCTION_DECLARATION: { // CHANGED
                // Define the function globally first, so it can be referenced by name (e.g., recursion)
                global->define(exp.funcName);

                // Create a new CodeObject for this function
                EvaluationValue fnVal = ALLOC_CODE(exp.funcName);
                CodeObject* fnCo = AS_CODE(fnVal);
                fnCo->arity = (int)exp.funcParams.size();

                // Save current CodeObject and scope state
                CodeObject* parentCo = co;
                auto oldScopeStack = scopeStack;

                co = fnCo;
                scopeStack.clear();
                scopeStack.emplace_back(); // new scope for function

                // Assign parameters as locals
                for (int i = 0; i < fnCo->arity; i++) {
                    scopeStack.back()[exp.funcParams[i]] = i;
                    co->localNames[i] = exp.funcParams[i];
                }

                // Generate the function body
                generate(*exp.funcBody);

                // If no explicit return at the end, return NIL
                emit(OP_NIL);
                emit(OP_RETURN);

                // Restore parent code object and scope
                co = parentCo;
                scopeStack = oldScopeStack;

                // Add this function code object to parent constants
                size_t fnIndex = co->constants.size();
                co->constants.push_back(fnVal);

                // Now assign the compiled function object to the global variable
                emit(OP_CONST);
                emit((uint8_t)fnIndex);

                emit(OP_SET_GLOBAL);
                emit((uint8_t)global->getGlobalIndex(exp.funcName));
                break;
            }

            case ExpType::FUNCTION_CALL: { // CHANGED
                // Generate arguments first
                for (auto &arg : exp.callArguments) {
                    generate(*arg);
                }

                // Now load the function
                int slot = -1;
                for (auto scopeIt = scopeStack.rbegin(); scopeIt != scopeStack.rend(); ++scopeIt) {
                    auto &scope = *scopeIt;
                    if (scope.find(exp.funcName) != scope.end()) {
                        slot = scope[exp.funcName];
                        emit(OP_GET_LOCAL);
                        emit(slot);
                        break;
                    }
                }

                if (slot == -1) {
                    // global
                    if (!global->exists(exp.funcName)) {
                        throw std::runtime_error("Undefined function: " + exp.funcName);
                    }
                    emit(OP_GET_GLOBAL);
                    emit(global->getGlobalIndex(exp.funcName));
                }

                // Emit call with number of arguments
                emit(OP_CALL);
                emit((uint8_t)exp.callArguments.size());
                break;
            }

            case ExpType::RETURN_STATEMENT: { // CHANGED
                if (exp.returnValue) {
                    generate(*exp.returnValue);
                } else {
                    emit(OP_NIL);
                }
                emit(OP_RETURN);
                break;
            }


        }
    }

    void disassembleBytecode() { disassembler->disassemble(co); }

private:
    //Global object
    std::shared_ptr<Global> global;

    std::unique_ptr<Disassembler> disassembler;

    // compiling code object
    CodeObject *co;

    std::vector<std::unordered_map<std::string, int> > scopeStack;
    /*std::unordered_map<std::string, int> locals;*/

    int localCount; // Tracks the number of local variables

    size_t getOffset() { return co->code.size(); }

    size_t numericConstIdx(double value) {
        for (auto i = 0; i < co->constants.size(); ++i) {
            if (!IS_NUMBER(co->constants[i])) {
                continue;
            }

            if (AS_NUMBER(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(NUMBER(value));
        return co->constants.size() - 1;
    }

    size_t booleanConstIdx(bool value) {
        for (auto i = 0; i < co->constants.size(); ++i) {
            if (!IS_BOOL(co->constants[i])) {
                continue;
            }

            if (AS_BOOL(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(BOOLEAN(value));
        return co->constants.size() - 1;
    }

    size_t stringConstIdx(const std::string &value) {
        for (auto i = 0; i < co->constants.size(); ++i) {
            if (!IS_STRING(co->constants[i])) {
                continue;
            }

            if (AS_CPP_STRING(co->constants[i]) == value) {
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
        emit((value >> 8) & 0xFF);
        emit(value & 0xFF);
    }

    void patchAddress(size_t addrPos, uint16_t value) {
        co->code[addrPos] = (value >> 8) & 0xFF;
        co->code[addrPos + 1] = value & 0xFF;
    }

    static std::map<std::string, uint8_t> compareOperator;
};

std::map<std::string, uint8_t> bytecodeGenerator::compareOperator = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};
