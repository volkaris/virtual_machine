#pragma once
#include <variant>

enum class EvaluationValueType {
    NUMBER,
    BOOLEAN,
    OBJECT,
    NIL,
};

enum class ObjectType {
    STRING,
    CODE,
};

struct Object {
    Object (ObjectType type) {
        this->type = type;
    }

    ObjectType type;
};

struct EvaluationValue {
    EvaluationValueType type;
    /*union
    {
    	bool boolean;
        double number;
        Object* object;
    };*/
    std::variant<bool, double, Object*,std::nullptr_t> value;

    [[nodiscard]] bool boolean () const {
        return std::get<bool>(value);
    }

    [[nodiscard]] double number () const {
        return std::get<double>(value);
    }

    [[nodiscard]] Object* object () const {
        return std::get<Object*>(value);
    }
};

struct StringObject : Object {
    StringObject (const std::string& str) : Object(ObjectType::STRING) {
        this->string = str;
    }

    std::string string;
};

struct CodeObject : public Object {
    CodeObject (const std::string& name) : name(name), Object(ObjectType::CODE) {
    }

    //name of unit (most of the cases for function name)
    std::string name;

    //constant pool
    std::vector<EvaluationValue> constants;

    // bytecode
    std::vector<uint8_t> code;
};


EvaluationValue NUMBER (double value) {
    EvaluationValue val;
    val.type = EvaluationValueType::NUMBER;
    val.value = value;
    return val;
}

EvaluationValue BOOLEAN (bool value) {
    EvaluationValue val;
    val.type = EvaluationValueType::BOOLEAN;
    val.value = value;
    return val;
}

EvaluationValue NIL() {
    EvaluationValue val;
    val.type = EvaluationValueType::NIL;
    val.value=nullptr;
    return val;
}


EvaluationValue ALLOC_STRING (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new StringObject(value));
    return val;
}


EvaluationValue ALLOC_CODE (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new CodeObject(value));
    return val;
}

double AS_NUMBER (const EvaluationValue& value) {
    return value.number();
}

StringObject* AS_STRING (const EvaluationValue& value) {
    return static_cast<StringObject*>(value.object());
}

bool AS_BOOLEAN (const EvaluationValue& value) {
    return value.boolean();
}


std::string AS_CPP_STRING (const EvaluationValue& evaValue) {
    return AS_STRING(evaValue)->string;
}

CodeObject* AS_CODE (const EvaluationValue& evaValue) {
    return static_cast<CodeObject*>(evaValue.object());
}

bool IS_NIL(const EvaluationValue& value) {
    return value.type == EvaluationValueType::NIL;
}
bool IS_NUMBER (const EvaluationValue& value) {
    return value.type == EvaluationValueType::NUMBER;
}

bool IS_BOOLEAN (const EvaluationValue& value) {
    return value.type == EvaluationValueType::BOOLEAN;
}

bool IS_OBJECT (const EvaluationValue& value) {
    return value.type == EvaluationValueType::OBJECT;
}

bool IS_OBJECT_TYPE (const EvaluationValue& value, const ObjectType& objectType) {
    return IS_OBJECT(value) and value.object()->type == objectType;
}

bool IS_STRING (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::STRING);
}

bool IS_CODE (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE);
}
