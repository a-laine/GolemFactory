#include "EventHandlerEnum.h"
#include "Utiles/Parser/Reader.h"
#include "Utiles/ToolBox.h"

#include <cctype>
#include <sstream>

//  Default
EventHandlerEnum::EventHandlerEnum(const std::string& path) : EventHandlerImpl(path)
{
    addEvent(QUIT,Event::KEY,GLFW_KEY_ESCAPE);
}
EventHandlerEnum::~EventHandlerEnum()
{
	clear();
}
//

//  Public functions
void EventHandlerEnum::loadKeyMapping(const std::string& file)
{
	loadKeyMapping(repository,file);
}
void EventHandlerEnum::loadKeyMapping(const std::string& path,std::string filename)
{
	if (filename.empty())
		filename = "RPG key mapping";

	//	initialize variable
	std::string line, name, tmp;
	std::map<Event*, EventEnum> eventMappingBuffer;
	std::multimap<EventEnum, Event*> userMappingBuffer;
	std::map<int, std::vector<Event*> > keyboardListenersBuffer;
	std::map<int, std::vector<Event*> > mouseButtonListenersBuffer;
	std::vector<Event*> charEnteredListenersBuffer;
	std::vector<Event*> cursorPositionListenersBuffer;
    std::vector<Event*> cursorEnterListenersBuffer;
    std::vector<Event*> scrollingListenersBuffer;
    std::vector<Event*> dragAndDropListenersBuffer;
	std::map<std::string, int> enumMap;

    //  Create map userEnum string to int map
	{
		std::string file = ToolBox::openAndCleanCStyleFile(path + "../Sources/UserEnum.enum");
		if (file.empty()) return;
		
		int index = 0;
		std::stringstream userEnumStream(file);
		while (!userEnumStream.eof())
		{
			std::getline(userEnumStream, line, ',');
			if (line.empty()) continue;

			//	remove unrevelant char from line begining
			auto it = line.begin();
			while (std::isspace(*it))
				it = line.erase(it);

			//	extract key and value for map insertion
			if (line.find('=') == std::string::npos)
			{
				name = line;
				index++;
			}
			else
			{
				//	remove space from line
				for (it = line.begin(); it != line.end(); it++)
					if (std::isspace(*it)) it = std::prev(line.erase(it));
				
				//	extract name and value from line string
				name.clear();
				name.append(line.begin(), line.begin() + line.find('='));
				tmp.clear();
				tmp.append(line.begin() + line.find('=') + 1,line.end());
				index = std::stoi(tmp, nullptr, 10);
			}
			enumMap[name] = index;
		}
	}

	//	Parse user configuration file and instanciate associated event
	{
		//	load and parse file into variant structure
		Variant v; Variant* tmpvariant;
		try
		{
			Reader::parseFile(v, path + "Config/" + filename + ".json");
			tmpvariant = &(v.getMap().begin()->second);
		}
		catch (std::exception&)
		{
			std::cerr << "EventHandler : Fail to open or parse file :" << std::endl;
			std::cerr << "               " << path + "Config/" + filename + ".cfg" << std::endl;
			return;
		}
		Variant& configMap = *tmpvariant;

		//	initialize parameters
		Variant currentEvent;
		Event* event;
		uint8_t eventConfig;
		bool errorHeaderPrinted = false;

		//	instanciate Event
		for (auto it = configMap.getMap().begin(); it != configMap.getMap().end(); it++)
		{
			currentEvent = it->second;

			//  extract name
			try
			{
				name.clear();
				name = currentEvent["eventPublished"].toString();
				if(enumMap.find(name) == enumMap.end()) throw std::logic_error("unknown publisher declared");
			}
			catch (std::exception& e)
			{
				if (!errorHeaderPrinted) { errorHeaderPrinted = true; std::cerr << "EventHandler : Errors occurs in loading key mapping :" << std::endl; }
				std::cerr << "               Event '" << it->first <<"' : " << e.what() << std::endl;
			}
			if (name.empty()) continue;

			//  extract configuration
			try
			{
				eventConfig = 0x00;
				if (currentEvent["configuration"].getType() == Variant::STRING)
				{
					tmp = currentEvent["configuration"].toString();
					if (tmp == "button")  eventConfig = (Event::EventType) Event::BUTTON;
					else if (tmp == "chord")  eventConfig = (Event::EventType) Event::CHORD;
					else if (tmp == "sequence")  eventConfig = (Event::EventType) Event::SEQUENCE;
					else if (tmp == "gesture")  throw std::logic_error("event type not supported");
					else throw std::logic_error("unknown type event");
				}
				else if (currentEvent["configuration"].getType() == Variant::ARRAY)
				{
					for (auto it2 = currentEvent["configuration"].getArray().begin(); it2 != currentEvent["configuration"].getArray().end(); it2++)
					{
						tmp = it2->toString();
						if (tmp == "button")  eventConfig |= (Event::EventType) Event::BUTTON;
						else if (tmp == "chord")  eventConfig |= (Event::EventType) Event::CHORD;
						else if (tmp == "sequence")  eventConfig |= (Event::EventType) Event::SEQUENCE;
						else if (tmp == "up" || tmp == "released")  eventConfig |= (Event::EventType) Event::UP_FLAG;
						else if (tmp == "down" || tmp == "pressed")  eventConfig |= (Event::EventType) Event::DOWN_FLAG;
						else if (tmp == "gesture")  throw std::logic_error("event type not supported");
						else throw std::logic_error("unknown type event");
					}
					if((eventConfig & Event::TYPE_MASK) > Event::SEQUENCE) throw std::logic_error("invalid configuration combination");
				}
			}
			catch (std::exception& e)
			{
				if (!errorHeaderPrinted) { errorHeaderPrinted = true; std::cerr << "EventHandler : Errors occurs in loading key mapping :" << std::endl; }
				std::cerr << "               Event '" << it->first << "' : " << e.what() << std::endl;
				eventConfig = 0x00;
			}
			if (!eventConfig) continue;

			//	check if valid event before instanciate
			if (currentEvent.getMap().find("listeningKey") == currentEvent.getMap().end()       && currentEvent.getMap().find("listeningMouse") == currentEvent.getMap().end()        &&
				currentEvent.getMap().find("listeningScroll") == currentEvent.getMap().end()	&& currentEvent.getMap().find("listeningText") == currentEvent.getMap().end()         &&
				currentEvent.getMap().find("listeningCursorPos") == currentEvent.getMap().end() && currentEvent.getMap().find("listeningCursorEntred") == currentEvent.getMap().end() &&
				currentEvent.getMap().find("listeningDragDrop") == currentEvent.getMap().end())
			{
				if (!errorHeaderPrinted) { errorHeaderPrinted = true; std::cerr << "EventHandler : Errors occurs in loading key mapping :" << std::endl; }
				std::cerr << "               Event '" << it->first << "' : no listening callback defined" << std::endl;
				continue;
			}
			else
			{
				event = nullptr;
				switch (eventConfig & Event::TYPE_MASK)
				{
					case Event::SEQUENCE: event = new EventSequence(); break;
					default: event = new Event(eventConfig); break;
				}
				if (!event)
				{
					if (!errorHeaderPrinted) { errorHeaderPrinted = true; std::cerr << "EventHandler : Errors occurs in loading key mapping :" << std::endl; }
					std::cerr << "               Event '" << it->first << "' : error instancing" << std::endl;
					continue;
				}

				userMappingBuffer.insert(std::pair<EventEnum, Event*>((EventEnum)enumMap[name], event));
				eventMappingBuffer[event] = (EventEnum)enumMap[name];
			}

			//	Attach keyboard input
			try
			{
				for (auto it2 = currentEvent["listeningKey"].getArray().begin(); it2 != currentEvent["listeningKey"].getArray().end(); it2++)
				{
					int key = it2->toInt();
					event->addInput(Event::KEY, key);
					auto it3 = std::find(keyboardListenersBuffer[key].begin(), keyboardListenersBuffer[key].end(), event);
					if (it3 == keyboardListenersBuffer[key].end()) keyboardListenersBuffer[key].insert(keyboardListenersBuffer[key].end(), event);
				}
			}
			catch (std::exception&) {}

			//	Attach mouse input
			try
			{
				for (auto it2 = currentEvent["listeningMouse"].getArray().begin(); it2 != currentEvent["listeningMouse"].getArray().end(); it2++)
				{
					int key = it2->toInt();
					event->addInput(Event::MOUSEBUTTON, key);
					auto it3 = std::find(mouseButtonListenersBuffer[key].begin(), mouseButtonListenersBuffer[key].end(), event);
					if (it3 == mouseButtonListenersBuffer[key].end()) mouseButtonListenersBuffer[key].insert(mouseButtonListenersBuffer[key].end(), event);
				}
			}
			catch (std::exception&) {}

			//	Attach scrolling input
			try
			{
				if (currentEvent["listeningScroll"].toBool())
				{
					event->addInput(Event::SCROLLING, -1);
					scrollingListenersBuffer.push_back(event);
				}
			}
			catch (std::exception&) {}

			//	Attach text input
			try
			{
				if (currentEvent["listeningText"].toBool())
				{
					event->addInput(Event::CHAR, -1);
					scrollingListenersBuffer.push_back(event);
				}
			}
			catch (std::exception&) {}

			//	Attach cursor position input
			try
			{
				if (currentEvent["listeningCursorPos"].toBool())
				{
					event->addInput(Event::CURSORPOS, -1);
					scrollingListenersBuffer.push_back(event);
				}
			}
			catch (std::exception&) {}

			//	Attach cursor entred input
			try
			{
				if (currentEvent["listeningCursorEntred"].toBool())
				{
					event->addInput(Event::CURSORENTER, -1);
					scrollingListenersBuffer.push_back(event);
				}
			}
			catch (std::exception&) {}

			//	Attach cursor drag and drop input
			try
			{
				if (currentEvent["listeningDragDrop"].toBool())
				{
					event->addInput(Event::DRAGANDDROP, -1);
					scrollingListenersBuffer.push_back(event);
				}
			}
			catch (std::exception&) {}
		}
	}

    //  Swap list content
	clear();
    mutex.lock();
		eventMapping.swap(eventMappingBuffer);
        userMapping.swap(userMappingBuffer);
        keyboardListeners.swap(keyboardListenersBuffer);
        mouseButtonListeners.swap(mouseButtonListenersBuffer);
        charEnteredListeners.swap(charEnteredListenersBuffer);
        cursorPositionListeners.swap(cursorPositionListenersBuffer);
        cursorEnterListeners.swap(cursorEnterListenersBuffer);
        scrollingListeners.swap(scrollingListenersBuffer);
        dragAndDropListeners.swap(dragAndDropListenersBuffer);
    mutex.unlock();
}
bool EventHandlerEnum::isActivated(EventEnum eventName)
{
    bool b = false;
    mutex.lock();
    auto it = userMapping.find(eventName);
    while(it!=userMapping.end() && it->first==eventName)
    {
        if(it->second->isActivated())
		{
            b = true;
            break;
		}
        ++it;
    }
    mutex.unlock();
    return b;
}

void EventHandlerEnum::getFrameEvent(std::vector<EventEnum>& buffer)
{
    mutex.lock();
    buffer.insert(buffer.end(),frameEvent.begin(),frameEvent.end());
    configuration |= CLEAR_EVENT_LIST;
    mutex.unlock();
}
void EventHandlerEnum::addFrameEvent(EventEnum literalEvent)
{
    mutex.lock();
	specialAddedEvent.insert(specialAddedEvent.end(),literalEvent);
    mutex.unlock();
}

void EventHandlerEnum::addEvent(EventEnum eventName,Event::InputType call,int key,uint8_t config)
{
    mutex.lock();
    std::pair<EventEnum,Event*> p;
        p.first = eventName;
        p.second = new Event(config);
        p.second->addInput(call,key);
    auto it = userMapping.insert(p);
    Event* event = it->second;
    eventMapping[event] = eventName;
    EventHandlerImpl::addEvent(event,call,key);
    mutex.unlock();
}
void EventHandlerEnum::removeEvent(EventEnum eventName)
{
    mutex.lock();
    try
    {
        auto it = userMapping.find(eventName);
        if(it!=userMapping.end())
        {
            EventHandlerImpl::removeEvent(it->second);
            eventMapping.erase(it->second);
            delete it->second;
            userMapping.erase(it);
        }
    }
    catch(std::out_of_range){}
    mutex.unlock();
}

void EventHandlerEnum::clear()
{
    mutex.lock();
    eventMapping.clear();
    frameEvent.clear();
    EventHandlerImpl::clear();
    for(auto it=userMapping.begin();it!=userMapping.end();it++)
        delete it->second;
    userMapping.clear();
    configuration &= ~CLEAR_EVENT_LIST;
    mutex.unlock();
}
//

//  Set/get functions
unsigned int EventHandlerEnum::getNumberOfEvent()
{
	unsigned int nb;
	mutex.lock();
	nb = userMapping.size();
	mutex.unlock();
	return nb;
}
//

//  Private functions
void EventHandlerEnum::emitUserEvent(Event* event)
{
    if(event) frameEventBuffer.insert(frameEventBuffer.end(),eventMapping[event]);
}
void EventHandlerEnum::swapFrameEventList()
{
    if(configuration&CLEAR_EVENT_LIST)
    {
        frameEvent.swap(frameEventBuffer);
		frameEvent.insert(frameEvent.end(), specialAddedEvent.begin(), specialAddedEvent.end());
        frameEventBuffer.clear();
		specialAddedEvent.clear();
        configuration &= ~CLEAR_EVENT_LIST;
    }
    else
    {
        frameEvent.insert(frameEvent.end(),frameEventBuffer.begin(),frameEventBuffer.end());
        frameEventBuffer.clear();
    }
}
//
