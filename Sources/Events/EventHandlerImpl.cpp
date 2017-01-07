#include "EventHandlerImpl.h"

//  Static attributes
EventHandlerImpl* EventHandlerImpl::This = nullptr;
//

//  Default
EventHandlerImpl::EventHandlerImpl(const std::string& path) : configuration(0x00), repository(path), focusedWindow(nullptr), resizeCallback(nullptr)
{
    This = this;
    cursorPositionRelative = glm::vec2(0, 0);
    cursorPositionRelativeBuffer = glm::vec2(0, 0);
    cursorPositionAbsolute = glm::vec2(0, 0);
    cursorPositionAbsoluteBuffer = glm::vec2(0, 0);
    scrollingRelative = glm::vec2(0, 0);
    scrollingRelativeBuffer = glm::vec2(0, 0);
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
    glfwSetKeyCallback(window,EventHandlerImpl::keyCallback);
    glfwSetCharCallback(window,EventHandlerImpl::charCallback);
    glfwSetCursorPosCallback(window,EventHandlerImpl::cursorPositionCallback);
    glfwSetCursorEnterCallback(window,EventHandlerImpl::cursorEnterCallback);
    glfwSetMouseButtonCallback(window,EventHandlerImpl::mouseButtonCallback);
    glfwSetScrollCallback(window,EventHandlerImpl::scrollingCallback);
    glfwSetDropCallback(window,EventHandlerImpl::dropCallback);

    glfwSetWindowFocusCallback(window,EventHandlerImpl::windowFocusCallback);
    glfwSetFramebufferSizeCallback(window,EventHandlerImpl::framebufferResizeCallback);
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
    cursorPositionRelativeBuffer = glm::vec2(0,0);
    scrollingRelativeBuffer = glm::vec2(0,0);

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
void EventHandlerImpl::setResizeCallback(ResizeCallback cb)
{
    resizeCallback = cb;
}


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


glm::vec2 EventHandlerImpl::getCursorPositionRelative()
{
	glm::vec2 position;
    mutex.lock();
    position = cursorPositionRelative;
    mutex.unlock();
    return position;
}
glm::vec2 EventHandlerImpl::getCursorPositionAbsolute()
{
	glm::vec2 position;
    mutex.lock();
    position = cursorPositionAbsolute;
    mutex.unlock();
    return position;
}
glm::vec2 EventHandlerImpl::getCursorNormalizedPosition()
{
	glm::vec2 position;
    mutex.lock();
    if(focusedWindow)
    {
        int width, height;
        glfwGetFramebufferSize(focusedWindow,&width,&height);
        position.x = (2.f*cursorPositionAbsolute.x - width)/width;
        position.y = (height - 2.f*cursorPositionAbsolute.y)/height;
    }
    else position = glm::vec2(0,0);
    mutex.unlock();
    return position;
}
glm::vec2 EventHandlerImpl::getScrollingRelative()
{
	glm::vec2 scroll;
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
        case Event::KEY:            keyboardListeners[key].push_back(event);     break;
        case Event::MOUSEBUTTON:    mouseButtonListeners[key].push_back(event);  break;
        case Event::CHAR:           charEnteredListeners.push_back(event);       break;
        case Event::CURSORPOS:      cursorPositionListeners.push_back(event);    break;
        case Event::CURSORENTER:    cursorEnterListeners.push_back(event);       break;
        case Event::SCROLLING:      scrollingListeners.push_back(event);         break;
        case Event::DRAGANDDROP:    dragAndDropListeners.push_back(event);       break;
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
            case Event::KEY:
                try
                {
                    auto eventList = keyboardListeners.at(event->inputList[i].first.key);
                    auto it = std::find(eventList.begin(),eventList.end(),event);
                    if(it != eventList.end()) eventList.erase(it);
                }
                catch(std::out_of_range){}
                break;
            case Event::MOUSEBUTTON:
                try
                {
                    auto eventList = mouseButtonListeners.at(event->inputList[i].first.key);
                    auto it = std::find(eventList.begin(),eventList.end(),event);
                    if(it != eventList.end()) eventList.erase(it);
                }
                catch(std::out_of_range){}
                break;
           case Event::CHAR:
                {
                    auto it = std::find(charEnteredListeners.begin(),charEnteredListeners.end(),event);
                    if(it!=charEnteredListeners.end()) charEnteredListeners.erase(it);
                }
                break;
           case Event::CURSORPOS:
                {
                    auto it = std::find(cursorPositionListeners.begin(),cursorPositionListeners.end(),event);
                    if(it!=cursorPositionListeners.end()) cursorPositionListeners.erase(it);
                }
                break;
           case Event::CURSORENTER:
                {
                    auto it = std::find(cursorEnterListeners.begin(),cursorEnterListeners.end(),event);
                    if(it!=cursorEnterListeners.end()) cursorEnterListeners.erase(it);
                }
                break;
           case Event::SCROLLING:
                {
                    auto it = std::find(scrollingListeners.begin(),scrollingListeners.end(),event);
                    if(it!=scrollingListeners.end()) scrollingListeners.erase(it);
                }
                break;
           case Event::DRAGANDDROP:
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


std::string EventHandlerImpl::openAndCleanCStyleFile(std::string fileName, std::string commentBlockEntry, std::string commentLineEntry)
{
	//	open file to parse
	std::string output;
	std::ifstream file(fileName);
	if (!file.good())
	{
		std::cerr << "EventHandler : Fail to open file :" << std::endl;
		std::cerr << "               " << fileName << std::endl;
		return output;
	}

	//	parsing parameters initialization
	unsigned int parsingCommentBlockIndex = 0;
	unsigned int parsingCommentLineIndex = 0;
	bool actuallyParsingCommentBlock = false;
	bool actuallyParsingCommentLine = false;
	char currentChar;

	//	parse file and remove all comment block and line
	while (file.get(currentChar))
	{
		if (actuallyParsingCommentBlock)
		{
			if (currentChar == commentBlockEntry[parsingCommentBlockIndex])
			{
				if (parsingCommentBlockIndex == 0)	//	end of parsing entry block backward so stop parsing block
				{
					//	reset comment entry parsing parameter
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = false;
				}
				else parsingCommentBlockIndex--;	//	parsing backward for end of block detection
			}
			else parsingCommentBlockIndex = commentBlockEntry.size() - 1;	//	parsing block fail so reset parameter
		}
		else if (actuallyParsingCommentLine)
		{
			if (currentChar == '\n' || currentChar == '\r')	//	end line char detected so stop parsing comment line ('\r' and / or '\n' depending on platform)
			{
				//	reset comment entry parsing parameter
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = false;

				//	push the end line char to keep a cooherence if using getline on the string after
				output.push_back(currentChar);
			}
		}
		else if (!commentBlockEntry.empty() && currentChar == commentBlockEntry[parsingCommentBlockIndex])
		{
			if (parsingCommentBlockIndex >= commentBlockEntry.size() - 1)	//	comment block entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment block entry string
				output.erase(std::prev(output.end(), parsingCommentBlockIndex),output.end());

				//	set parameters for parsing a block
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = true;
				actuallyParsingCommentLine = false;
				continue;
			}
			else parsingCommentBlockIndex++;	//	parsing forward

			if (!commentLineEntry.empty() && currentChar == commentLineEntry[parsingCommentLineIndex])
			{
				if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
				{
					//	erase last pushed char coresponding to the comment line entry string
					output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

					//	set parameters for parsing a line
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = true;
					continue;
				}
				else parsingCommentLineIndex++;	//	parsing forward
			}
			else parsingCommentLineIndex = 0;	//	parsing line entry fail so reset parameter

			output.push_back(currentChar);
		}
		else if (!commentLineEntry.empty()  && currentChar == commentLineEntry[parsingCommentLineIndex])
		{
			parsingCommentBlockIndex = 0;	//	parsing block fail so reset parameter
			if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment line entry string
				output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

				//	set parameters for parsing a line
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = true;
				continue;
			}
			else parsingCommentLineIndex++;	//	parsing forward

			output.push_back(currentChar);	//	not begining parsing comment block or line so push current char to output string
		}
		else 
		{
			//	all parsing test fail so reset parameters
			parsingCommentBlockIndex = 0;
			parsingCommentLineIndex = 0;
			actuallyParsingCommentBlock = false;
			actuallyParsingCommentLine = false;

			//	not begining parsing comment block or line so push current char to output string
			output.push_back(currentChar);
		}
	}

	//	end
	file.close();
	if (output.empty())
	{
		std::cerr << "EventHandler : A file was sucessfully parsed for removing all comment but the result is an empty file !"  << std::endl;
		std::cerr << "               Please check this file for more details :" << std::endl;
		std::cerr << "               " << fileName << std::endl;
	}
	return output;
}
//

//  Callbacks
void EventHandlerImpl::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(!((This->configuration)&REPEAT) && action==GLFW_REPEAT) return;

    try
    {
        auto events = This->keyboardListeners.at(key); 
		if ((This->configuration&CHORD_HIGH_PRIORITY))
		{
			//	first pass process chord
			for(unsigned int i=0;i<events.size();i++)	
			{
				if ((events[i]->configuration & Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::KEY, key, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process !chord
			for(unsigned int i=0;i<events.size();i++)	
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::KEY, key, action))
					This->emitUserEvent(events[i]);
			}
		}
		else 
		{
			//	first pass process !chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::KEY, key, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration & Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::KEY, key, action))
					This->emitUserEvent(events[i]);
			}
		}
    }
    catch(std::out_of_range){}
}
void EventHandlerImpl::charCallback(GLFWwindow* window, unsigned int codepoint)
{
    if(!((This->configuration)&TEXTIN)) return;
    bool chord = false;
    for(unsigned int i=0;i<This->charEnteredListeners.size();i++)
    {
        if((This->charEnteredListeners[i]->configuration & Event::TYPE_MASK)==Event::CHORD &&
           This->charEnteredListeners[i]->check(Event::CHAR,codepoint,-1))
        {
            chord = true;
            This->emitUserEvent(This->charEnteredListeners[i]);
        }
    }
    if((This->configuration&CHORD_HIGH_PRIORITY) && chord) return;
    for(unsigned int i=0;i<This->charEnteredListeners.size();i++)
    {
        if((This->charEnteredListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD &&
           This->charEnteredListeners[i]->check(Event::CHAR,codepoint,-1))
            This->emitUserEvent(This->charEnteredListeners[i]);
    }
}
void EventHandlerImpl::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    This->cursorPositionRelativeBuffer.x = (float)xpos - This->cursorPositionAbsoluteBuffer.x;
    This->cursorPositionRelativeBuffer.y = (float)ypos - This->cursorPositionAbsoluteBuffer.y;
    This->cursorPositionAbsoluteBuffer.x = (float)xpos;
    This->cursorPositionAbsoluteBuffer.y = (float)ypos;

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->cursorPositionListeners.size();i++)
		{
			if((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->cursorPositionListeners[i]->check(Event::CURSORPOS,-1,-1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->cursorPositionListeners.size();i++)
		{
			if((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->cursorPositionListeners[i]->check(Event::CURSORPOS,-1,-1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->cursorPositionListeners.size(); i++)
		{
			if ((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorPositionListeners[i]->check(Event::CURSORPOS, -1, -1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
		
		//	second pass process chord
		for (unsigned int i = 0; i<This->cursorPositionListeners.size(); i++)
		{
			if ((This->cursorPositionListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorPositionListeners[i]->check(Event::CURSORPOS, -1, -1))
				This->emitUserEvent(This->cursorPositionListeners[i]);
		}
	}
    
}
void EventHandlerImpl::cursorEnterCallback(GLFWwindow* window, int entered)
{
	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorEnterListeners[i]->check(Event::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}

		//	second pass process !chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorEnterListeners[i]->check(Event::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->cursorEnterListeners[i]->check(Event::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i < This->cursorEnterListeners.size(); i++)
		{
			if ((This->cursorEnterListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->cursorEnterListeners[i]->check(Event::CURSORENTER, entered, -1))
				This->emitUserEvent(This->cursorEnterListeners[i]);
		}
	}
}
void EventHandlerImpl::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    try
    {
        auto events = This->mouseButtonListeners.at(button);

		if ((This->configuration & CHORD_HIGH_PRIORITY))
		{
			//	first pass process chord
			for(unsigned int i=0;i<events.size();i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::MOUSEBUTTON, button, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process !chord
			for (unsigned int i = 0; i < events.size(); i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::MOUSEBUTTON, button, action))
					This->emitUserEvent(events[i]);
			}
		}
		else
		{
			//	first pass process !chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) != Event::CHORD && events[i]->check(Event::MOUSEBUTTON, button, action))
					This->emitUserEvent(events[i]);
			}

			//	second pass process chord
			for (unsigned int i = 0; i<events.size(); i++)
			{
				if ((events[i]->configuration&Event::TYPE_MASK) == Event::CHORD && events[i]->check(Event::MOUSEBUTTON, button, action))
					This->emitUserEvent(events[i]);
			}
		}
    }
    catch(std::out_of_range){}
}
void EventHandlerImpl::scrollingCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    This->scrollingRelativeBuffer.x = (float)xoffset;
    This->scrollingRelativeBuffer.y = (float)yoffset;

	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->scrollingListeners.size();i++)
		{
			if((This->scrollingListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->scrollingListeners[i]->check(Event::SCROLLING,-1,-1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->scrollingListeners.size();i++)
		{
			if((This->scrollingListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->scrollingListeners[i]->check(Event::SCROLLING,-1,-1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->scrollingListeners.size(); i++)
		{
			if ((This->scrollingListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->scrollingListeners[i]->check(Event::SCROLLING, -1, -1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i<This->scrollingListeners.size(); i++)
		{
			if ((This->scrollingListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->scrollingListeners[i]->check(Event::SCROLLING, -1, -1))
				This->emitUserEvent(This->scrollingListeners[i]);
		}
	}
    
}
void EventHandlerImpl::dropCallback(GLFWwindow* window, int count, const char** paths)
{
	if ((This->configuration&CHORD_HIGH_PRIORITY))
	{
		//	first pass process chord
		for(unsigned int i=0;i<This->dragAndDropListeners.size();i++)
		{
			if((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK)==Event::CHORD && This->dragAndDropListeners[i]->check(Event::DRAGANDDROP,-1,-1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}

		//	second pass process !chord
		for(unsigned int i=0;i<This->dragAndDropListeners.size();i++)
		{
			if((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK)!=Event::CHORD && This->dragAndDropListeners[i]->check(Event::DRAGANDDROP,-1,-1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}
	}
	else
	{
		//	first pass process !chord
		for (unsigned int i = 0; i<This->dragAndDropListeners.size(); i++)
		{
			if ((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK) != Event::CHORD && This->dragAndDropListeners[i]->check(Event::DRAGANDDROP, -1, -1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}

		//	second pass process chord
		for (unsigned int i = 0; i<This->dragAndDropListeners.size(); i++)
		{
			if ((This->dragAndDropListeners[i]->configuration&Event::TYPE_MASK) == Event::CHORD && This->dragAndDropListeners[i]->check(Event::DRAGANDDROP, -1, -1))
				This->emitUserEvent(This->dragAndDropListeners[i]);
		}
	}
}


void EventHandlerImpl::windowFocusCallback(GLFWwindow* window, int focused)
{
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
    glViewport(0,0,width,height);
    if(This->resizeCallback) (*This->resizeCallback)(width,height);
}
//
