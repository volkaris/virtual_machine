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

    CodeObject* compile (const Exp& exp) {

    co=AS_CODE(ALLOC_CODE("main"));

    generate(exp);

    //forcefully stop the program
    emit(OP_HALT);
    return co;
}

    void generate(const Exp &exp) {
        switch (exp.type) {
            case ExpType::NUMBER :
                emit(OP_CONST);
                emit(numericConstIdx(exp.number));
                break;
            case ExpType::STRING :
                emit(OP_CONST);
                emit(stringConstIdx(exp.string));
                break;

            case ExpType::SYMBOL :
                throw std::exception();
                /*break;*/

            case ExpType::BINARY_EXP :
                throw std::exception();
                /*break;*/
        }
    }

private:
    size_t numericConstIdx(double value) {
    for (auto i=0; i<co->constants.size();++i){
        if (!IS_NUMBER(co->constants[i])) {
            continue;
        }

        if (AS_NUMBER(co->constants[i])==value){
            return i;
        }
        co->constants.emplace_back(NUMBER(value));
        return co->constants.size()-1;
    }
}

        size_t stringConstIdx(const std::string& value) {
            for (auto i=0; i<co->constants.size();++i){
                if (!IS_STRING(co->constants[i])) {
                    continue;
                }

                if (AS_CPP_STRING(co->constants[i])==value){
                    return i;
                }
                co->constants.emplace_back(ALLOC_STRING(value));
                return co->constants.size()-1;
            }
}

void emit (uint8_t code) {
    co->code.emplace_back(code);
}
    // compiling code object
CodeObject* co;
};


#endif //VIRTUAL_MACHINE_COMPILER_H
