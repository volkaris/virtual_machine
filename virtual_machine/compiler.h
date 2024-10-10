//
// Created by Ilya on 06.10.2024.
//

#ifndef VIRTUAL_MACHINE_COMPILER_H
#define VIRTUAL_MACHINE_COMPILER_H


#include "parser.h"
#include "EvaluationValue.h"
#include "OpCode.h"

class compiler {
public :
    explicit compiler ()
        : co(nullptr) {
    }

    CodeObject* compile (const Exp& exp) {
        co = AS_CODE(ALLOC_CODE("main"));

        generate(exp);

        //forcefully stop the program
        emit(OP_HALT);
        return co;
    }

    void generate (const Exp& exp) {
        switch (exp.type) {
        case ExpType::NUMBER:
            emit(OP_CONST);
            emit(numericConstIdx(exp.number));
            break;

        case ExpType::STRING:
            emit(OP_CONST);
            emit(stringConstIdx(exp.string));
            break;

        case ExpType::SYMBOL:
            if (exp.symbol == "true" or exp.symbol == "false") {
                emit(OP_CONST);
                emit(booleanConstIdx(exp.symbol == "true") ? true : false);
                break;
            }
            else {
                //variables
                throw std::runtime_error("unknown symbol");
            }


        case ExpType::BINARY_EXP:

            generate(*exp.left);
            generate(*exp.right);

            if (exp.op == "+") {
                emit(OP_ADD);
            }
            else if (exp.op == "-") {
                emit(OP_SUB);
            }
            else if (exp.op == "*") {
                emit(OP_MUL);
            }
            else if (exp.op == "/") {
                emit(OP_DIV);
            }
            else if (compareOperator.count(exp.op) != 0) {
                emit(OP_COMPARE);
                emit(compareOperator[exp.op]);
            }
            else {
                throw std::runtime_error("Unknown operator in binary expression.");
            }
            break;
        /*break;*/

        case ExpType::IF_EXP:
            // Generate code for condition
            generate(*exp.condition);

        // Emit OP_JUMP_IF_FALSE with placeholder address
            emit(OP_JUMP_IF_FALSE);
            size_t jumpIfFalseAddr = co->code.size();

            emit16(0);

            generate(*exp.thenBranch);

           if (exp.elseBranch!=nullptr) {
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
           }

           else {
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

        /*size_t jumpIfFalseAddr = co->code.size();

        emit(0);
        emit(0);

        // Generate code for thenBranch
        generate(*exp.thenBranch);

        emit(OP_JUMP);

        auto jumpIf=co->code.size();

        emit(0);
        emit(0);*/
        }
    }

private
:
    size_t numericConstIdx (double value) {
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

    size_t booleanConstIdx (bool value) {
        for (auto i = 0; i < co->constants.size(); ++i) {
            if (!IS_BOOLEAN(co->constants[i])) {
                continue;
            }

            if (AS_BOOLEAN(co->constants[i]) == value) {
                return i;
            }
        }
        co->constants.emplace_back(BOOLEAN(value));
        return co->constants.size() - 1;
    }

    size_t stringConstIdx (const std::string& value) {
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

    void emit (uint8_t code) {
        co->code.emplace_back(code);
    }

    void emit16 (uint16_t value) {
        emit((value >> 8) & 0xFF);
        emit(value & 0xFF);
    }

    void patchAddress (size_t addrPos, uint16_t value) {
        co->code[addrPos] = (value >> 8) & 0xFF;
        co->code[addrPos + 1] = value & 0xFF;
    }

    // compiling code object
    CodeObject* co;

    static std::map<std::string, uint8_t> compareOperator;
};

std::map<std::string, uint8_t> compiler::compareOperator = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};

#endif //VIRTUAL_MACHINE_COMPILER_H
