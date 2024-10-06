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

        co=_compiler->compile(*ast.get());
		/*constants.push_back(ALLOC_STRING("Hello"));
		constants.push_back(ALLOC_STRING(",world!"));
		
		code = {
			OP_CONST,0,
			OP_CONST,1,
			OP_ADD,
			OP_HALT
		};

		sp = stack.begin();
		ip = &code[0];*/

        ip=&co->code[0];
        sp=&stack[0];
		/*return eval(ast);*/

        return evalExp(ast);
	  }

    /*EvaluationValue eval() {
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


          default:
              DIE << "unknown code : " << std :: hex << op_code;
          }
      }
  }*/



    EvaluationValue evalExp(const std::shared_ptr<Exp>& exp) {
        switch (exp->type) {
            case ExpType::NUMBER:
                return NUMBER(exp->number);

            case ExpType::STRING:
                return ALLOC_STRING(exp->string);

            case ExpType::SYMBOL:
                // Handle symbols if necessary
                break;

            case ExpType::BINARY_EXP: {
                auto left = evalExp(exp->left);
                auto right = evalExp(exp->right);

                if (exp->op == "+") {
                    if (IS_NUMBER(left) && IS_NUMBER(right)) {
                        return NUMBER(AS_NUMBER(left) + AS_NUMBER(right));
                    } else if (IS_STRING(left) && IS_STRING(right)) {
                        return ALLOC_STRING(AS_CPP_STRING(left) + AS_CPP_STRING(right));
                    }
                } else if (exp->op == "-") {
                    return NUMBER(AS_NUMBER(left) - AS_NUMBER(right));
                } else if (exp->op == "*") {
                    return NUMBER(AS_NUMBER(left) * AS_NUMBER(right));
                } else if (exp->op == "/") {
                    return NUMBER(AS_NUMBER(left) / AS_NUMBER(right));
                }
                throw std::runtime_error("unknown operator");
            }
        }
        // Handle default case or throw an error
        DIE << "Unknown expression type.";
        return NUMBER(0); // Default return
    }

private:

	void push(const EvaluationValue& value) {
		if ((size_t)(sp - stack.begin()) == STACK_LIMIT)
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
		return constants[READ_BYTE()];
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