#pragma once
#include <vector>

#include "EvaluationValue.h"
#include "EvaluationValueType.h"
#include "Object.h"


struct EvaluationValue;

struct ArrayObject : Object {
    ArrayObject() : Object(ObjectType::ARRAY) {}
    std::vector<EvaluationValue> elements; // Stores array elements
};

// Allocation functions
inline EvaluationValue ALLOC_ARRAY() {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new ArrayObject());
    return val;
}

// Helper functions to cast EvaluationValue to ArrayObject
inline ArrayObject* AS_ARRAY(const EvaluationValue& value) {
    return static_cast<ArrayObject*>(value.object());
}

inline bool IS_ARRAY(const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::ARRAY);
}