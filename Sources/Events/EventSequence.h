#pragma once

/*!
*	\file EventSequence.h
*	\brief Declaration of the EventSequence class.
*	\author Thibault LAINE
*/

#include "Event.h"

#include <GLFW/glfw3.h>

/** \class EventSequence
*	\brief The class for sequence event.
*
*	A sequence is an event which becomes activated when several button and/or key are activated in a special order.
*	Useful for implementing attack like A+A+A+B+C+Left in combat video game.
*
*/
class EventSequence : public Event
{
    public:
        //  Default
		/*!
		*  \brief Constructor
		*
		*	Constructor of the class
		*
		*/
        EventSequence();

		/*!
		*  \brief Destructor
		*
		*	Destructor of the class
		*
		*/
        ~EventSequence();
        //

        //  Public functions
		/*!
		*	\brief Function to check if the event is actualy activated.
		*
		*	Always return false because a sequence is activated just at the end of the sequence.
		*	The EventHandler will always publish a user event when this event becomes activated.
		*
		*	\return false.
		*/
        bool isActivated() const override;

		/*!
		*	\brief Update function called by the EventHandler.
		*
		*	Same as the "check" function of Event class.
		*	If the event is configured with an UP_FLAG, the EventHandler will publish a user event when this event becomes activated.
		*	If the event is configured with an DOWN_FLAG, the EventHandler will publish a user event when this event becomes unactivated.
		*
		*	\param call : Type of the callback that generate this update
		*	\param key : The key code of the input
		*	\param action : A second parameter of the input (ex : for a button it's for pressed/released)
		*	\return true if a user event publish is needed (up/down flags in #configuration. byte), false otherwise
		*/
        bool check(InputType call,int key,int action) override;
        //

        //  Attributes
        static float timeout;	//!<	The timeout common to all sequence event
        //

    protected:
        //  Attributes
        uint8_t state;			//!<	The state of the sequence (which Input is needed for the next check)
        float lastCheckTime;	//!<	The last check time (used to check if the input is reached in time or not)
        //
};
