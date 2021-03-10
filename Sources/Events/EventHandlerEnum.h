#pragma once

/*!
*	\file EventHandlerEnum.h
*	\brief Declaration of the EventHandlerEnum class.
*	\author Thibault LAINE
*/

#include <map>
#include <string>

#include <Utiles/Singleton.h>
#include "EventHandlerImpl.h"
//#include "EventEnum.h"

/** \class EventHandlerEnum
*	\brief Second part of the event handler implementation.
*
*	The EventHandlerEnum use a user enum as type of user event.
*
*/
class EventHandlerEnum : public EventHandlerImpl, public Singleton<EventHandlerEnum>
{
    friend class Singleton<EventHandlerEnum>;

    public:
        //  Public functions
		
		/*!
		*	\brief Load a new key mapping configuration.
		*
		*	Use the default repository path to find key mapping configuration file.
		*	This function is an overload of void loadKeyMapping(std::string path,std::string file);
		*
		*	\param file : The key mapping configuration file to load.
		*/
        void loadKeyMapping(const std::string& file, const std::string& enumFile);

        /*!
		*	\brief Load a new key mapping configuration.
		*
		*	Use the repository path specified to find key mapping configuration file, like this :
		*	"complete file name to load" = "repository string" + "file name string"
		*	
		*	A key mapping configuration is a list of user event.
		*
		*	\param path : The new repository location of key mapping configuration files.
		*	\param file : The key mapping configuration file to load.
		*/
		void loadKeyMapping(const std::string& path, std::string eventFilename, std::string enumFilename);

		/*!
		*	\brief Verify if an event is actually activated or not.
		*	\param eventName : The event to check. In user defined enumeration.
		*	\return true if event is activated, false otherwise
		*/
        bool isActivated(int eventName);

		/*!
		*	\brief Fill a container with all event published since last frame.
		*	\param buffer : A container to store events.
		*/
        void getFrameEvent(std::vector<int>& buffer);

		/*!
		*	\brief Explicitly publish an event.
		*
		*	Add the event to the event stack, which can be checked with getFrameEvent function.
		*	Usefull for customization because this class is a singleton class (accessed by almost every class everywhere)
		*
		*	\param literalEvent : Event to publish.
		*/
        void addFrameEvent(int literalEvent);

		/*!
		*	\brief Create and add a simple event.
		*
		*	The event have a unique input associate.
		*	Prefer the use of loading configuration file for more flexibility.
		*
		*	\param eventName : The user event associated.
		*	\param call : The callback type for the input
		*	\param key : The key for the input
		*	\param config : The configuration for the event
		*/
        void addEvent(int eventName,Event::InputType call,int key = -1,uint8_t config = Event::DOWN_FLAG|Event::BUTTON);

		/*!
		*	\brief Remove and delete a specific event
		*	\param eventName : the event to delete.
		*/
        void removeEvent(int eventName);

		/*!
		*	\brief Remove and delete all event handled
		*/
        void clear() override;
        //

        //  Set/get functions
		/*!
		*	\brief Return the number of event actually handled
		*	\return the number of event in the container
		*/
        unsigned int getNumberOfEvent();
        //

    private:
        //  Miscellaneous
		/*!
		* \enum FlagsConfig2
		* \brief Some complementary flags for the configuration byte herited from EventHandlerImpl class
		*/
        enum FlagsConfig2
        {
            CLEAR_EVENT_LIST = 1<<4		//!< If flag is not set in configuration byte the container frameEvent will be appened with the content of frameEventBuffer instead of a simple double buffer use
        };
        //

        //  Default
		/*!
		*	\brief Constructor
		*	\param path : the directory to find event file configuration
		*/
        EventHandlerEnum(const std::string& path = "Resources/");

		/*!
		*	\brief Destructor
		*/
        ~EventHandlerEnum();
        //

        //  Private functions
		/*!
		*	\brief Herited function from EventHandlerImpl class
		*/
        void emitUserEvent(Event* event) override;
        
		/*!
		*	\brief Herited function from EventHandlerImpl class
		*/
		void swapFrameEventList() override;
        //

        //  Attributes
        std::map<Event*, int> eventMapping;		//!< Container for assosiate Event object -> user event
        std::multimap<int,Event*> userMapping;	//!< Container for assosiate user event -> Event object
        std::vector<int> frameEvent;				//!< The list of all event occured and non treated yet
		std::vector<int> specialAddedEvent;
        std::vector<int> frameEventBuffer;		//!< A buffer used for thread safe uses
        //
};
