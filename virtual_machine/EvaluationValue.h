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


std::string evaValueToConstantString(const EvaluationValue &evaValue)
{
    std::stringstream ss;
    if (IS_NUMBER(evaValue))
    {
        ss << evaValue.number();
    }
    else if (IS_BOOLEAN(evaValue))
    {
        ss << (evaValue.boolean() ? "true" : "false");
    }
    else if (IS_STRING(evaValue))
    {
        ss << '"' << AS_CPP_STRING(evaValue) << '"';
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
    else
    {
        DIE << "evaValueToConstantString: unknown type " << (int)evaValue.type;
    }
    return ss.str();
}
