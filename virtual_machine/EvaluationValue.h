#pragma once
#include <utility>
#include <variant>
#include <vector>

enum class EvaluationValueType {
    NUMBER,
    BOOLEAN,
    OBJECT,
    NIL,
};

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
    explicit StringObject (const std::string& str) : Object(ObjectType::STRING) {
        this->string = str;
    }

    std::string string;
};

/*struct CodeObject : public Object {
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
};*/

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




inline EvaluationValue NUMBER (double value) {
    EvaluationValue val;
    val.type = EvaluationValueType::NUMBER;
    val.value = value;
    return val;
}

inline EvaluationValue BOOLEAN (bool value) {
    EvaluationValue val;
    val.type = EvaluationValueType::BOOLEAN;
    val.value = value;
    return val;
}

inline EvaluationValue NIL() {
    EvaluationValue val;
    val.type = EvaluationValueType::NIL;
    val.value=nullptr;
    return val;
}


inline EvaluationValue ALLOC_STRING (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new StringObject(value));
    return val;
}


/*inline EvaluationValue ALLOC_CODE (std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object*>(new CodeObject(value));
    return val;
}*/

inline double AS_NUMBER (const EvaluationValue& value) {
    return value.number();
}

inline StringObject* AS_STRING (const EvaluationValue& value) {
    return static_cast<StringObject*>(value.object());
}

inline bool AS_BOOL (const EvaluationValue& value) {
    return value.boolean();
}


inline std::string AS_CPP_STRING (const EvaluationValue& evaValue) {
    return AS_STRING(evaValue)->string;
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

inline bool IS_OBJECT (const EvaluationValue& value) {
    return value.type == EvaluationValueType::OBJECT;
}

inline bool IS_OBJECT_TYPE (const EvaluationValue& value, const ObjectType& objectType) {
    return IS_OBJECT(value) and value.object()->type == objectType;
}

// Type checking
inline bool IS_ARRAY(const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::ARRAY);
}

inline bool IS_STRING (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::STRING);
}

inline bool IS_CODE (const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE);
}



/*inline std::string evaValueToConstantString(const EvaluationValue &evaValue)
{
    std::stringstream ss;
    switch (evaValue.type) {
        case EvaluationValueType::NUMBER:
            ss << evaValue.number();
            break;
        case EvaluationValueType::BOOLEAN:
            ss << (evaValue.boolean() ? "true" : "false");
            break;
        case EvaluationValueType::NIL:
            ss << "nil";
            break;
        case EvaluationValueType::OBJECT:
            if (evaValue.object() == nullptr) {
                ss << "null_object";
                break;
            }
            switch (evaValue.object()->type) {
                case ObjectType::STRING: {
                    StringObject* strObj = static_cast<StringObject*>(evaValue.object());
                    ss << "\"" << strObj->string << "\"";
                    break;
                }
                case ObjectType::ARRAY: {
                    ArrayObject* arrObj = static_cast<ArrayObject*>(evaValue.object());
                    ss << "[";
                    for (size_t i = 0; i < arrObj->elements.size(); ++i) {
                        ss << evaValueToConstantString(arrObj->elements[i]);
                        if (i < arrObj->elements.size() - 1) {
                            ss << ", ";
                        }
                    }
                    ss << "]";
                    break;
                }
                case ObjectType::CODE: {
                    CodeObject* codeObj = static_cast<CodeObject*>(evaValue.object());
                    ss << "<CodeObject: " << codeObj->name << ">";
                    break;
                }
                default:
                    ss << "<Unknown Object Type>";
                    break;
            }
            break;
        default:
            throw std::runtime_error("evaValueToConstantString: Unknown EvaluationValueType " + std::to_string(static_cast<int>(evaValue.type)));
    }
    return ss.str();
}*/
