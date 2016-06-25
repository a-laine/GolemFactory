#pragma once

template <typename T>
class Singleton
{
    public :
        //  Default access
        static T* getInstance()
        {
            if (!This) This = new T;
            return This;
        }
        static void deleteInstance()
        {
            delete This;
            This = nullptr;
        }
        //

    protected :
        //  Default
        Singleton() {};
        Singleton(Singleton&) = delete;
        ~Singleton() {};
        //

        //  Attributes
        static T* This; //!< The current instance pointer
        //
};

template <typename T> T* Singleton<T>::This = nullptr;
