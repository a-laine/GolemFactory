#pragma once

#include "Variant.h"
#include <istream>
#include <string>


class Reader
{
    public:
        static void parseFile(Variant &result, std::string file);
        static void parseString(Variant &result, std::string text);

        Reader(std::istream* input);
        void setStream(std::istream* input);

        void parse(Variant &result);

    private:
        std::istream* ifs;
        int nbErrors;
        char charBuf;
        char comment;

        char nextChar();
        bool goodChar(char c) const;
        void readKey(std::string& key);
        bool readValue(Variant* exp);
        void readMap(Variant* vmap);
        void readArray(Variant* varray);
        std::string readString(char endChar, bool escape);
        void readNumber(Variant* num, std::string& str) const;
};
