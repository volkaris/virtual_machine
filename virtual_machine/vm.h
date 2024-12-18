#pragma once

#include <vector>
#include <string>
#include <memory>
#include <array>

#include "bytecodeGenerator.h"
#include "parser.h"

#include "CodeObject.h"      // Needed for CodeObject*
#include "Global.h"          // If needed// If needed
 // If handlers used here, but usually just needed in vm.cpp


#define STACK_LIMIT 512

class bytecodeGenerator;
struct CallFrame;
class vm;

/*typedef void (*InstructionHandler)(vm*, CallFrame&, uint8_t*&);*/

/*static void handleHalt(vm* machine, CallFrame &frame, uint8_t *&ip);
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
static void handleLogicalAnd(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleLogicalOr(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJumpIfFalseOrPop(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleJumpIfTrueOrPop(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleDup(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleCall(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleReturn(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArray(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArrayGet(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleArraySet(vm* machine, CallFrame &frame, uint8_t *&ip);
static void handleNil(vm* machine, CallFrame &frame, uint8_t *&ip);*/

// Create the handlers table
// Make sure every opcode from your OpCode.h is assigned here in the correct order.
/*
static InstructionHandler handlers[0xFF + 1] = {
    handleHalt,            // 0x00 OP_HALT
    handleConst,           // 0x01 OP_CONST
    handleAdd,             // 0x02 OP_ADD
    handleSub,             // 0x03 OP_SUB
    handleMul,             // 0x04 OP_MUL
    handleDiv,             // 0x05 OP_DIV
    handleCompare,         // 0x06 OP_COMPARE
    handleJumpIfFalse,     // 0x07 OP_JUMP_IF_FALSE
    handleJump,            // 0x08 OP_JUMP
    handleGetGlobal,       // 0x09 OP_GET_GLOBAL
    nullptr,               // 0x0A (unused)
    nullptr,               // 0x0B (unused)
    nullptr,               // 0x0C (unused)
    nullptr,               // 0x0D (unused)
    nullptr,               // 0x0E (unused)
    nullptr,               // 0x0F (unused)
    handleSetGlobal,       // 0x10 OP_SET_GLOBAL
    handleGetLocal,        // 0x11 OP_GET_LOCAL
    handleSetLocal,        // 0x12 OP_SET_LOCAL
    handleLogicalNot,      // 0x13 OP_LOGICAL_NOT
    handleLogicalAnd,      // 0x14 OP_LOGICAL_AND
    handleLogicalOr,       // 0x15 OP_LOGICAL_OR
    handleJumpIfFalseOrPop,// 0x16 OP_JUMP_IF_FALSE_OR_POP
    handleJumpIfTrueOrPop, // 0x17 OP_JUMP_IF_TRUE_OR_POP
    handleDup,             // 0x18 OP_DUP
    handleCall,            // 0x19 OP_CALL
    handleReturn,          // 0x1A OP_RETURN
    handleArray,           // 0x1B OP_ARRAY
    handleArrayGet,        // 0x1C OP_ARRAY_GET
    handleArraySet,        // 0x1D OP_ARRAY_SET
    nullptr,               // 0x1E (unused)
    nullptr,               // 0x1F (unused)
    handleNil              // 0x20 OP_NIL
    // Make sure to fill in any missing handlers if needed
};
*/


struct CallFrame {
    CodeObject *co; // Pointer to the function's CodeObject
    uint8_t *ip; // Instruction pointer within the function's bytecode
    std::vector<EvaluationValue> locals; // Local variables for the function

    CallFrame(CodeObject *codeObject)
        : co(codeObject), ip(codeObject->code.data()) {
        // Determine the maximum slot index
        int maxSlot = -1;
        for (const auto &pair: co->localNames) {
            if (pair.first > maxSlot) {
                maxSlot = pair.first;
            }
        }
        // Initialize locals vector with NIL
        locals.resize(maxSlot + 1, NIL());
    }
};

class vm {
public:
    vm() : global(std::make_shared<Global>()),
           _parser(std::make_unique<syntax::parser>()),
           _bytecodeGenerator(std::make_unique<bytecodeGenerator>(global)) {
        setGlobalVariables();
    }

    size_t nextInstruction = 0;

    EvaluationValue exec(const std::string &program) {

        std::shared_ptr<Exp> ast = _parser->parse(program);
        co = _bytecodeGenerator->compile(*ast);

        // Initialize the main call frame
        callStack.emplace_back(co);

        // Initialize the instruction pointer in the main call frame
        CallFrame &currentFrame = callStack.back();
        currentFrame.ip = currentFrame.co->code.data();
        sp = stack.begin();

        // Optionally, disassemble bytecode for debugging
         _bytecodeGenerator->disassembleBytecode();

        return evalExp();
    }

    template<typename T>
    void compare_values(const T &casted_left, const T &casted_right, uint8_t compare_op) {
        bool res = false;
        switch (compare_op) {
            case 0:
                res = casted_left < casted_right;
                break;
            case 1:
                res = casted_left > casted_right;
                break;
            case 2:
                res = casted_left == casted_right;
                break;
            case 3:
                res = casted_left >= casted_right;
                break;
            case 4:
                res = casted_left <= casted_right;
                break;
            case 5:
                res = casted_left != casted_right;
                break;
            default: {
                throw std::runtime_error("Unknown compare operation." + std::to_string(compare_op));
            }
        }
        push(BOOLEAN(res));
    }

    EvaluationValue evalExp() {
        CallFrame &currentFrame = callStack.back();
        auto &decoded = currentFrame.co->decodedInstructions;

        nextInstruction = 0; // start at 0

        while (!callStack.empty()) {
            if (nextInstruction >= decoded.size()) {
                // Out of range => halt or error
                break;
            }

            DecodedInstruction &inst = decoded[nextInstruction];
            // Call the handler
            inst.fn(this, currentFrame, inst.args);

            // If callStack empty (halt or return from main), break
            if (callStack.empty()) {
                break;
            }

            // By default, go to next instruction
            // If a jump was executed, nextInstruction was changed by that handler
            // If not changed, increment by 1
            nextInstruction++;
        }

        if (sp == stack.begin()) {
            return NIL();
        }
        return pop();
    }


    void setGlobalVariables() {
        /*global->addConst("x", 10);*/
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
        if (static_cast<size_t>(sp - stack.begin()) == STACK_LIMIT) {
            throw std::runtime_error("Stack overflow.");
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
            throw std::runtime_error("Stack empty.");
        }
        sp--;
        return *sp;
    }

    EvaluationValue peek() {
        if (sp == stack.begin()) {
            throw std::runtime_error("Stack empty.");
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

    //Global object
    std::shared_ptr<Global> global;

    //Stack pointer
    std::array<EvaluationValue, STACK_LIMIT> stack;
    std::array<EvaluationValue, STACK_LIMIT>::iterator sp = stack.begin();

    std::unique_ptr<syntax::parser> _parser;

    // Code object
    CodeObject *co;

    std::vector<CallFrame> callStack;

    std::unique_ptr<bytecodeGenerator> _bytecodeGenerator;
};


// Define a function pointer type for handlers


