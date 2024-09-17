#pragma once
#include <vector>
#include <string>
#include "OpCode.h"
#include <array>

//#define READ_BYTE() *ip++
#define STACK_LIMIT 512
//#define GET_CONST()  constants[READ_BYTE()]

#include "Logger.h"
#include "EvaValue.h"

class EvaVm {
public:

	EvaVm()
	{
		//sp = stack.begin();
	}

	EvaValue exec(const std::string& program) {
		
		constants.push_back(ALLOC_STRING("Hello"));
		constants.push_back(ALLOC_STRING(",world!"));
		
		code = {
			OP_CONST,0,
			OP_CONST,1,
			OP_ADD,
			OP_HALT
		};

		sp = stack.begin();
		ip = &code[0];


		return eval();
	  }


private:

	void push(const EvaValue& value) {
		if ((size_t)(sp - stack.begin()) == STACK_LIMIT)
		{
			DIE << "push(): Stack overflow.\n";
		}
		*sp = value;
		sp++;
	}


	EvaValue pop() {
		if (sp == stack.begin()) {
			DIE << "push () : Stacl empty";
		}
		--sp;
		return *sp;
	}

	EvaValue eval() {
		for (;;) {
			auto op_code = READ_BYTE();
			switch (op_code)
			{
			case OP_HALT :
				return pop();


			case OP_CONST:
			
				push(GET_CONST());
				break;

			case OP_ADD: {


				decltype(auto) secondOperand = pop();
				decltype(auto) firstOperand = pop();

				if (IS_NUMBER(firstOperand) and IS_NUMBER(secondOperand)) {
					decltype(auto) first = AS_NUMBER(firstOperand);
					decltype(auto) second = AS_NUMBER(secondOperand);
					push(NUMBER(first + second));
				}
				
				if (IS_STRING(firstOperand) and IS_STRING(secondOperand)) {
					decltype(auto) first = AS_CPP_STRING(firstOperand);
					decltype(auto) second = AS_CPP_STRING(secondOperand);
					push(ALLOC_STRING(first + second));
				}
				

				break;
			}

			case OP_SUB: {

				decltype(auto) secondOperand = AS_NUMBER(pop());
				decltype(auto) firstOperand = AS_NUMBER(pop());

				push(NUMBER(firstOperand - secondOperand));

				break;
			}

			case OP_MUL: {

				decltype(auto) secondOperand = AS_NUMBER(pop());
				decltype(auto) firstOperand = AS_NUMBER(pop());

				push(NUMBER(firstOperand * secondOperand));

				break;
			}
			case OP_DIV: {

				decltype(auto) secondOperand = AS_NUMBER(pop());
				decltype(auto) firstOperand = AS_NUMBER(pop());

				push(NUMBER(firstOperand / secondOperand));

				break;
			}
			


			default:
				DIE << "unknown code : " << std :: hex << op_code;
			}
		}
	}



	uint8_t READ_BYTE() {
		return *ip++;
	}

	EvaValue GET_CONST() {
		return constants[READ_BYTE()];
	}


	//Stack pointer
	std::array<EvaValue, STACK_LIMIT>::iterator sp;

	//Instruction pointer (Program counter)
	uint8_t* ip;

	//Constant pool
	std::vector<EvaValue> constants;

	//Bytecode
	std::vector<uint8_t> code;
	
	std::array<EvaValue, STACK_LIMIT>  stack;
};