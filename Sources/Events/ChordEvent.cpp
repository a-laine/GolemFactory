#include "ChordEvent.h"


ChordEvent::ChordEvent()
    : m_scale(1.f)
{
}

ChordEvent::ChordEvent(float scale)
    : m_scale(scale)
{
}

void ChordEvent::setScale(float scale)
{
    m_scale = scale;
}

void ChordEvent::addInput(GameInput input)
{
    m_inputs.push_back({input, false});
}

float ChordEvent::getScale() const
{
    return m_scale;
}

bool ChordEvent::ckeck(GameInput input, InputAction action)
{
    bool oldActivated = true;
    bool newActivated = true;
    for(ChordInput chord : m_inputs)
    {
        oldActivated = oldActivated && chord.activated != false;
        if(chord.input == input)
            chord.activated = action != KEY_RELEASED;
        newActivated = newActivated && chord.activated != false;
    }

    return newActivated || (oldActivated && !newActivated);
}

