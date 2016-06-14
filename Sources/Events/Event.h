#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <string>

class Event
{
    public:
        //  Miscellaneous
        enum EventType
        {
            BUTTON = 1,
            CHORD = 2,
            SEQUENCE = 3,
            GESTURE = 4,
            TYPE_MASK = 0x07,

            UP_FLAG = 1<<3,
            DOWN_FLAG = 1<<4,
            ACTIVATED_FLAG = 1<<5
        };
        enum InputType
        {
            KEY = 1,
            CHAR = 2,

            CURSORPOS = 4,
            CURSORENTER = 5,
            MOUSEBUTTON = 6,
            SCROLLING = 7,

            DRAGANDDROP = 8
        };
        struct Input
        {
            InputType callback;
            int key;
        };
        //

        //  Default
        Event(uint8_t config = 0x00);
        virtual ~Event();
        //

        //  Public functions
        virtual bool isActivated() const;
        virtual bool check(InputType call,int key,int action);
        virtual bool check(Input in,int action);
        void addInput(Input in);
        void addInput(InputType call,int key);
        //

        //  Attributes
        uint8_t configuration;
        std::vector<std::pair<Input,bool> > inputList;
        //
};
