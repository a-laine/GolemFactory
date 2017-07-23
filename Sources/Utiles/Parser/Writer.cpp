#include "Writer.h"
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>

void Writer::writeInFile(Variant &object, std::string file)
{
    std::ofstream strm(file.c_str(),std::ofstream::out | std::ofstream::trunc);
    if(!strm.is_open())
        throw std::invalid_argument("Writer::writeInFile : Cannot opening file");
    strm.precision(10);
    Writer writer(&strm);
    writer.write(object);
}

std::string Writer::writeInString(Variant &object)
{
    std::ostringstream strm(std::ostringstream::out);
    strm.precision(10);
    Writer writer(&strm);
    writer.write(object);
    return strm.str();
}


//******************************** Constructors *******************************//
Writer::Writer(std::ostream* output)
{
    ostr = 0;
    if(!output->good())
        throw std::logic_error("Writer::Writer : stream error");
    ostr = output;
	inlineArray = false;
}


//****************************** Public functions *******************************//
void Writer::setStream(std::ostream* output)
{
    json = false;
    ostr = 0;
    if(!output->good())
        throw std::logic_error("Writer::setStream : stream error");
    ostr = output;
}
void Writer::setOption(bool arrayInUniqueLine)
{
	inlineArray = arrayInUniqueLine;
}


void Writer::write(Variant &object)
{
    if(ostr == 0 || ostr->fail())
        throw std::logic_error("Writer::write : writing error");
    if(object.getType() == Variant::MAP && !json)
    {
        Variant::MapType::iterator it = object.getMap().begin();
        Variant::MapType::iterator itend = object.getMap().end();
        if(it!=itend)
        {
            *ostr << '\"' << it->first << "\" : ";
            writeVariant(it->second);
            for(++it ; it!=itend; ++it)
            {
                *ostr << ',' << std::endl;
                *ostr << '\"' << it->first << "\" : ";
                writeVariant(it->second);
            }
        }

    }
    else
    {
        writeVariant(object);
    }
    *ostr << std::endl;
}

void Writer::writeVariant(Variant& var,const int decal) const
{
    std::string decalStr((decal+1)*4,' ');
    switch(var.getType())
    {
        case Variant::ARRAY:
            {
                *ostr << '['; if(!inlineArray) *ostr << std::endl;
                Variant::ArrayType::iterator it = var.getArray().begin();
                Variant::ArrayType::iterator itend = var.getArray().end();
                if(it!=itend)
                {
					if (!inlineArray) *ostr << decalStr;
                    writeVariant(*it,decal+1);
                    for(++it; it!=itend; ++it)
                    {
                        *ostr << ',';
						if (!inlineArray) *ostr << std::endl;
						else *ostr << ' ';

						if (!inlineArray) *ostr << decalStr;
                        writeVariant(*it,decal+1);
                    }
                }

                *ostr << " ]";
            }
            break;
        case Variant::MAP:
            {
                *ostr << '{' << std::endl;
                Variant::MapType::iterator it = var.getMap().begin();
                Variant::MapType::iterator itend = var.getMap().end();
                if(it!=itend)
                {
                    *ostr << decalStr << '\"' << it->first << "\" : ";
                    writeVariant(it->second,decal+1);
                    for(++it; it!=itend; ++it)
                    {
                        *ostr << ',' << std::endl;
                        *ostr << decalStr << '\"' << it->first << "\" : ";
                        writeVariant(it->second,decal+1);
                    }
                }

                *ostr << " }";
            }
            break;
        case Variant::STRING:
            *ostr << '\"' << var.toString() << '\"';
            break;
        case Variant::CHAR:
            *ostr << '\"' << var.toChar() << '\"';
            break;
        case Variant::BOOL:
            if(var.toBool())
                *ostr << "true";
            else
                *ostr << "false";
            break;
        case Variant::INT:
            *ostr << var.toInt();
            break;
        case Variant::LONG:
            *ostr << var.toLong();
            break;
        case Variant::FLOAT:
            *ostr << var.toFloat();
            break;
        case Variant::DOUBLE:
            *ostr << var.toDouble();
            break;
        case Variant::NULLTYPE:
            *ostr << "null";
            break;
        default:
            *ostr << "null";
            break;
    }
}
