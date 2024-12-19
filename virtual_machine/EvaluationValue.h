#pragma once
#include <unordered_map>
#include <utility>
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
    ARRAY,
};

struct Object {
    explicit Object(ObjectType type) {
        this->type = type;
    }

    ObjectType type;
};

struct EvaluationValue {
    EvaluationValueType type;


    std::variant<bool, double, Object *, std::nullptr_t> value;

    [[nodiscard]] bool boolean() const {
        return std::get<bool>(value);
    }

    [[nodiscard]] double number() const {
        return std::get<double>(value);
    }

    [[nodiscard]] Object *object() const {
        return std::get<Object *>(value);
    }
};

struct StringObject : Object {
    explicit StringObject(const std::string &str) : Object(ObjectType::STRING) {
        this->string = str;
    }

    std::string string;
};

struct CodeObject : public Object {
    explicit CodeObject(std::string name) : Object(ObjectType::CODE), name(std::move(name)) {
    }


    std::string name;


    std::vector<EvaluationValue> constants;


    std::vector<uint8_t> code;


    std::unordered_map<int, std::string> localNames;
};

struct ArrayObject : Object {
    ArrayObject() : Object(ObjectType::ARRAY) {
    }

    std::vector<EvaluationValue> elements;
};


inline EvaluationValue ALLOC_ARRAY() {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object *>(new ArrayObject());
    return val;
}


inline ArrayObject *AS_ARRAY(const EvaluationValue &value) {
    return static_cast<ArrayObject *>(value.object());
}

inline EvaluationValue NUMBER(double value) {
    EvaluationValue val;
    val.type = EvaluationValueType::NUMBER;
    val.value = value;
    return val;
}

inline EvaluationValue BOOLEAN(bool value) {
    EvaluationValue val;
    val.type = EvaluationValueType::BOOLEAN;
    val.value = value;
    return val;
}

inline EvaluationValue NIL() {
    EvaluationValue val;
    val.type = EvaluationValueType::NIL;
    val.value = nullptr;
    return val;
}

inline EvaluationValue ALLOC_STRING(std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object *>(new StringObject(value));
    return val;
}

inline EvaluationValue ALLOC_CODE(std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object *>(new CodeObject(value));
    return val;
}

inline double AS_NUMBER(const EvaluationValue &value) {
    return value.number();
}

inline StringObject *AS_STRING(const EvaluationValue &value) {
    return static_cast<StringObject *>(value.object());
}

inline bool AS_BOOL(const EvaluationValue &value) {
    return value.boolean();
}

inline std::string AS_CPP_STRING(const EvaluationValue &evaValue) {
    return AS_STRING(evaValue)->string;
}

inline CodeObject *AS_CODE(const EvaluationValue &evaValue) {
    return static_cast<CodeObject *>(evaValue.object());
}

inline bool IS_NIL(const EvaluationValue &value) {
    return value.type == EvaluationValueType::NIL;
}

inline bool IS_NUMBER(const EvaluationValue &value) {
    return value.type == EvaluationValueType::NUMBER;
}

inline bool IS_BOOL(const EvaluationValue &value) {
    return value.type == EvaluationValueType::BOOLEAN;
}

inline bool IS_OBJECT(const EvaluationValue &value) {
    return value.type == EvaluationValueType::OBJECT;
}

inline bool IS_OBJECT_TYPE(const EvaluationValue &value, const ObjectType &objectType) {
    return IS_OBJECT(value) and value.object()->type == objectType;
}


inline bool IS_ARRAY(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::ARRAY);
}

inline bool IS_STRING(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::STRING);
}

inline bool IS_CODE(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE);
}

inline std::string evaluationValueToConstantString(const EvaluationValue &evaluationValue) {
    std::stringstream ss;
    switch (evaluationValue.type) {
        case EvaluationValueType::NUMBER:
            ss << evaluationValue.number();
            break;
        case EvaluationValueType::BOOLEAN:
            ss << (evaluationValue.boolean() ? "true" : "false");
            break;
        case EvaluationValueType::NIL:
            ss << "nil";
            break;
        case EvaluationValueType::OBJECT:
            if (evaluationValue.object() == nullptr) {
                ss << "null_object";
                break;
            }
            switch (evaluationValue.object()->type) {
                case ObjectType::STRING: {
                    StringObject *strObj = static_cast<StringObject *>(evaluationValue.object());
                    ss << "\"" << strObj->string << "\"";
                    break;
                }
                case ObjectType::ARRAY: {
                    ArrayObject *arrObj = static_cast<ArrayObject *>(evaluationValue.object());
                    ss << "[";
                    for (size_t i = 0; i < arrObj->elements.size(); ++i) {
                        ss << evaluationValueToConstantString(arrObj->elements[i]);
                        if (i < arrObj->elements.size() - 1) {
                            ss << ", ";
                        }
                    }
                    ss << "]";
                    break;
                }
                case ObjectType::CODE: {
                    CodeObject *codeObj = static_cast<CodeObject *>(evaluationValue.object());
                    ss << "<Объект кода " << codeObj->name << ">";
                    break;
                }
                default:
                    ss << " неизвестный тип объекта, по-хорошему,вас здесь не должно быть ";
                    break;
            }
            break;
        default:
            throw std::runtime_error(
                " неизвестный тип вычисляемого значения, по-хорошему,вас здесь не должно быть " + std::to_string(
                    static_cast<int>(evaluationValue.type)));
    }
    return ss.str();
}
