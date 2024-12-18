#pragma once
#include <string>

#include "EvaluationValue.h"
#include "EvaluationValueType.h"
#include "Object.h"


struct StringObject : Object {
    explicit StringObject (const std::string& str) : Object(ObjectType::STRING) {
        this->string = str;
    }

    std::string string;
};

inline EvaluationValue ALLOC_STRING (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new StringObject(value));
    return val;
}

inline StringObject* AS_STRING (const EvaluationValue& value) {
    return static_cast<StringObject*>(value.object());
}

inline std::string AS_CPP_STRING (const EvaluationValue& evaValue) {
    return AS_STRING(evaValue)->string;
}

inline bool IS_STRING (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::STRING);
}