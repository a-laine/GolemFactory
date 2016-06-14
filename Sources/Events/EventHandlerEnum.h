#pragma once

#include <map>
#include <string>

#include "Utiles/Singleton.h"
#include "EventHandlerImpl.h"
#include "EventEnum.h"

class EventHandlerEnum : public EventHandlerImpl, public Singleton<EventHandlerEnum>
{
    friend class Singleton<EventHandlerEnum>;

    public:
        //  Public functions
		void reload(std::string path,std::string file);

        void loadKeyMapping(std::string file);
        void loadKeyMapping(std::string path,std::string file);
        bool isActivated(EventEnum eventName);

        void getFrameEvent(std::vector<EventEnum>& buffer);
        void addFrameEvent(EventEnum literalEvent);

        void addEvent(EventEnum eventName,Event::InputType call,int key = -1,uint8_t config = Event::DOWN_FLAG|Event::BUTTON);
        void removeEvent(EventEnum eventName);

        void clear();
        //

        //  Set/get functions
        unsigned int getNumberOfEvent();
        //

    private:
        //  Miscellaneous
        enum FlagsConfig2
        {
            CLEAR_EVENT_LIST = 1<<4
        };
        //

        //  Default
        EventHandlerEnum(std::string path = "");
        ~EventHandlerEnum();
        //

        //  Private functions
        void emitUserEvent(Event* event);
        void swapFrameEventList();
        //

        //  Attributes
        std::map<Event*,EventEnum> eventMapping;
        std::multimap<EventEnum,Event*> userMapping;
        std::vector<EventEnum> frameEvent;
        std::vector<EventEnum> frameEventBuffer;
        //
};
