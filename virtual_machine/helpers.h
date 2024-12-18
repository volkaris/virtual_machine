#pragma once
#include "EvaluationValue.h"
#include "EvaluationValueType.h"

struct EvaluationValue;

inline double AS_NUMBER (const EvaluationValue& value) {
    return value.number();
}

inline bool AS_BOOL (const EvaluationValue& value) {
    return value.boolean();
}

inline bool IS_NIL(const EvaluationValue& value) {
    return value.type == EvaluationValueType::NIL;
}
inline bool IS_NUMBER (const EvaluationValue& value) {
    return value.type == EvaluationValueType::NUMBER;
}

inline bool IS_BOOL (const EvaluationValue& value) {
    return value.type == EvaluationValueType::BOOLEAN;
}