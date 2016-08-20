#pragma once

/*!
 *	\file Event.h
 *	\brief Declaration of the Event class.
 *	\author Thibault LAINE
 */

#include <iostream>
#include <vector>
#include <utility>
#include <string>


/*! \class Event
 * \brief Base class for event implementation.
 *
 *	The EventHandler manage Event object in an efficent way to catch user input.
 *  An Event can be attached to several user input to generate complex event, for example double click or combo sequence.
 *  This is a virtual class so special Event is inherited from this class
 *
 */
class Event
{
    public:
        //  Miscellaneous
		/*!
		 *	\enum EventType
		 *	\brief Used to diferenciate event type.
		 *
		 *	Use this to generate a configuration byte for an Event
		 *
		 */
        enum EventType
        {
            BUTTON = 1,					//!< Event type button from keyboard or mouse
            CHORD = 2,					//!< Event type chord (when multiple button are silmutaneously pressed)
            SEQUENCE = 3,				//!< Event type sequence (when multiple button are pressed in special order)
            GESTURE = 4,				//!< Event type for gesture recognition patern [not yet implemented]
            TYPE_MASK = 0x07,			//!< Mask used for extract event type

            UP_FLAG = 1<<3,				//!< Flag for raise event on released (EventHandler will publish an event)
            DOWN_FLAG = 1<<4,			//!< Flag for raise event on pressed (EventHandler will publish an event)
            ACTIVATED_FLAG = 1<<5		//!< Flag to keep event state
        };
		
		/*!
		*	\enum InputType
		*	\brief Used to diferenciate input.
		*
		*	An Input is principaly associated to a callback (see GLFW documentation for more detail about callback)
		*	And the InputType enumeration permit to differenciate this diferent callback
		*
		*/
        enum InputType
        {
            KEY = 1,					//!< Input of type Keyboard key pressed or released
            CHAR = 2,					//!< Input of type char entered (for text entry)

            CURSORPOS = 4,				//!< Input of type mouse moved
            CURSORENTER = 5,			//!< Input of type mouse cursor enter in window
            MOUSEBUTTON = 6,			//!< Input of type mouse button pressed or released
            SCROLLING = 7,				//!< Input of type mouse scrolling

            DRAGANDDROP = 8				//!< Input of type a drag and drop operation performed on window
        };

		/*!
		 *	\struct Input
		 *	\brief An input for event. Regroup an input type and a key code (or button code)
		 */
        struct Input
        {
            InputType callback;			//!< The type of callback related to the input. For more info see #InputType.
            int key;					//!< The key code (or button code)
        };
        //

        //  Default
		/*!
		 *  \brief Constructor
		 *  \param config : the event configuration (see #configuration.)
		 */
        Event(uint8_t config = 0x00);

		/*!
		*  \brief Destructor
		*/
        virtual ~Event();
        //

        //  Public functions
		/*!
		 *	\brief Function to check if the event is actualy activated.
		 *
		 *	Simply extract the activated flag from the configuration byte.
		 *
		 *	\return true if activated, false otherwise
		 */
        virtual bool isActivated() const;

		/*!
		 *	\brief Update function called by the EventHandler.
		 *
		 *	If the event is configured with an UP_FLAG, the EventHandler will publish a user event when this event becomes activated.
		 *	If the event is configured with an DOWN_FLAG, the EventHandler will publish a user event when this event becomes unactivated.
		 *
		 *	\param call : Type of the callback that generate this update
		 *	\param key : The key code of the input
		 *	\param action : A second parameter of the input (ex : for a button it's for pressed/released)
		 *	\return true if a user event publish is needed (up/down flags in #configuration. byte), false otherwise
		 */
        virtual bool check(InputType call,int key,int action);

		/*!
		*	\brief Update function called by the EventHandler.
		*
		*	If the event is configured with an UP_FLAG, the EventHandler will publish a user event when this event becomes activated.
		*	If the event is configured with an DOWN_FLAG, the EventHandler will publish a user event when this event becomes unactivated.
		*
		*	\param in : The Input responsible of this update
		*	\param action : A second parameter of the input (ex : for a button it's for pressed/released)
		*	\return true if a user event publish is needed (up/down flags in #configuration. byte), false otherwise
		*/
        virtual bool check(Input in,int action);

		/*!
		*	\brief Attach an input to the event.
		*
		*	An Event can be attached to several user input to generate complex event, for example double click or combo sequence.
		*
		*	\param in : The Input to attach
		*/
        void addInput(Input in);

		/*!
		*	\brief Attach an input to the Event.
		*
		*	Create an input with in and key and attach it to the event.
		*	An Event can be attached to several user input to generate complex event, for example double click or combo sequence.
		*
		*	\param call : The InputType of the input
		*	\param key : The key of the input
		*/
        void addInput(InputType call,int key);
        //

        //  Attributes
        uint8_t configuration;							//!< A configuration byte. To unpack variable see the enumeration type #EventType.
        std::vector<std::pair<Input,bool> > inputList;	//!< The list of all input attached to the event. the boolean is to track the input state (true = activated)
        //
};
