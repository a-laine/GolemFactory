#pragma once

#include "Variant.h"
#include <istream>
#include <string>
#include <vector>


class Reader
{
    public:
        static void parseFile(Variant &result, std::string file);
        static void parseString(Variant &result, std::string text);

		explicit Reader(std::istream* input);
        void setStream(std::istream* input);

        void parse(Variant &result);

        std::vector<std::string>* codeBlocksKeys;

    private:
        std::istream* ifs;
        int nbErrors;
        char charBuf;
        char comment;

        char nextChar(const bool& removeSpaces = true);
        bool goodChar(char c) const;
        void readKey(std::string& key);
        bool readValue(Variant* exp, bool isCodeBlock = false);
        void readMap(Variant* vmap);
        void readArray(Variant* varray);
        std::string readString(char endChar, bool escape);
        void readNumber(Variant* num, std::string& str) const;
        void readCodeBlock(Variant* exp);
};
