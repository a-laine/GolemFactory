#pragma once

#include "Event.h"

#include <GLFW/glfw3.h>

class EventSequence : public Event
{
    public:
        //  Default
        EventSequence();
        ~EventSequence();
        //

        //  Public functions
        bool isActivated() const;
        bool check(InputType call,int key,int action);
        //

        //  Attributes
        static float timeout;
        //

    protected:
        //  Attributes
        uint8_t state;
        float lastCheckTime;
        //
};
