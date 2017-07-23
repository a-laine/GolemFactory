#pragma once

#include "Variant.h"
#include <ostream>
#include <string>

class Writer
{
    public:
		//	Default
		Writer(std::ostream* output);
		//

		//	Public functions
        static void writeInFile(Variant &object, std::string file);
        static std::string writeInString(Variant &object);
        void write(Variant &object);
		//

		//	Set/get functions
		void setStream(std::ostream* output);
		void setInlineArray(bool enable);
		void setInlineEmptyMap(bool enable);
		//

    private:
		//	Private functions
		void writeVariant(Variant& var, const int decal = 0) const;
		//

		//	Attributes
        std::ostream* ostr;
        bool json;
		bool inlineArray;
		bool inlineEmptyMap;
		//
};
