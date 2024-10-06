#pragma once
#include <variant>

enum class EvaluationValueType {
	NUMBER,
	OBJECT
};

enum class ObjectType {
	STRING,
    CODE,
};

struct Object {
	Object(ObjectType type)	 
	{
		this->type = type;
	} 
	ObjectType type;
};

struct EvaluationValue
{
    EvaluationValueType type;
    union
    {
        double number;
        Object* object;
    };
};

struct StringObject : public Object {
	StringObject(const std::string& str) : Object(ObjectType::STRING)
	{
		this->string = str;
	}
	std::string string;
 };

struct CodeObject : public Object {


    CodeObject(const std::string &name) :  name(name), Object(ObjectType::CODE) {}

    //name of unit (most of the cases for function name)
    std::string name;

    //constant pool
    std::vector<EvaluationValue> constants;

    // bytecode
    std::vector<uint8_t> code;
};


EvaluationValue NUMBER(double value) {
	EvaluationValue val;
    val.type = EvaluationValueType::NUMBER;
    val.number = value;
	return val;
}

EvaluationValue ALLOC_STRING(std::string value) {
	EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.object = (Object*)new StringObject(value);
	return val;
}


EvaluationValue ALLOC_CODE(std::string value) {
    EvaluationValue val;
    val.type = EvaluationValueType::OBJECT;
    val.object = (Object*)new CodeObject(value);
    return val;
}

double AS_NUMBER(const EvaluationValue& value) {
	return  static_cast<double> ((value).number);
}

StringObject* AS_STRING(const EvaluationValue& value) {
	return  static_cast<StringObject*> ((value).object);
	
}

std::string AS_CPP_STRING(const EvaluationValue& evaValue) {
	return  AS_STRING(evaValue)->string;
}

CodeObject* AS_CODE(const EvaluationValue& evaValue) {
    return  static_cast<CodeObject*> ((evaValue).object);

}


bool IS_NUMBER(const EvaluationValue& value) {
    return value.type == EvaluationValueType::NUMBER  ;
}

bool IS_OBJECT(const EvaluationValue& value) {
	return value.type == EvaluationValueType::OBJECT;
}

bool IS_OBJECT_TYPE(const EvaluationValue& value, const ObjectType& objectType) {
	return IS_OBJECT(value) and value.object->type == objectType ;
}

bool IS_STRING(const EvaluationValue& value) {
	return  IS_OBJECT_TYPE(value, ObjectType::STRING);
}

bool IS_CODE(const EvaluationValue& value) {
    return IS_OBJECT_TYPE(value, ObjectType::CODE) ;
}