#include <stdexcept>
#include <cstring>

#include "Texture.h"
#include "Utiles/Parser/Reader.h"
#include "Loader/ImageLoader.h"

//  Static attributes
std::string Texture::extension = ".texture";
//

//  Default
Texture::Texture(std::string path,std::string textureName,uint8_t conf) :
    ResourceVirtual(path,textureName,ResourceVirtual::TEXTURE),configuration(conf)
{
    //  Initialization
    texture = 0;
    std::string errorLog = "An exception occur in Texture::Texture() of "+textureName+" :\n";

    //  Simple 2D texture case (no information file)
    if((configuration&TYPE_MASK)==TEXTURE_2D)
    {
        //  Loading the image
        int x,y,n;
        uint8_t* image = ImageLoader::loadFromFile(path+textureName,x,y,n,ImageLoader::RGB_ALPHA);
        if(!image) {std::cout<<"\tfail loading texture image"<<std::endl;return;}

        //  Generate opengl texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D,0);
        ImageLoader::freeImage(image);
        if(!glIsTexture(texture)) return;

        size.x = (float)x; size.y = (float)y; size.z = 0.f;
    }
    else    //  Texture type is not 2D or not specify
    {
        //  Initialization
        Variant v;
        Variant* tmp = NULL;

        try { Reader::parseFile(v, path + textureName + extension);
              tmp = &(v.getMap().begin()->second);                      }
        catch(std::exception&) {std::cout<<errorLog<<"parser error"<<std::endl;return;}
        Variant& textureInfo = *tmp;

        //  Configuration
        try { if(textureInfo["type"].toString() == "D3")      configuration = TEXTURE_1D;
              else if(textureInfo["type"].toString() == "D2") configuration = TEXTURE_2D;
              else if(textureInfo["type"].toString() == "D3") configuration = TEXTURE_3D;
              else throw std::string("Texture::Texture : unknown texture type");                        }
        catch(std::exception& e){std::cout<<errorLog<<e.what()<<std::endl;return;}

        try{if(textureInfo["mipmap"].toBool()) configuration |= USE_MIPMAP;}      catch(std::exception){}
        try{if(textureInfo["minnearest"].toBool()) configuration |= MIN_NEAREST;} catch(std::exception){}
        try{if(textureInfo["magnearest"].toBool()) configuration |= MAG_NEAREST;} catch(std::exception){}

        try { if(textureInfo["wrap"].toString() == "repeat")      configuration |= WRAP_REPEAT;
              else if(textureInfo["wrap"].toString() == "mirror") configuration |= WRAP_MIRROR;  }
        catch(std::exception){}

        //  Get layers & generate texture data
        uint8_t* textureData = nullptr;
        uint8_t* dataEnd = nullptr;

        try
        {
            if((configuration&TYPE_MASK)==TEXTURE_2D && textureInfo["texture"].getType()==Variant::STRING)//texture="a.png";
            {
                int x,y,n;
                textureData = ImageLoader::loadFromFile(path + textureInfo["texture"].toString(),x,y,n,ImageLoader::RGB_ALPHA);
                if(!textureData) throw std::runtime_error("fail loading 2D texture");
                dataEnd = textureData;

                size.x = (float)x;
                size.y = (float)y;
                size.z = 0.f;
            }
            else if(textureInfo["texture"].getType()==Variant::ARRAY && textureInfo["texture"][0].getType()==Variant::STRING)//texture=["a.png","b.png"];
            {
                configuration &= ~TYPE_MASK;
                configuration |= TEXTURE_3D;
                size.x = (float)textureInfo["width"].toInt();
                size.y = (float)textureInfo["height"].toInt();
                size.z = (float) textureInfo["texture"].size();

                textureData = new uint8_t[4*(int)(size.x*size.y*size.z)];
                if(!textureData) throw std::runtime_error("fail init texture ptr");
                dataEnd = textureData;

                int x,y,n;
                uint8_t* image = nullptr;

                for(unsigned int i=0;i<textureInfo["texture"].size();i++)
                {
                    image = ImageLoader::loadFromFile(path+textureInfo["texture"][i].toString(),x,y,n,ImageLoader::RGB_ALPHA);
                    if(!image)
                        throw std::runtime_error("fail loading 3D texture");
                    else if(x!=(int)size.x || y!=(int)size.y)
                        throw std::runtime_error("wrong 2D texture size in 3D texture layer description");
                    memcpy(dataEnd,image,4*x*y);
                    dataEnd += 4*x*y;
                    ImageLoader::freeImage(image);
                }
            }
            else if(textureInfo["texture"].getType()==Variant::ARRAY && textureInfo["texture"][0].getType()==Variant::STRING)//texture=[0,0,0,255 , 255,0,0,255];
            {
                unsigned int n = textureInfo["width"].toInt();
				size.x = (float)n;
                if((configuration&TYPE_MASK)==TEXTURE_2D) { size.y = (float)textureInfo["height"].toInt(); n *= textureInfo["height"].toInt(); }
                if((configuration&TYPE_MASK)==TEXTURE_3D) { size.z = (float)textureInfo["depth"].toInt();  n *= textureInfo["depth"].toInt();  }

                textureData = new uint8_t[4*n];
                if(!textureData) throw std::runtime_error("error allocation array");
                dataEnd = textureData;

                for(unsigned int i=0;i<textureInfo["texture"].size();i++)
                {
                    if(i>=4*n) throw std::runtime_error("invalid width, height, depth");
                    textureData[i] = (uint8_t) (textureInfo["texture"][0].toInt());
                }
            }
            else throw std::runtime_error("fail to parse file description");
        }
        catch(std::exception& e)
        {
            std::cout<<errorLog<<e.what()<<std::endl;
            if(dataEnd && textureData) delete[] textureData;
            return;
        }

        //  Generate & load texture
        glGenTextures(1,&texture);
        switch(configuration&TYPE_MASK)
        {
            case TEXTURE_1D:
                glBindTexture(GL_TEXTURE_1D,texture);
                glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, (int)size.x,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, textureData );
                if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_1D);
                break;
            case TEXTURE_2D:
                glBindTexture(GL_TEXTURE_2D,texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)size.x, (int)size.y,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
                if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_2D);
                break;
            case TEXTURE_3D:
                glBindTexture(GL_TEXTURE_3D,texture);
                glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, (int)size.z, (int)size.x, (int)size.y,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, textureData );
                if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_3D);
                break;
            default:
                glDeleteTextures(1,&texture);
                texture = 0;
                if(dataEnd && textureData) delete[] textureData;
                return;
        }

        //  MAG & MIN filter parameter
        if(configuration&USE_MIPMAP)
        {
            if(configuration&MAG_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            if(configuration&MIN_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            if(configuration&MAG_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if(configuration&MIN_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        //  WRAP parameter
        switch(configuration&WRAP_MASK)
        {
            case WRAP_CLAMP:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                break;
            case WRAP_REPEAT:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
                break;
            case WRAP_MIRROR:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
                break;
        }

        //  End
        if(textureData) delete[] textureData;
        glBindTexture(GL_TEXTURE_2D,0);
    }
}
Texture::~Texture()
{
    glDeleteTextures(1,&texture);
}

bool Texture::isValid() const { return true; glIsTexture(texture); }
//

//  Set/get functions
int Texture::getType() { return (configuration&TYPE_MASK); }
GLenum Texture::getGLenumType()
{
    switch(configuration&TYPE_MASK)
    {
        case TEXTURE_1D: return GL_TEXTURE_1D;
        case TEXTURE_2D: return GL_TEXTURE_2D;
        case TEXTURE_3D: return GL_TEXTURE_3D;
        default: return TEXTURE_2D;
    }
}
//

//  Public functions
GLuint Texture::getTextureId() const{return texture;}
GLuint* Texture::getTextureIdPt(){return &texture;}
//
