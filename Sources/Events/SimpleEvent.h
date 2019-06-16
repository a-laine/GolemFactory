#pragma once

#include "Events/InputType.h"


class SimpleEvent
{
    public:
        SimpleEvent();
        SimpleEvent(GameInput input, float scale = 1.f);

        void setInput(GameInput input);
        void setScale(float scale);
        GameInput getInput() const;
        float getScale() const;

    private:
        GameInput m_input;
        float m_scale;
        // void* m_callback;
};
