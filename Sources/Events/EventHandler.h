#pragma once



#ifndef USE_STRING_EVENT_TYPE
    #include "EventHandlerEnum.h"
    typedef EventHandlerEnum EventHandler;
    typedef EventEnum UserEventType;

#else
	#error EventHandlerString not yet implemented
    //#include "EventHandlerString.h"
    //typedef EventHandlerString EventHandler;
    //typedef std::string UserEventType;

#endif // USE_STRING_EVENT_TYPE
