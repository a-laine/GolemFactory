#pragma once

#include <stdint.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <atomic>

#include <Events/InputType.h>
#include <Events/SimpleEvent.h>
#include <Events/ChordEvent.h>
#include <Utiles/Mutex.h>



namespace InputConfig {
enum InputConfig
{
    ENABLED = 1 << 0,
    REPEAT_MODE = 1 << 1,
    TEXT_INPUT = 1 << 2
};
}


class UserInputListener
{
    public:
        UserInputListener();

        void setRepeatMode(bool enable);
        void setTextInput(bool enable);
        void setCursorMode(bool enable);
        void setDeviceToListen(InputDeviceId device);

        bool getRepeatMode() const;
        bool getTextInput() const;
        bool getCursorMode() const;
        bool isListening() const;
        uint8_t getConfigFlags() const;
        InputDeviceId getDeviceToListen() const;

        void startListening();
        void pauseListening();

        void onButtonInput(InputDeviceId device, GameInput button, InputAction action);
        void onAxisInput(InputDeviceId device, GameInput axis, double value);
        void onCharacterInput(uint32_t codepoint);
        void onViewportResize(int width, int height);
        void onControllerConnection(InputDeviceId device, bool connected);


    private:
        void processEvent(void* callback, float value);


        std::atomic_bool enabled;
        std::atomic_bool repeatMode;
        std::atomic_bool textInput;

        std::unordered_multimap<GameInput, SimpleEvent> simpleMap;
        std::unordered_multimap<GameInput, uint32_t> chordMap;
        std::unordered_multimap<GameInput, uint32_t> sequenceMap;

        std::vector<ChordEvent> chordEvents;
        //std::vector<SequenceEvent> sequenceEvents;

        //void* charInputCallback;
        //void* viewportResizeCallback;
        //void* controllerConnectionCallback;

        InputDeviceId controller;
};
