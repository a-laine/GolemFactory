#include "Variant.h"
#include <stdexcept>

//********************************************----------------------------*********************************************//
//******************************************** constructors / destructors *********************************************//
//********************************************----------------------------*********************************************//
Variant::~Variant()
{
    setToNull();
}

Variant::Variant() : type(Variant::NULLTYPE) {
    value.Long=0; }

Variant::Variant(const Variant &v)
{
    type = v.getType();
    switch(type)
    {
        case Variant::STRING:
            value.String = new std::string(*v.value.String);
            break;
        case Variant::ARRAY:
            value.Array = new ArrayType(*v.value.Array);
            break;
        case Variant::MAP:
            value.Map = new MapType(*v.value.Map);
            break;
        default:
            value = v.value;
    }
}

Variant::Variant(const bool& var) : type(Variant::BOOL) {
    value.Bool=var; }

Variant::Variant(const char& var) : type(Variant::CHAR) {
    value.Int=var; }

Variant::Variant(const int& var) : type(Variant::INT) {
    value.Int=var; }

Variant::Variant(const int64_t& var) : type(Variant::LONG) {
    value.Long=var; }

Variant::Variant(const float& var) : type(Variant::FLOAT) {
    value.Float=var; }

Variant::Variant(const double& var) : type(Variant::DOUBLE) {
    value.Double=var; }

Variant::Variant(const std::string& var) : type(Variant::STRING) {
    value.String = new std::string(var); }

Variant::Variant(const char* var) : type(Variant::STRING) {
    value.String = new std::string(var); }

Variant::Variant(const ArrayType& var) : type(Variant::ARRAY) {
    value.Array = new ArrayType(var); }

Variant::Variant(const MapType& var) : type(Variant::MAP) {
    value.Map = new MapType(var); }



//********************************************----------------*********************************************//
//******************************************** type accessors *********************************************//
//********************************************----------------*********************************************//

Variant::VariantType Variant::getType() const {
    return type; }

bool Variant::isNull() const {
    return type==Variant::NULLTYPE; }




//********************************************----------------*********************************************//
//******************************************** data accessors *********************************************//
//********************************************----------------*********************************************//
bool Variant::toBool() const {
    return value.Bool; }

char Variant::toChar() const {
    return toInt(); }

int64_t Variant::toLong() const {
    return value.Long; }

int Variant::toInt() const {
    return value.Int; }

double Variant::toDouble() const {
    return value.Double; }

float Variant::toFloat() const {
    return value.Float; }

std::string Variant::toString() const
{
    if(type!=Variant::STRING)
		throw std::logic_error("Variant::toString : wrong type");
    return *value.String;
}


Variant& Variant::operator[] (const size_t key)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::operator[](size_t) : wrong type");
    return value.Array->at(key);
}

const Variant& Variant::operator[] (const size_t key) const
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::operator[](size_t) : wrong type");
    return value.Array->at(key);
}

Variant& Variant::operator[] (const std::string key)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::operator[](string) : wrong type");
    return value.Map->at(key);
}

const Variant& Variant::operator[] (const std::string key) const
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::operator[](string) : wrong type");
    return value.Map->at(key);
}


Variant::ArrayType& Variant::getArray() const
{
	if(type!=Variant::ARRAY)
		throw std::logic_error("Variant::getArray : wrong type");
	return *value.Array;
}

Variant::MapType& Variant::getMap() const
{
	if(type!=Variant::MAP)
		throw std::logic_error("Variant::getMap : wrong type");
	return *value.Map;
}

size_t Variant::size() const
{
    switch(type)
    {
        case Variant::ARRAY:
            return value.Array->size();
            break;
        case Variant::MAP:
            return value.Map->size();
            break;
        default:
            return 0;
    }
}



//********************************************----------------*********************************************//
//******************************************** data modifiers *********************************************//
//********************************************----------------*********************************************//
void Variant::setToNull()
{
    switch(type)
    {
        case STRING:
            delete value.String;
            break;
        case ARRAY:
            delete value.Array;
            break;
        case MAP:
            delete value.Map;
            break;
        default:
            break;
    }
    type = Variant::NULLTYPE;
    value.Long = 0;
}

Variant& Variant::operator= (const Variant &v)
{
	if (&v == this) return *this;
    setToNull();
    type = v.getType();
    switch(type)
    {
        case Variant::STRING:
            value.String = new std::string(*v.value.String);
            break;
        case Variant::ARRAY:
            value.Array = new ArrayType(*v.value.Array);
            break;
        case Variant::MAP:
            value.Map = new MapType(*v.value.Map);
            break;
        default:
            value = v.value;
    }
    return *this;
}

Variant& Variant::operator= (const bool& var)
{
    setToNull();
    type=Variant::BOOL; value.Bool=var;
    return *this;
}

Variant& Variant::operator= (const char& var)
{
    setToNull();
    type=Variant::CHAR; value.Int=var;
    return *this;
}

Variant& Variant::operator= (const int& var)
{
    setToNull();
    type=Variant::INT; value.Int=var;
    return *this;
}

Variant& Variant::operator= (const int64_t& var)
{
    setToNull();
    type=Variant::LONG; value.Long=var;
    return *this;
}

Variant& Variant::operator= (const float& var)
{
    setToNull();
    type=Variant::FLOAT; value.Float=var;
    return *this;
}

Variant& Variant::operator= (const double& var)
{
    setToNull();
    type=Variant::DOUBLE; value.Double=var;
    return *this;
}

Variant& Variant::operator= (const std::string& var)
{
    setToNull();
    type=Variant::STRING; value.String = new std::string(var);
    return *this;
}

Variant& Variant::operator= (const char* var)
{
    *this = std::string(var);
    return *this;
}

Variant& Variant::operator= (const ArrayType& var)
{
    setToNull();
    type=Variant::ARRAY; value.Array = new ArrayType(var);
    return *this;
}

Variant& Variant::operator= (const MapType& var)
{
    setToNull();
    type=Variant::MAP; value.Map = new MapType(var);
    return *this;
}


Variant::ArrayType& Variant::createArray()
{
    setToNull();
    type=Variant::ARRAY;
    value.Array = new ArrayType();
    return *value.Array;
}

Variant::MapType& Variant::createMap()
{
    setToNull();
    type=Variant::MAP;
    value.Map = new MapType();
    return *value.Map;
}


Variant& Variant::insert(const bool val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(bool) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const char val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(char) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const int val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(int) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const int64_t val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(long) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const float val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(float) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const double val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(double) : wrong type");
    value.Array->push_back(Variant(val));
    return value.Array->back();
}

Variant& Variant::insert(const std::string val)
{
    if(type==Variant::ARRAY)
    {
        value.Array->push_back(Variant(val));
        return value.Array->back();
    }
    else if(type==Variant::MAP)
    {
        (*value.Map)[val] = Variant();
        return (*value.Map)[val];
    }
    else
        throw std::logic_error("Variant::insert(string) : wrong type");

}

Variant& Variant::insert(const char* val)
{
    return insert(std::string(val));
}

Variant& Variant::insert(const Variant &val)
{
    if(type!=Variant::ARRAY)
        throw std::logic_error("Variant::insert(Variant&) : wrong type");
    if(val.getType()==Variant::ARRAY || val.getType()==Variant::MAP)
    {
        value.Array->push_back(Variant());
        value.Array->back() = val;
    }
    else
        value.Array->push_back(val);
    return value.Array->back();
}

Variant& Variant::insert(const std::string key, const bool val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,bool) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const char val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,char) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const int val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,int) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const int64_t val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,long) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const float val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,float) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const double val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,double) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const std::string val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,string) : wrong type");
    (*value.Map)[key] = Variant(val);
    return (*value.Map)[key];
}

Variant& Variant::insert(const std::string key, const char* val)
{
    return insert(key,std::string(val));
}

Variant& Variant::insert(const std::string key, const Variant &val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,Variant&) : wrong type");
    if(val.getType()==Variant::ARRAY || val.getType()==Variant::MAP)
    {
        (*value.Map)[key];
        (*value.Map)[key] = val;
    }
    else
        (*value.Map)[key] = val;
    return (*value.Map)[key];
}

