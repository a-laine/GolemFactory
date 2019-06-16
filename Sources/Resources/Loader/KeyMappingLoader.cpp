#include "KeyMappingLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>


KeyMappingLoader::KeyMappingLoader()
{
}
/*
bool KeyMappingLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //	initialize variable
    std::string line, name, tmp;

    //	Parse user configuration file and instanciate associated event
    {
        //	load and parse file into variant structure
        Variant v; Variant* tmpvariant;
        try
        {
            Reader::parseFile(v, getFileName(resourceDirectory, fileName));
            tmpvariant = &(v.getMap().begin()->second);
        }
        catch(std::exception&)
        {
            if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                std::cerr << "ERROR : loading key mapping : " << fileName << " : fail to open or parse file" << std::endl;
            return false;
        }
        Variant& configMap = *tmpvariant;

        //	initialize parameters
        Variant currentEvent;
        Event* event;
        uint8_t eventConfig;
        bool errorHeaderPrinted = false;

        //	instanciate Event
        for(auto it = configMap.getMap().begin(); it != configMap.getMap().end(); it++)
        {
            currentEvent = it->second;

            //  extract name
            try
            {
                name.clear();
                name = currentEvent["eventPublished"].toString();
            }
            catch(std::exception& e)
            {
                if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                    std::cerr << "ERROR : loading key mapping at event '" << it->first << "' : " << e.what() << std::endl;
            }
            if(name.empty()) continue;

            //  extract configuration
            try
            {
                eventConfig = 0x00;
                if(currentEvent["configuration"].getType() == Variant::STRING)
                {
                    tmp = currentEvent["configuration"].toString();
                    if(tmp == "button")  eventConfig = (Event::EventType) Event::BUTTON;
                    else if(tmp == "chord")  eventConfig = (Event::EventType) Event::CHORD;
                    else if(tmp == "sequence")  eventConfig = (Event::EventType) Event::SEQUENCE;
                    else if(tmp == "gesture")  throw std::logic_error("event type not supported");
                    else throw std::logic_error("unknown type event");
                }
                else if(currentEvent["configuration"].getType() == Variant::ARRAY)
                {
                    for(auto it2 = currentEvent["configuration"].getArray().begin(); it2 != currentEvent["configuration"].getArray().end(); it2++)
                    {
                        tmp = it2->toString();
                        if(tmp == "button")  eventConfig |= (Event::EventType) Event::BUTTON;
                        else if(tmp == "chord")  eventConfig |= (Event::EventType) Event::CHORD;
                        else if(tmp == "sequence")  eventConfig |= (Event::EventType) Event::SEQUENCE;
                        else if(tmp == "up" || tmp == "released")  eventConfig |= (Event::EventType) Event::UP_FLAG;
                        else if(tmp == "down" || tmp == "pressed")  eventConfig |= (Event::EventType) Event::DOWN_FLAG;
                        else if(tmp == "gesture")  throw std::logic_error("event type not supported");
                        else throw std::logic_error("unknown type event");
                    }
                    if((eventConfig & Event::TYPE_MASK) > Event::SEQUENCE) throw std::logic_error("invalid configuration combination");
                }
            }
            catch(std::exception& e)
            {
                if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                    std::cerr << "ERROR : loading key mapping at event '" << it->first << "' : " << e.what() << std::endl;
                eventConfig = 0x00;
            }
            if(!eventConfig) continue;

            //	check if valid event before instanciate
            if(currentEvent.getMap().find("listeningKey") == currentEvent.getMap().end() && currentEvent.getMap().find("listeningMouse") == currentEvent.getMap().end() &&
                currentEvent.getMap().find("listeningScroll") == currentEvent.getMap().end() && currentEvent.getMap().find("listeningText") == currentEvent.getMap().end() &&
                currentEvent.getMap().find("listeningCursorPos") == currentEvent.getMap().end() && currentEvent.getMap().find("listeningCursorEntred") == currentEvent.getMap().end() &&
                currentEvent.getMap().find("listeningDragDrop") == currentEvent.getMap().end())
            {
                if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                    std::cerr << "ERROR : loading key mapping at event '" << it->first << "' : no listening callback defined" << std::endl;
                continue;
            }
            else
            {
                event = nullptr;
                switch(eventConfig & Event::TYPE_MASK)
                {
                    case Event::SEQUENCE: event = new EventSequence(); break;
                    default: event = new Event(eventConfig); break;
                }
                if(!event)
                {
                    if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                        std::cerr << "ERROR : loading key mapping at event '" << it->first << "' : error instancing" << std::endl;
                    continue;
                }
                eventMap.insert(std::pair<std::string, Event*>(name, event));
            }

            //	Attach keyboard input
            try
            {
                for(auto it2 = currentEvent["listeningKey"].getArray().begin(); it2 != currentEvent["listeningKey"].getArray().end(); it2++)
                {
                    int key = it2->toInt();
                    event->addInput(Event::KEY, key);
                }
            }
            catch(std::exception&) {}

            //	Attach mouse input
            try
            {
                for(auto it2 = currentEvent["listeningMouse"].getArray().begin(); it2 != currentEvent["listeningMouse"].getArray().end(); it2++)
                {
                    int key = it2->toInt();
                    event->addInput(Event::MOUSEBUTTON, key);
                }
            }
            catch(std::exception&) {}

            //	Attach scrolling input
            try
            {
                if(currentEvent["listeningScroll"].toBool())
                {
                    event->addInput(Event::SCROLLING, -1);
                }
            }
            catch(std::exception&) {}

            //	Attach text input
            try
            {
                if(currentEvent["listeningText"].toBool())
                {
                    event->addInput(Event::CHAR, -1);
                }
            }
            catch(std::exception&) {}

            //	Attach cursor position input
            try
            {
                if(currentEvent["listeningCursorPos"].toBool())
                {
                    event->addInput(Event::CURSORPOS, -1);
                }
            }
            catch(std::exception&) {}

            //	Attach cursor entred input
            try
            {
                if(currentEvent["listeningCursorEntred"].toBool())
                {
                    event->addInput(Event::CURSORENTER, -1);
                }
            }
            catch(std::exception&) {}

            //	Attach cursor drag and drop input
            try
            {
                if(currentEvent["listeningDragDrop"].toBool())
                {
                    event->addInput(Event::DRAGANDDROP, -1);
                }
            }
            catch(std::exception&) {}
        }
    }
}

void KeyMappingLoader::initialize(ResourceVirtual* resource)
{
    KeyMapping* km = static_cast<KeyMapping*>(resource);
    km->initialize(std::move(eventMap));
}
*/
void KeyMappingLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{
}

std::string KeyMappingLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += KeyMapping::directory;
    str += fileName;
    str += KeyMapping::extension;
    return str;
}
