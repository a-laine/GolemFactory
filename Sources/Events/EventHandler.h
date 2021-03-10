#pragma once

/*!
* \file EventHandler.h
* \brief Define EventHandler type (as EventHandlerEnum or EventHandlerString).
* \author Thibault LAINE
*/

#ifndef USE_STRING_EVENT_TYPE
    #include "EventHandlerEnum.h"
    typedef EventHandlerEnum EventHandler;
    typedef int UserEventType;

#else
    #include "EventHandlerString.h"
    typedef EventHandlerString EventHandler;
    typedef std::string UserEventType;

#endif // USE_STRING_EVENT_TYPE
