#pragma once
#include <vector>
#include <string>
#include "OpCode.h"
#include <array>
#include "parser.h"
using namespace syntax;
//#define READ_BYTE() *ip++
#define STACK_LIMIT 512
//#define GET_CONST()  constants[READ_BYTE()]

#include "Logger.h"
#include "EvaluationValue.h"
#include "compiler.h"

class Vm {
public:

	Vm()	 : _parser(std::make_unique<parser>()) , _compiler(std::make_unique<compiler>())
	{
	}

	EvaluationValue exec(const std::string& program) {

        std::shared_ptr<Exp> ast = _parser->parse(program);

        co=_compiler->compile(*ast);


        ip=&co->code[0];
        sp=stack.begin();


        return evalExp();
	  }

    EvaluationValue evalExp() {
	    for (;;) {
	        auto op_code = READ_BYTE();
	        switch (op_code) {
	            case OP_HALT:
	                return pop();

	            case OP_CONST:
	                push(GET_CONST());
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
	                push(NUMBER(AS_NUMBER(left) / AS_NUMBER(right)));
	                break;
	            }

	            default:
	                DIE << "Unknown opcode: " << std::hex << static_cast<int>(op_code);
	        }
	    }
    }

private:

	void push(const EvaluationValue& value) {
		if (static_cast<size_t>(sp - stack.begin()) == STACK_LIMIT)
		{
			DIE << "push(): Stack overflow.\n";
		}
		*sp = value;
		sp++;
	}


	EvaluationValue pop() {
		if (sp == stack.begin()) {
			DIE << "push () : Stack empty";
		}
		--sp;
		return *sp;
	}

	uint8_t READ_BYTE() {
		return *ip++;
	}

	EvaluationValue GET_CONST() {
		return co->constants[READ_BYTE()];
	}


	//Stack pointer
	std::array<EvaluationValue, STACK_LIMIT>::iterator sp;

	//Instruction pointer (Program counter)
	uint8_t* ip;

	std::unique_ptr<parser> _parser;

	//Constant pool
	std::vector<EvaluationValue> constants;

    // Code object
    CodeObject* co;
	
	std::array<EvaluationValue, STACK_LIMIT>  stack;
    std::unique_ptr<compiler> _compiler;
};