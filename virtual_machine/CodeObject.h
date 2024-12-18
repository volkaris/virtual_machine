//
// Created by Ilya on 18.12.2024.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "handlers.h"
#include "EvaluationValue.h" // This should define Object, EvaluationValue, etc.
                             // Make sure EvaluationValue.h doesn't include CodeObject.h


struct DecodedInstruction {
    InstructionHandler fn;
    std::vector<uint8_t> args; // Arguments (if any)
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

inline EvaluationValue ALLOC_CODE (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new CodeObject(value));
    return val;
}

inline CodeObject* AS_CODE (const EvaluationValue& evaValue) {
    return static_cast<CodeObject*>(evaValue.object());
}