#pragma once




enum class ObjectType {
    STRING,
    CODE,
    ARRAY,   // Added for arrays
};

struct Object {
    explicit Object (ObjectType type) {
        this->type = type;
    }

    ObjectType type;
};

inline bool IS_OBJECT (const EvaluationValue& value) {
    return value.type == EvaluationValueType::OBJECT;
}

inline bool IS_OBJECT_TYPE (const EvaluationValue& value, const ObjectType& objectType) {
    return IS_OBJECT(value) and value.object()->type == objectType;
}