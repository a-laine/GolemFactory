#include "EventHandlerImpl.h"

#include <Core/RenderContext.h>


//  Static attributes
EventHandlerImpl* EventHandlerImpl::This = nullptr;
//

//  Default
EventHandlerImpl::EventHandlerImpl(const std::string& path) :
	configuration(0x00), repository(path), focusedWindow(nullptr),
	cursorPositionRelative(0), cursorPositionRelativeBuffer(0), cursorPositionAbsolute(0), cursorPositionAbsoluteBuffer(0),
	scrollingRelative(0), scrollingRelativeBuffer(0)
{
    This = this;
}
EventHandlerImpl::~EventHandlerImpl(){}
//

//  Public functions
void EventHandlerImpl::addWindow(GLFWwindow* window)
{
    if(!window) return;

	//	add window to the window list and set the window cursor
    mutex.lock();
    if(windowList.empty()) focusedWindow = window;
    windowList.push_back(window);
    if(configuration&CURSOR_DISABLE) glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    else glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    mutex.unlock();

	//	attach callback to the window
    KeyCallback keyCb = glfwSetKeyCallback(window,EventHandlerImpl::keyCallback);
    TextInputCallback charCb = glfwSetCharCallback(window,EventHandlerImpl::charCallback);
    CursorPositionCallback curposCb = glfwSetCursorPosCallback(window,EventHandlerImpl::cursorPositionCallback);
    CursorEnterCallback cursorCb = glfwSetCursorEnterCallback(window,EventHandlerImpl::cursorEnterCallback);
    MouseButtonCallback mouseButCb = glfwSetMouseButtonCallback(window,EventHandlerImpl::mouseButtonCallback);
    ScrollingCallback scrollCb = glfwSetScrollCallback(window,EventHandlerImpl::scrollingCallback);
    DropCallback dropCb = glfwSetDropCallback(window,EventHandlerImpl::dropCallback);
    WindowFocusCallback focusCb = glfwSetWindowFocusCallback(window,EventHandlerImpl::windowFocusCallback);
    ResizeCallback resizeCb = glfwSetFramebufferSizeCallback(window,EventHandlerImpl::framebufferResizeCallback);

    // save already present ones as forwarding callbacks
    if (keyCb)
        addKeyCallback(keyCb);
    if (charCb)
        addTextInputCallback(charCb);
    if (curposCb)
        addCursorPositionCallback(curposCb);
    if (cursorCb)
        addCursorEnterCallback(cursorCb);
    if (mouseButCb)
        addMouseButtonCallback(mouseButCb);
    if (scrollCb)
        addScollingCallback(scrollCb);
    if (dropCb)
        addDropCallback(dropCb);
    if (focusCb)
        addWindowFocusCallback(focusCb);
    if (resizeCb)
        addResizeCallback(resizeCb);
}
void EventHandlerImpl::removeWindow(GLFWwindow* window)
{
    if(!window) return;

	//	remove windo from list
    mutex.lock();
    auto it = std::find(windowList.begin(),windowList.end(),window);
    if(it==windowList.end())
    {
        mutex.unlock();
        return;
    }
    windowList.erase(it);
    mutex.unlock();

	//	detach window callback
    glfwSetKeyCallback(window,NULL);
    glfwSetCharCallback(window,NULL);
    glfwSetCursorPosCallback(window,NULL);
    glfwSetCursorEnterCallback(window,NULL);
    glfwSetMouseButtonCallback(window,NULL);
    glfwSetScrollCallback(window,NULL);
    glfwSetDropCallback(window,NULL);

    glfwSetWindowFocusCallback(window,NULL);
}


void EventHandlerImpl::handleEvent()
{
	//	init
    cursorPositionRelativeBuffer = vec2f(0,0);
    scrollingRelativeBuffer = vec2f(0,0);

	//	process all event (call the callbacks)
    glfwPollEvents();

	//	swap state to give access to the user
    mutex.lock();
		swapFrameEventList();
		cursorPositionRelative = cursorPositionRelativeBuffer;
		cursorPositionAbsolute = cursorPositionAbsoluteBuffer;
		scrollingRelative = scrollingRelativeBuffer;
    mutex.unlock();
}
void EventHandlerImpl::clear()
{
	//	Remove all event from queues
    keyboardListeners.clear();
    mouseButtonListeners.clear();
    charEnteredListeners.clear();
    cursorPositionListeners.clear();
    cursorEnterListeners.clear();
    scrollingListeners.clear();
    dragAndDropListeners.clear();
}
//

//  Set/get functions
void EventHandlerImpl::setRepository(std::string path)
{
	repository = path;
}
void EventHandlerImpl::setRepeatMode(bool enable)
{
    mutex.lock();
    if(enable)configuration |= REPEAT;
    else configuration &= ~REPEAT;
    mutex.unlock();
}
void EventHandlerImpl::setTextInput(bool enable)
{
    mutex.lock();
    if(enable)configuration |= TEXTIN;
    else configuration &= ~TEXTIN;
    mutex.unlock();
}
void EventHandlerImpl::setCursorMode(bool enable)
{
    mutex.lock();
    for(unsigned int i=0;i<windowList.size();i++)
    {
        if(enable) glfwSetInputMode(windowList[i],GLFW_CURSOR,GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(windowList[i],GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    }
    if(enable) configuration &= ~CURSOR_DISABLE;
    else configuration |= CURSOR_DISABLE;
    mutex.unlock();
}
void EventHandlerImpl::setChordPriority(bool highPriority)
{
    mutex.lock();
    if(highPriority) configuration |= CHORD_HIGH_PRIORITY;
    else configuration &= ~CHORD_HIGH_PRIORITY;
    mutex.unlock();
}


void EventHandlerImpl::addResizeCallback(ResizeCallback cb) { m_resizeCallbacks.push_back(cb); }
void EventHandlerImpl::addTextInputCallback(TextInputCallback cb) { m_textInputCallbacks.push_back(cb); }
void EventHandlerImpl::addScollingCallback(ScrollingCallback cb) { m_scrollingCallbacks.push_back(cb); }
void EventHandlerImpl::addKeyCallback(KeyCallback cb) { m_keyCallbacks.push_back(cb); }
void EventHandlerImpl::addCursorPositionCallback(CursorPositionCallback cb) { m_cursorPositionCallbacks.push_back(cb); }
void EventHandlerImpl::addCursorEnterCallback(CursorEnterCallback cb) { m_cursorEnterCallbacks.push_back(cb); }
void EventHandlerImpl::addMouseButtonCallback(MouseButtonCallback cb) { m_mouseButtonCallbacks.push_back(cb); }
void EventHandlerImpl::addDropCallback(DropCallback cb) { m_dropCallbacks.push_back(cb); }
void EventHandlerImpl::addWindowFocusCallback(WindowFocusCallback cb) { m_windowFocusCallbacks.push_back(cb); }


bool EventHandlerImpl::getRepeatMode()
{
    bool b;
    mutex.lock();
    b = configuration&REPEAT;
    mutex.unlock();
    return b;
}
bool EventHandlerImpl::getTextInput()
{
	bool b;
    mutex.lock();
	b = (configuration&TEXTIN) != 0;
    mutex.unlock();
    return b;
}
bool EventHandlerImpl::getCursorMode()
{
    bool b;
    mutex.lock();
    b = (configuration&CURSOR_DISABLE) == 0;
    mutex.unlock();
    return b;
}
bool EventHandlerImpl::getChordPriority()
{
	bool b;
    mutex.lock();
	b = (configuration&CHORD_HIGH_PRIORITY) != 0;
    mutex.unlock();
    return b;
}


vec2f EventHandlerImpl::getCursorPositionRelative()
{
	vec2f position;
    mutex.lock();
    position = cursorPositionRelative;
    mutex.unlock();
    return position;
}
vec2f EventHandlerImpl::getCursorPositionAbsolute()
{
	vec2f position;
    mutex.lock();
    position = cursorPositionAbsolute;
    mutex.unlock();
    return position;
}
vec2f EventHandlerImpl::getCursorNormalizedPosition()
{
	vec2f position = vec2f(0, 0);
    mutex.lock();
    if(focusedWindow)
    {
        int width, height;
        glfwGetFramebufferSize(focusedWindow,&width,&height);
        if (width > 0 && height > 0)
        {
            position.x = (2.f * cursorPositionAbsolute.x - width) / width;
            position.y = (height - 2.f * cursorPositionAbsolute.y) / height;
        }
    }
    mutex.unlock();
    return position;
}
vec2f EventHandlerImpl::getScrollingRelative()
{
	vec2f scroll;
    mutex.lock();
    scroll = scrollingRelative;
    mutex.unlock();
    return scroll;
}
//

//  Protected functions
void EventHandlerImpl::addEvent(Event* event,Event::InputType call,int key)
{
    if(!event) return;
    switch(call)
    {
        case Event::InputType::KEY:            keyboardListeners[key].push_back(event);     break;
        case Event::InputType::MOUSEBUTTON:    mouseButtonListeners[key].push_back(event);  break;
        case Event::InputType::CHAR:           charEnteredListeners.push_back(event);       break;
        case Event::InputType::CURSORPOS:      cursorPositionListeners.push_back(event);    break;
        case Event::InputType::CURSORENTER:    cursorEnterListeners.push_back(event);       break;
        case Event::InputType::SCROLLING:      scrollingListeners.push_back(event);         break;
        case Event::InputType::DRAGANDDROP:    dragAndDropListeners.push_back(event);       break;
        default: return;
    }
}
void EventHandlerImpl::removeEvent(Event* event)
{
    if(!event) return;

    for(unsigned int i=0;i<event->inputList.size();i++)
    {
        switch(event->inputList[i].first.callback)
        {
            case Event::InputType::KEY:
                try
                {
                    auto eventList = keyboardListeners.at(event->inputList[i].first.key);
                    auto it = std::find(eventList.begin(),eventList.end(),event);
                    if(it != eventList.end()) eventList.erase(it);
                }
                catch(std::out_of_range){}
                break;
            case Event::InputType::MOUSEBUTTON:
                try
                {
                    auto eventList = mouseButtonListeners.at(event->inputList[i].first.key);
                    auto it = std::find(eventList.begin(),eventList.end(),event);
                    if(it != eventList.end()) eventList.erase(it);
                }
                catch(std::out_of_range){}
                break;
           case Event::InputType::CHAR:
                {
                    auto it = std::find(charEnteredListeners.begin(),charEnteredListeners.end(),event);
                    if(it!=charEnteredListeners.end()) charEnteredListeners.erase(it);
                }
                break;
           case Event::InputType::CURSORPOS:
                {
                    auto it = std::find(cursorPositionListeners.begin(),cursorPositionListeners.end(),event);
                    if(it!=cursorPositionListeners.end()) cursorPositionListeners.erase(it);
                }
                break;
           case Event::InputType::CURSORENTER:
                {
                    auto it = std::find(cursorEnterListeners.begin(),cursorEnterListeners.end(),event);
                    if(it!=cursorEnterListeners.end()) cursorEnterListeners.erase(it);
                }
                break;
           case Event::InputType::SCROLLING:
                {
                    auto it = std::find(scrollingListeners.begin(),scrollingListeners.end(),event);
                    if(it!=scrollingListeners.end()) scrollingListeners.erase(it);
                }
                break;
           case Event::InputType::DRAGANDDROP:
                {
                    auto it = std::find(dragAndDropListeners.begin(),dragAndDropListeners.end(),event);
                    if(it!=dragAndDropListeners.end()) dragAndDropListeners.erase(it);
                }
                break;
           default:
                std::cerr<<"EventHandler : Error in removing event:"<<std::endl;
                std::cerr<<"               The callback type is unknown."<<std::endl;
                break;
        }
    }
}
//

//  Callbacks
void EventHandlerImpl::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    for (auto& cb : This->m_keyCallbacks)
        (*cb)(window, key, scancode, action, mods);

    if(!((This->configuration)&REPEAT) && action==GLFW_REPEAT) return;

    try
    {
        auto events = This->keyboardListeners.at(key); 
		if ((This->configuration&CHORD_HIGH_PRIORITY))
		{
			//	first pass process chord
			for(unsigned int i=0;i<events.size();i++)	
			{
				if ((events[i]->configuration & Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::InputType::KEY, key, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process !chord
			for(unsigned int i=0;i<events.size();i++)	
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::InputType::KEY, key, action))
					This->emitUserEvent(events[i]);
			}
		}
		else 
		{
			//	first pass process !chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::InputType::KEY, key, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration & Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::InputType::KEY, key, action))
					This->emitUserEvent(events[i]);
			}
		}
    }
    catch(std::out_of_range){}
}
void EventHandlerImpl::charCallback(GLFWwindow* window, unsigned int codepoint)
{
    for (auto& cb : This->m_textInputCallbacks)
        (*cb)(window, codepoint);

    if(!((This->configuration)&TEXTIN)) return;
    bool chord = false;
    for(unsigned int i=0;i<This->charEnteredListeners.size();i++)
    {
        if((This->charEnteredListeners[i]->configuration & Event::TYPE_MASK)==Event::CHORD &&
           This->charEnteredListeners[i]->check(Event::InputType::CHAR,codepoint,-1))
        {
            chord = true;
            This->emitUserEvent(This->charEnteredListeners[i]);
        }
    }
    if((This->configuration&CHORD_HIGH_PRIORITY) && chord) return;
    for(unsigned int i=0;i<This->charEnteredListeners.size();i++)
    {
        if((This->charEnteredListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD &&
           This->charEnteredListeners[i]->check(Event::InputType::CHAR,codepoint,-1))
            This->emitUserEvent(This->charEnteredListeners[i]);
    }
}
void EventHandlerImpl::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    for (auto& cb : This->m_cursorPositionCallbacks)
        (*cb)(window, xpos, ypos);

    This->cursorPositionRelativeBuffer.x = (float)xpos - This->cursorPositionAbsoluteBuffer.x;
    This->cursorPositionRelativeBuffer.y = (float)ypos - This->cursorPositionAbsoluteBuffer.y;
    This->cursorPositionAbsoluteBuffer.x = (float)xpos;
    This->cursorPositionAbsoluteBuffer.y = (float)ypos;

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->cursorPositionListeners.size();i++)
		{
			if((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->cursorPositionListeners[i]->check(Event::InputType::CURSORPOS,-1,-1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->cursorPositionListeners.size();i++)
		{
			if((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->cursorPositionListeners[i]->check(Event::InputType::CURSORPOS,-1,-1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->cursorPositionListeners.size(); i++)
		{
			if ((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorPositionListeners[i]->check(Event::InputType::CURSORPOS, -1, -1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
		
		//	second pass process chord
		for (unsigned int i = 0; i<This->cursorPositionListeners.size(); i++)
		{
			if ((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorPositionListeners[i]->check(Event::InputType::CURSORPOS, -1, -1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
	}
    
}
void EventHandlerImpl::cursorEnterCallback(GLFWwindow* window, int entered)
{
    for (auto& cb : This->m_cursorEnterCallbacks)
        (*cb)(window, entered);

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorEnterListeners[i]->check(Event::InputType::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}

		//	second pass process !chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorEnterListeners[i]->check(Event::InputType::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorEnterListeners[i]->check(Event::InputType::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorEnterListeners[i]->check(Event::InputType::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}
	}
}
void EventHandlerImpl::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    for (auto& cb : This->m_mouseButtonCallbacks)
        (*cb)(window, button, action, mods);

    auto it = This->mouseButtonListeners.find(button);
    if (it == This->mouseButtonListeners.end())
        return;

    auto& events = it->second;
	if ((This->configuration & CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<events.size();i++)
		{
			if ((events[i]->configuration&Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::InputType::MOUSEBUTTON, button, action))
				This->emitUserEvent(events[i]);
		}

		//	second pass process !chord
		for (unsigned int i = 0; i < events.size(); i++)
		{
			if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::InputType::MOUSEBUTTON, button, action))
				This->emitUserEvent(events[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<events.size(); i++)
		{
			if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::InputType::MOUSEBUTTON, button, action))
				This->emitUserEvent(events[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i<events.size(); i++)
		{
			if ((events[i]->configuration&Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::InputType::MOUSEBUTTON, button, action))
				This->emitUserEvent(events[i]);
		}
	}
}
void EventHandlerImpl::scrollingCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    for (auto& cb : This->m_scrollingCallbacks)
        (*cb)(window, xoffset, yoffset);

    This->scrollingRelativeBuffer.x = (float)xoffset;
    This->scrollingRelativeBuffer.y = (float)yoffset;

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->scrollingListeners.size();i++)
		{
			if((This->scrollingListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->scrollingListeners[i]->check(Event::InputType::SCROLLING,-1,-1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->scrollingListeners.size();i++)
		{
			if((This->scrollingListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->scrollingListeners[i]->check(Event::InputType::SCROLLING,-1,-1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->scrollingListeners.size(); i++)
		{
			if ((This->scrollingListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->scrollingListeners[i]->check(Event::InputType::SCROLLING, -1, -1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i<This->scrollingListeners.size(); i++)
		{
			if ((This->scrollingListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->scrollingListeners[i]->check(Event::InputType::SCROLLING, -1, -1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}
	}
    
}
void EventHandlerImpl::dropCallback(GLFWwindow* window, int count, const char** paths)
{
    for (auto& cb : This->m_dropCallbacks)
        (*cb)(window, count, paths);

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->dragAndDropListeners.size();i++)
		{
			if((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->dragAndDropListeners[i]->check(Event::InputType::DRAGANDDROP,-1,-1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->dragAndDropListeners.size();i++)
		{
			if((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->dragAndDropListeners[i]->check(Event::InputType::DRAGANDDROP,-1,-1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->dragAndDropListeners.size(); i++)
		{
			if ((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->dragAndDropListeners[i]->check(Event::InputType::DRAGANDDROP, -1, -1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i<This->dragAndDropListeners.size(); i++)
		{
			if ((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->dragAndDropListeners[i]->check(Event::InputType::DRAGANDDROP, -1, -1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}
	}
}


void EventHandlerImpl::windowFocusCallback(GLFWwindow* window, int focused)
{
    for (auto& cb : This->m_windowFocusCallbacks)
        (*cb)(window, focused);

	for (unsigned int i = 0; i < This->windowList.size(); i++)
	{
		if (glfwGetWindowAttrib(This->windowList[i], GLFW_FOCUSED))
		{
			This->focusedWindow = window;
			return;
		}
	}
	This->focusedWindow = nullptr;
}
void EventHandlerImpl::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    for (auto& cb : This->m_resizeCallbacks)
        (*cb)(window, width, height);

	RenderContext* context = RenderContext::getContextFromWindow(window);
	context->updateViewportSize(vec2i(width, height));
}
//
