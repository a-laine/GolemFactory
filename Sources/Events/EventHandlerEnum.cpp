#include "EventHandlerEnum.h"

#include <cctype>
#include <fstream>
#include <sstream>

//  Default
EventHandlerEnum::EventHandlerEnum(std::string path) : EventHandlerImpl(path)
{
    addEvent(QUIT,Event::KEY,GLFW_KEY_ESCAPE);
    loadKeyMapping("RPG key mapping");
}
EventHandlerEnum::~EventHandlerEnum(){}
//

//  Public functions
void EventHandlerEnum::reload(std::string path, std::string file)
{
	repository = path;
	loadKeyMapping(path, file);
}

void EventHandlerEnum::loadKeyMapping(std::string file){loadKeyMapping(repository,file);}
void EventHandlerEnum::loadKeyMapping(std::string path,std::string file)
{
    std::string line,name,tmp; int index,key;
    std::map<Event*,EventEnum> eventMappingBuffer;
    std::multimap<EventEnum,Event*> userMappingBuffer;
    std::map<int,std::vector<Event*> > keyboardListenersBuffer;
    std::map<int,std::vector<Event*> > mouseButtonListenersBuffer;
    std::vector<Event*> charEnteredListenersBuffer;
    std::vector<Event*> cursorPositionListenersBuffer;
    std::vector<Event*> cursorEnterListenersBuffer;
    std::vector<Event*> scrollingListenersBuffer;
    std::vector<Event*> dragAndDropListenersBuffer;

    //  Create map userEnum -> int
    std::map<std::string,int> enumMap;
    std::ifstream enumFile(path + "../Sources/UserEnum.enum");
	if (!enumFile.good() && path != "")
    {
		std::cerr << "EventHandler : Unable to load key mapping:" << std::endl;
		std::cerr << "               Fail to open event enumeration file :" << std::endl;
		std::cerr << "               " << path + "../Sources/UserEnum.enum" << std::endl;
        return;
    }
	else if (path == "")
	{
		std::cerr << "EventHandler : Unable to load key mapping:" << std::endl;
		std::cerr << "               Fail to open event enumeration file" << std::endl;
		std::cerr << "               Please check your repository path" << std::endl;
		return;
	}

    index = 0;
    while(!enumFile.eof())
    {
        std::getline(enumFile,line,',');
        line.erase(remove_if(line.begin(),line.end(),[](char x){return std::isspace(x);}),line.end());
        if(line.size() == 0) continue;          //empty line
        else if(line.find("//") == 0) continue; //comment line

        if(line.find('=') == std::string::npos)
        {
            name = line;
            index++;
        }
        else
        {
            std::stringstream lineStream(line);
            std::getline(lineStream,name,'=');
            std::getline(lineStream,tmp);
            index = atoi(tmp.c_str());
        }
        enumMap[name] = index;
    }
    enumFile.close();

    //  Parse command file
    std::ifstream configFile(path + "Config/" + file + ".cfg");
	if (!configFile.good())
    {
		std::cerr << "EventHandler : Unable to load key mapping:" << std::endl;
		std::cerr << "               Fail to open event config file :" << std::endl;
		std::cerr << "               " << path + "Config/" + file + ".cfg" << std::endl;
        return;
    }

    while(!configFile.eof())
    {
        std::getline(configFile,line,';');
        line.erase(remove_if(line.begin(),line.end(),[](char x){return std::isspace(x);}),line.end());
        if(line.size() == 0) continue;          //empty line
        else if(line.find("//") == 0) continue; //comment line
        std::stringstream lineStream(line);

        //  extract NAME & config byte
        std::getline(lineStream,name,',');
            if(lineStream.eof()) continue;
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());

        Event* event;
        if((index&Event::TYPE_MASK) == Event::SEQUENCE)
        {
            std::pair<EventEnum,Event*> p;
                p.first = (EventEnum)enumMap[name];
                p.second = new EventSequence();
            auto it = userMappingBuffer.insert(p);
            event = it->second;
            eventMappingBuffer[event] = p.first;
        }
        else if((index&Event::TYPE_MASK) == Event::GESTURE)
        {
            std::cerr<<"EventHandler : GESTURE event type not yet implemented :"<<std::endl;
            std::cerr<<"               \'"<<name<<"\' not added to the event list."<<std::endl;
            continue;
        }
        else
        {
            std::pair<EventEnum,Event*> p;
                p.first = (EventEnum)enumMap[name];
                p.second = new Event((uint8_t)index);
            auto it = userMappingBuffer.insert(p);
            event = it->second;
            eventMappingBuffer[event] = p.first;
        }

        //  Process keyboard input
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        for(int i=0;i<index;i++)
        {
            std::getline(lineStream,tmp,',');
                if(lineStream.eof()) continue;
                key = atoi(tmp.c_str());
            event->addInput(Event::KEY,key);
            auto it = std::find(keyboardListenersBuffer[key].begin(),keyboardListenersBuffer[key].end(),event);
            if(it==keyboardListenersBuffer[key].end()) keyboardListenersBuffer[key].insert(keyboardListenersBuffer[key].end(),event);
        }

        //  Process mouse input
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        for(int i=0;i<index;i++)
        {
            std::getline(lineStream,tmp,',');
                if(lineStream.eof()) continue;
                key = atoi(tmp.c_str());
            event->addInput(Event::MOUSEBUTTON,key);
            auto it = std::find(mouseButtonListenersBuffer[key].begin(),mouseButtonListenersBuffer[key].end(),event);
            if(it==mouseButtonListenersBuffer[key].end()) mouseButtonListenersBuffer[key].insert(mouseButtonListenersBuffer[key].end(),event);
        }

        //  Process scrolling
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        if(index)
        {
            event->addInput(Event::SCROLLING,-1);
            scrollingListenersBuffer.push_back(event);
        }

        //  Process char enter
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        if(index)
        {
            event->addInput(Event::CHAR,-1);
            charEnteredListenersBuffer.push_back(event);
        }

        //  Process cursor pos
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        if(index)
        {
            event->addInput(Event::CURSORPOS,-1);
            cursorPositionListenersBuffer.push_back(event);
        }

        //  Process cursor enter
        std::getline(lineStream,tmp,',');
            if(lineStream.eof()) continue;
            index = atoi(tmp.c_str());
        if(index)
        {
            event->addInput(Event::CURSORENTER,-1);
            cursorEnterListenersBuffer.push_back(event);
        }

        //  Process drag & drop
        std::getline(lineStream,tmp,',');
            index = atoi(tmp.c_str());
        if(index)
        {
            event->addInput(Event::DRAGANDDROP,-1);
            dragAndDropListenersBuffer.push_back(event);
        }
    }
    configFile.close();

    //  Swap list content
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
        if(it->second->isActivated()) {
            b = true;
            break;                   }
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
    frameEvent.insert(frameEvent.end(),literalEvent);
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
    mutex.lock();
    unsigned int nb = userMapping.size();
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
        frameEventBuffer.clear();
        configuration &= ~CLEAR_EVENT_LIST;
    }
    else
    {
        frameEvent.insert(frameEvent.end(),frameEventBuffer.begin(),frameEventBuffer.end());
        frameEventBuffer.clear();
    }
}
//
