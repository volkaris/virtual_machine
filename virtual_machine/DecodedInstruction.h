#pragma once
#include "vm.h"

struct DecodedInstruction {
    InstructionHandler fn;
    std::vector<uint8_t> args; // Arguments (if any) for this instruction
};

struct CodeObject : public Object {
    explicit CodeObject (std::string  name) : Object(ObjectType::CODE), name(std::move(name)) {
    }

    //name of unit (most of the cases for function name)
    std::string name;

    //constant pool
    std::vector<EvaluationValue> constants;

    // bytecode
    std::vector<uint8_t> code;

    // Mapping from slot indices to variable names
    std::unordered_map<int, std::string> localNames;

    std::vector<DecodedInstruction> decodedInstructions;
};
inline CodeObject* AS_CODE (const EvaluationValue& evaValue) {
    return static_cast<CodeObject*>(evaValue.object());
}

inline EvaluationValue ALLOC_CODE (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new CodeObject(value));
    return val;
}

inline bool IS_CODE (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE);
}