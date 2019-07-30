#pragma once

/*!
* \file Singleton.h
* \brief Declaration of the Singleton class.
* \author Aurelien LAINE
*/

/** \class Singleton
*	\brief Simple Singleton class.
*
*	This the base class to have a unique object instanciate in the application.
*	The class use a static pointer to keep access to the unique object.
*	The object is realy instanciate in the first demand of the pointer via the getInstance function.
*
*/
template <typename T>
class Singleton
{
    public :
        //  Default access
		/*!
		*	\brief Get the pointer on instance for access.
		*
		*	If the instance is not created a call to constroctor is performed.
		*
		*/
        static T* getInstance()
        {
            if (!This) This = new T;
            return This;
        }

		/*!
		*	\brief Delete the instance.
		*/
        static void deleteInstance()
        {
            delete This;
            This = nullptr;
        }
        //

    protected :
        //  Default
		/*!
		*	\brief Constructor
		*
		*	Protected to prevent user instance creation via constructor.
		*	Use getInstance function instead.
		*
		*/
        Singleton() {};

		/*!
		*  \brief Constructor by copy
		*	
		*	Deleted function to keep object unique.
		*
		*/
        Singleton(Singleton&) = delete;

		/*!
		*	\brief Destructor
		*
		*	Protected to prevent user instance deletion via destructor.
		*	Use deleteInstance function instead but carefuly to prevent multiple creation via next call to getInstance.
		*
		*/
        ~Singleton() {};
        //

        //  Attributes
        static T* This;			//!< The current instance pointer
        //
};

template <typename T> T* Singleton<T>::This = nullptr; //!< A global pointer on instance (better than the instance itself !)
