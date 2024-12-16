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
    NATIVE
};

struct Object {
    explicit Object(ObjectType type) {
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

    //name of unit (most of the cases for function name)
    std::string name;

    //constant pool
    std::vector<EvaluationValue> constants;

    // bytecode
    std::vector<uint8_t> code;

    // Mapping from slot indices to variable names
    std::unordered_map<int, std::string> localNames;
};

using NativeFn = std::function<void()>;

struct NativeObject : public Object {
    NativeObject(NativeFn function, const std::string &name, size_t arity) : Object(ObjectType::NATIVE),
                                                                             function(function), name(name),
                                                                             arity(arity) {
    }

    NativeFn function;
    std::string name;
    size_t arity;
};


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

inline EvaluationValue ALLOC_NATIVE(NativeFn function,std::string name,size_t arity) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.value = static_cast<Object *>(new NativeObject(function,name,arity));
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

inline NativeObject* AS_NATIVE(const EvaluationValue &value) {
    return static_cast<NativeObject*>(value.object());
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

inline bool IS_STRING(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::STRING);
}

inline bool IS_CODE(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE);
}

inline bool IS_NATIVE(const EvaluationValue &value) {
    return IS_OBJECT_TYPE(value, ObjectType::NATIVE);
}

inline std::string evaValueToConstantString(const EvaluationValue &evaValue) {
    std::stringstream ss;
    if (IS_NUMBER(evaValue)) {
        ss << evaValue.number();
    } else if (IS_BOOL(evaValue)) {
        ss << (evaValue.boolean() ? "true" : "false");
    } else if (IS_STRING(evaValue)) {
        ss << '"' << AS_CPP_STRING(evaValue) << '"';
    }
    else if (IS_CODE(evaValue))
    {
        auto code = AS_CODE(evaValue);
        ss << "code: " << code << ": " << code->name << "/" << code->name;
    }
    else if (IS_NATIVE(evaValue))
    {
        auto fn = AS_NATIVE(evaValue);
        ss << fn->name << "/" << fn->arity;
    }

    /*
    else if (IS_CODE(evaValue))
    {
        auto code = AS_CODE(evaValue);
        ss << "code: " << code << ": " << code->name << "/" << code->arity;
    }
    else if (IS_FUNCTION(evaValue))
    {
        auto fn = AS_FUNCTION(evaValue);
        ss << fn->co->name << "/" << fn->co->arity;
    }
    else if (IS_NATIVE(evaValue))
    {
        auto fn = AS_NATIVE(evaValue);
        ss << fn->name << "/" << fn->arity;
    }
    else if (IS_CELL(evaValue))
    {
        auto cell = AS_CELL(evaValue);
        ss << "cell: " << evaValueToConstantString(cell->value);
    }
    else if (IS_CLASS(evaValue))
    {
        auto cls = AS_CLASS(evaValue);
        ss << "class: " << cls->name;
    }
    else if (IS_INSTANCE(evaValue))
    {
        auto inst = AS_INSTANCE(evaValue);
        ss << "instance: " << inst->cls->name;
    }
    */
    else {
        throw std::runtime_error(
            "valueToConstantString: unknown type " + std::to_string(static_cast<int>(evaValue.type)));
    }
    return ss.str();
}
