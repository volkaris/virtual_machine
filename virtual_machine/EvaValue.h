#pragma once
#include <variant>

enum class EvaValueType {

	NUMBER,
	OBJECT


};


enum class ObjectType {

	STRING,


};

struct Object {
	Object(ObjectType type)	 
	{
		this->type = type;
	} 
	ObjectType type;
};


struct StringObject : public Object {
	StringObject(const std::string& str) : Object(ObjectType::STRING)
	{
		this->string = str;
	}
	std::string string;
 };


struct EvaValue
{
	EvaValueType type;
	union 
	{
		double number;
		Object* object; // todo когда будешь переделывать на std::variant, 
						// todo то вместо обджекта можно напрямую хранить строчку
	};

	
	/*std::variant<double , std::string> value;*/


};
//#define NUMBER(value) EvaValue { EvaValueType::NUMBER,  value}
EvaValue NUMBER(double value) {
	EvaValue evaValue;
	evaValue.type = EvaValueType::NUMBER;
	evaValue.number = value;
	return evaValue;
}

EvaValue ALLOC_STRING(std::string value) {
	EvaValue evaValue;
	evaValue.type = EvaValueType::OBJECT;
	evaValue.object = (Object*)new StringObject(value);
	return evaValue;
}
//#define AS_NUMBER(evaValue) EvaValue ((double)(evaValue).number)
double AS_NUMBER(const EvaValue& evaValue) {
	return  static_cast<double> ((evaValue).number);
	
	/*return std::get<double> (evaValue.value);*/
}

StringObject* AS_STRING(const EvaValue& evaValue) {

	return  static_cast<StringObject*> ((evaValue).object);
	/*return std::get<std::string>(evaValue.value);*/
	
}

std::string AS_CPP_STRING(const EvaValue& evaValue) {

	return  AS_STRING(evaValue)->string;
	/*return std::get<std::string>(evaValue.value);*/

}

bool IS_NUMBER(const EvaValue& evaValue) {

	return  evaValue.type== EvaValueType::NUMBER  ;

}

bool IS_OBJECT(const EvaValue& evaValue) {

	return  evaValue.type == EvaValueType::OBJECT;

}

bool IS_OBJECT_TYPE(const EvaValue& evaValue,const ObjectType& objectType) {

	return  IS_OBJECT(evaValue) and evaValue.object->type==objectType ;
}

bool IS_STRING(const EvaValue& evaValue) {

	return  IS_OBJECT_TYPE(evaValue, ObjectType::STRING);
}