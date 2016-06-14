#pragma once

#include "Variant.h"
#include <ostream>
#include <string>

class Writer
{
    public:
        static void writeInFile(Variant &object, std::string file);
        static std::string writeInString(Variant &object);

        Writer(std::ostream* output);
        void setStream(std::ostream* output);

        void write(Variant &object);

    private:
        std::ostream* ostr;
        bool json;

        void writeVariant(Variant& var,const int decal=0) const;
};
