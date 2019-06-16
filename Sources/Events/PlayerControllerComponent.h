#pragma once

#include <EntityComponent/Component.hpp>

#include <Events/Event.h>
#include <Events/EventSequence.h>


class KeyMapping;


class PlayerControllerComponent : public Component
{
    GF_DECLARE_COMPONENT_CLASS(PlayerControllerComponent, Component)
    public:
        PlayerControllerComponent();
        virtual ~PlayerControllerComponent() override;

        void registerEvent(const std::string& eventname, Event::InputType call, int key = -1, uint8_t config = Event::DOWN_FLAG | Event::BUTTON);
        void unregisterEvent(const std::string& eventName);
        void loadKeyMapping(const std::string& fileName);

    private:
        KeyMapping* m_keyMapping;
};

