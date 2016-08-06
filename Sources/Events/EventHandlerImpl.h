#pragma once

#include <map>
#include <vector>
#include <algorithm>

#include "Utiles/Mutex.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Event.h"
#include "EventSequence.h"
#include "Utiles/Singleton.h"

class EventHandlerImpl
{
    public:
        //  Miscellaneous
        typedef void(*ResizeCallback)(int,int);
        //

        //  Public functions
        void addWindow(GLFWwindow* window);
        void removeWindow(GLFWwindow* window);

        void handleEvent();
        virtual void clear();
        //

        //  Set/get functions
        void setRepeatMode(bool enable);
        void setTextInput(bool enable);
        void setCursorMode(bool enable);
        void setChordPriority(bool highPriority);
        void setResizeCallback(ResizeCallback cb);

        bool getRepeatMode();
        bool getTextInput();
        bool getCursorMode();
        bool getChordPriority();

        glm::vec2 getCursorPositionRelative();
		glm::vec2 getCursorPositionAbsolute();
		glm::vec2 getCursorNormalizedPosition();
		glm::vec2 getScrollingRelative();

		virtual void setPath(std::string path) {};
        //

    protected:
        //  Miscellaneous
        enum FlagsConfig
        {
            REPEAT = 1<<0,
            TEXTIN = 1<<1,
            CURSOR_DISABLE = 1<<2,
            CHORD_HIGH_PRIORITY = 1<<3
        };
        //

        //  Default
        EventHandlerImpl(std::string path);
        virtual ~EventHandlerImpl();
        //

        //  Protected functions
        void addEvent(Event* event,Event::InputType call,int key);
        void removeEvent(Event* event);

        virtual void emitUserEvent(Event* e){};
        virtual void swapFrameEventList(){};
        //

        //  Callbacks
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void charCallback(GLFWwindow* window, unsigned int codepoint);
        static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        static void cursorEnterCallback(GLFWwindow* window, int entered);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void scrollingCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void dropCallback(GLFWwindow* window, int count, const char** paths);

        static void windowFocusCallback(GLFWwindow* window, int focused);
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        //

        //  Attributes
        uint8_t configuration;
        std::string repository;

        std::map<int,std::vector<Event*> > keyboardListeners;
        std::map<int,std::vector<Event*> > mouseButtonListeners;
        std::vector<Event*> charEnteredListeners;
        std::vector<Event*> cursorPositionListeners;
        std::vector<Event*> cursorEnterListeners;
        std::vector<Event*> scrollingListeners;
        std::vector<Event*> dragAndDropListeners;

        std::vector<GLFWwindow*> windowList;
        GLFWwindow* focusedWindow;

        Mutex mutex;
		glm::vec2 cursorPositionRelative,cursorPositionRelativeBuffer;
		glm::vec2 cursorPositionAbsolute,cursorPositionAbsoluteBuffer;
		glm::vec2 scrollingRelative,scrollingRelativeBuffer;

        ResizeCallback resizeCallback;

        static EventHandlerImpl* This;
        //
};
