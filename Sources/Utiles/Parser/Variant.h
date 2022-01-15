#pragma once

#include <string>
#include <map>
#include <deque>


class Variant
{
    public:
        typedef std::deque<Variant> ArrayType;
        typedef std::map<std::string,Variant> MapType;
        enum VariantType {
            NULLTYPE,
            BOOL,
            CHAR,
            INT,
            LONG,
            FLOAT,
            DOUBLE,
            STRING,
            ARRAY,
            MAP,
            CODEBLOCK
        };

        //++++ constructors / destructors
        //===============================
        Variant();
        Variant(const Variant &v);
        Variant(const bool& var);
        Variant(const char& var);
        Variant(const int& var);
        Variant(const int64_t& var);
        Variant(const float& var);
        Variant(const double& var);
        Variant(const std::string& var, bool asCodeBlock = false);
        Variant(const char* var);
        Variant(const ArrayType& var);
        Variant(const MapType& var);
        virtual ~Variant();

        //++++ type accessors
        //===================
        VariantType getType() const;
        bool isNull() const;

        //++++ data accessors
        //===================
        bool toBool() const;
        char toChar() const;
        int64_t toLong() const;
        int toInt() const;
        double toDouble() const;
        float toFloat() const;
        std::string toString() const;

        Variant& operator[] (const size_t& key);
        const Variant& operator[] (const size_t& key) const;
        Variant& operator[] (const std::string& key);
        const Variant& operator[] (const std::string& key) const;

        ArrayType& getArray() const;
        MapType& getMap() const;

        size_t size() const;

        //++++ modifiers
        //==============
        void setToNull();

        Variant& operator= (const Variant &v);
        Variant& operator= (const bool& var);
        Variant& operator= (const char& var);
        Variant& operator= (const int& var);
        Variant& operator= (const int64_t& var);
        Variant& operator= (const float& var);
        Variant& operator= (const double& var);
        Variant& operator= (const std::string& var);
        Variant& operator= (const char* var);
        Variant& operator= (const ArrayType& var);
        Variant& operator= (const MapType& var);

        ArrayType& createArray();
        MapType& createMap();

        Variant& insert(const Variant &val);
        Variant& insert(const bool& val);
        Variant& insert(const char& val);
        Variant& insert(const int& val);
        Variant& insert(const int64_t& val);
        Variant& insert(const float& val);
        Variant& insert(const double& val);
        Variant& insert(const std::string& val);
        Variant& insert(const char* val);

        Variant& insert(const std::string& key, const Variant &val);
        Variant& insert(const std::string& key, const bool& val);
        Variant& insert(const std::string& key, const char& val);
        Variant& insert(const std::string& key, const int& val);
        Variant& insert(const std::string& key, const int64_t& val);
        Variant& insert(const std::string& key, const float& val);
        Variant& insert(const std::string& key, const double& val);
        Variant& insert(const std::string& key, const std::string& val);
        Variant& insert(const std::string& key, const char* val);

        void setAsCodeBlock(const std::string& source);


    private:
        typedef union
        {
            bool Bool;
            int Int;
            int64_t Long;
            float Float;
            double Double;
            std::string* String;
            ArrayType* Array;
            MapType* Map;
        } Var;

        VariantType type;
        Var value;
};
