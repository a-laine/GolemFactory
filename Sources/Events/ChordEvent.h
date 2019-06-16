#pragma once

#include <vector>

#include "Events/InputType.h"


class ChordEvent
{
    public:
        ChordEvent();
        ChordEvent(float scale = 1.f);

        void addInput(GameInput input);
        void setScale(float scale);
        float getScale() const;
        
        bool ckeck(GameInput input, InputAction action);

    private:
        struct ChordInput
        {
            GameInput input;
            bool activated;
        };

        std::vector<ChordInput> m_inputs;
        //void* callback
        float m_scale;
};
