#include "SimpleEvent.h"


SimpleEvent::SimpleEvent()
    : m_input(INVALID_GAME_INPUT)
    , m_scale(1.f)
{
}

SimpleEvent::SimpleEvent(GameInput input, float scale)
    : m_input(input)
    , m_scale(scale)
{
}

void SimpleEvent::setInput(GameInput input)
{
    m_input = input;
}

void SimpleEvent::setScale(float scale)
{
    m_scale = scale;
}

GameInput SimpleEvent::getInput() const
{
    return m_input;
}

float SimpleEvent::getScale() const
{
    return m_scale;
}
