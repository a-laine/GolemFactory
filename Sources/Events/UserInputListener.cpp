#include "UserInputListener.h"

#include <Events/UserInputManager.h>



UserInputListener::UserInputListener()
    : enabled(false)
    , repeatMode(false)
    , textInput(false)
    , controller(DEVICE_ID_INVALID)
{
}

void UserInputListener::setRepeatMode(bool enable)
{
    repeatMode = enable;
}

void UserInputListener::setTextInput(bool enable)
{
    textInput = enable;
}

void UserInputListener::setCursorMode(bool enable)
{
    UserInputManager::getInstance()->setCursorMode(enable);
}

void UserInputListener::setDeviceToListen(InputDeviceId device)
{
    controller = device;
}

bool UserInputListener::getRepeatMode() const
{
    return repeatMode;
}

bool UserInputListener::getTextInput() const
{
    return textInput;
}

bool UserInputListener::getCursorMode() const
{
    return UserInputManager::getInstance()->getCursorMode();
}

bool UserInputListener::isListening() const
{
    return enabled;
}

uint8_t UserInputListener::getConfigFlags() const
{
    uint8_t res = 0;
    res |= enabled ? InputConfig::ENABLED : 0;
    res |= repeatMode ? InputConfig::REPEAT_MODE : 0;
    res |= textInput ? InputConfig::TEXT_INPUT : 0;
    return res;
}

InputDeviceId UserInputListener::getDeviceToListen() const
{
    return controller;
}

void UserInputListener::startListening()
{
    enabled = true;
}

void UserInputListener::pauseListening()
{
    enabled = false;
}

void UserInputListener::onButtonInput(InputDeviceId device, GameInput button, InputAction action)
{
    if(controller != DEVICE_ID_INVALID && controller != device)
        return;

    // process chords
    auto chordRange = chordMap.equal_range(button);
    if(chordRange.first != chordRange.second)
    {
        for(auto it = chordRange.first; it != chordRange.second; it++)
        {
            ChordEvent& evt = chordEvents[it->second];
            if(evt.ckeck(button, action))
            {
                float val = (action == KEY_RELEASED) ? 0 : evt.getScale();
                //processEvent(evt.getCallback(), val);
            }
        }
        return;
    }

    // process non chords
    auto simpleRange = simpleMap.equal_range(button);
    for(auto it = simpleRange.first; it != simpleRange.second; it++)
    {
        SimpleEvent& evt = it->second;
        if(evt.getInput() == button)
        {
            float val = (action == KEY_RELEASED) ? 0 : evt.getScale();
            //processEvent(evt.getCallback(), val);
        }
    }
}

void UserInputListener::onAxisInput(InputDeviceId device, GameInput axis, double value)
{
    if(controller != DEVICE_ID_INVALID && controller != device)
        return;
    
    auto simpleRange = simpleMap.equal_range(axis);
    for(auto it = simpleRange.first; it != simpleRange.second; it++)
    {
        SimpleEvent& evt = it->second;
        if(evt.getInput() == axis)
        {
            //processEvent(evt.getCallback(), value * evt.getScale());
        }
    }
}

void UserInputListener::onCharacterInput(uint32_t codepoint)
{
    //
}

void UserInputListener::onViewportResize(int width, int height)
{
    //
}

void UserInputListener::onControllerConnection(InputDeviceId device, bool connected)
{
    //
}

void UserInputListener::processEvent(void* callback, float value)
{
    //
}
