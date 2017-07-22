#pragma once

/*!
*	\file EventHandlerImpl.h
*	\brief Declaration of the common part between EventHandlerEnum and EventHandlerString.
*	\author Thibault LAINE
*/

#include <map>
#include <vector>
#include <algorithm>
#include <fstream>

#include "Utiles/Mutex.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Event.h"
#include "EventSequence.h"
#include "Utiles/Singleton.h"

/** \class EventHandlerImpl
*	\brief Base class for the event handler implementation.
*
*	The EventHandler manage all user input incoming in the application using GLFW specification.
*
*	The EventHandler manage Event object in an efficent way to catch user input.
*	An event can be listener of several Input, acording to the number of Input attached on it.
*	An input is defined by a GLFW callback and a key code or button code. See Event class for more detail.
*
*/
class EventHandlerImpl
{
    public:
        //  Miscellaneous
        typedef void(*ResizeCallback)(int,int);			//!< The resize callback type definition (for user utilisation)
        //

        //  Public functions
		/*!
		*	\brief Add a window to the event handler.
		*
		*	An application can have multiple window, so to track all user event the event handler need to have access to all of them.
		*	Just add the window and that's all !
		*
		*	\param window : Window to add
		*/
        void addWindow(GLFWwindow* window);

		/*!
		*	\brief Remove a window to the event handler.
		*
		*	Used to remove a window from the window list.
		*	Call this function before to delete the window to prevent crash !
		*	The function also detach all callback to the window.
		*
		*	\param window : Window to remove
		*/
        void removeWindow(GLFWwindow* window);

		/*!
		*	\brief The update function to call every frame.
		*
		*	This fonction perfom an update :
		*	- Get all event occured since the last update,
		*	- Generate the user event list,
		*	- Swap states for thread safe user access
		*
		*/
        void handleEvent();

		/*!
		*	\brief Remove all event declared
		*/
        virtual void clear();
        //

        //  Set/get functions
		/*!
		*	\brief Enable/disable the repeat functionality (see GLFW for more infos)
		*	\param enable : true to enable, false to disable
		*/
        void setRepeatMode(bool enable);

		/*!
		*	\brief Enable/disable the text entry functionality (see GLFW for more infos)
		*	\param enable : true to enable, false to disable
		*/
        void setTextInput(bool enable);

		/*!
		*	\brief Show/hide the cursor
		*	\param enable : true to show, false to hide
		*/
        void setCursorMode(bool enable);

		/*!
		*	\brief Define chord to high or low priority
		*
		*	If defined to high priority chord event will be publish before key pressed event.
		*	If defined to low chord event will be publish after key pressed event.
		*
		*	\param highPriority : true to high priority, false to low priority
		*/
        void setChordPriority(bool highPriority);

		/*!
		*	\brief Attach a user callback for all resize event on a window
		*
		*	Permit to specify a callback to call. Usefull for GUI update on this event for example.
		*
		*	\param cb : callback to attach
		*/
        void setResizeCallback(ResizeCallback cb);

		/*!
		*	\brief Return true if repeat mode is activated
		*	\return true if repeat mode is activated, false otherwise.
		*/
        bool getRepeatMode();

		/*!
		*	\brief Return true if text entry mode is activated
		*	\return true if text entry mode is activated, false otherwise.
		*/
        bool getTextInput();
        
		/*!
		*	\brief Return cursor visibility
		*	\return true if cursor is visible, false otherwise.
		*/
		bool getCursorMode();
        
		/*!
		*	\brief Return chords priority
		*	\return true if chords have high priority, false if chords have low priority.
		*/
		bool getChordPriority();

		/*!
		*	\brief Return cursor displacement from last update
		*	\return cursor displacement from last update.
		*/
        glm::vec2 getCursorPositionRelative();

		/*!
		*	\brief Return cursor position in window coordinates
		*	\return cursor position
		*/
		glm::vec2 getCursorPositionAbsolute();

		/*!
		*	\brief Return cursor position in viewport coordinates
		*	\return cursor position in viewport coordinates ([-1, 1] for each coordinates)
		*/
		glm::vec2 getCursorNormalizedPosition();

		/*!
		*	\brief Return scrolling displacement from last update
		*	\return scrolling displacement from last update.
		*/
		glm::vec2 getScrollingRelative();

		/*!
		*	\brief Change the directory of where to find event file configuration for key mapping loading
		*	\param path : directory to find event file configuration
		*/
		virtual void setRepository(std::string path);
        //

    protected:
        //  Miscellaneous
		/*!
		* \enum FlagsConfig
		* \brief All flags location for the configuration byte.
		*/
        enum FlagsConfig
        {
            REPEAT = 1<<0,						//!< Repeat mode flag location
            TEXTIN = 1<<1,						//!< Text entry mode flag location
            CURSOR_DISABLE = 1<<2,				//!< Cursor visibility flag location
            CHORD_HIGH_PRIORITY = 1<<3,			//!< Chord priority flag location
			FUTUR_USE = 1 << 4					//!< Flag used in herited class
        };
        //

        //  Default
		/*!
		*	\brief Constructor
		*	\param path : the directory to find event file configuration
		*/
        EventHandlerImpl(const std::string& path);

		/*!
		*  \brief Destructor
		*/
        virtual ~EventHandlerImpl();
        //

        //  Protected functions
		/*!
		*	\brief Add a simple event.
		*	\param event : Pointer on the event to add
		*	\param call : Type of the callback this event is listening
		*	\param key : The key the event is listening
		*/
        void addEvent(Event* event,Event::InputType call,int key);

		/*!
		*	\brief Remove an event to the handler.
		*	\param event : event to remove
		*/
        void removeEvent(Event* event);

		/*!
		*	\brief Virtual function to publish a user event (string or enum)
		*	\param event : event responsible of emission
		*/
        virtual void emitUserEvent(Event* e){};

		/*!
		*	\brief Swap the double buffered user event list
		*/
        virtual void swapFrameEventList(){};
		//

        //  Callbacks
		/*!
		*	\brief The keyboard callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		/*!
		*	\brief The text entered callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void charCallback(GLFWwindow* window, unsigned int codepoint);

		/*!
		*	\brief The mouse move callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

		/*!
		*	\brief The cursor enter in window callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void cursorEnterCallback(GLFWwindow* window, int entered);

		/*!
		*	\brief The mouse button callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		/*!
		*	\brief The mouse scrolling callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void scrollingCallback(GLFWwindow* window, double xoffset, double yoffset);

		/*!
		*	\brief The drag and drop in window callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void dropCallback(GLFWwindow* window, int count, const char** paths);

		/*!
		*	\brief The window gain/lose focus callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void windowFocusCallback(GLFWwindow* window, int focused);

		/*!
		*	\brief The window resize callback attach to all GLFW window
		*
		*	See GLFW documentation for more informations
		*
		*/
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        //

        //  Attributes
        uint8_t configuration;													//!< Configuration byte wher mode flags are packed
        std::string repository;													//!< The path directory to find key mapping configuration

        std::map<int,std::vector<Event*> > keyboardListeners;					//!< Keyboard listener event queue (one per key used)
        std::map<int,std::vector<Event*> > mouseButtonListeners;				//!< Mouse button listener event queue (one per button used)
        std::vector<Event*> charEnteredListeners;								//!< Text entry listener event queue
        std::vector<Event*> cursorPositionListeners;							//!< Mouse move listener event queue
        std::vector<Event*> cursorEnterListeners;								//!< Cursor enter in window listener event queue
        std::vector<Event*> scrollingListeners;									//!< Scrolling listener event queue
        std::vector<Event*> dragAndDropListeners;								//!< Drag and drop listener event queue

        std::vector<GLFWwindow*> windowList;									//!< Window list used by the application
        GLFWwindow* focusedWindow;												//!< A pointer on the window which have focus or nullptr if none have focus

        Mutex mutex;															//!< A mutex for thread safe use
		glm::vec2 cursorPositionRelative,cursorPositionRelativeBuffer;			//!< A double buffered vector for relative cursor position
		glm::vec2 cursorPositionAbsolute,cursorPositionAbsoluteBuffer;			//!< A double buffered vector for absolute cursor position
		glm::vec2 scrollingRelative,scrollingRelativeBuffer;					//!< A double buffered vector for relative scrolling position

        ResizeCallback resizeCallback;											//!< The callback for resize on a window

        static EventHandlerImpl* This;											//!< Needed for have access to attributes in static function (typicaly all callbacks)
        //
};
