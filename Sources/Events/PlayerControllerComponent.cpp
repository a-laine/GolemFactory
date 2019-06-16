#include "PlayerControllerComponent.h"

#include <Resources/KeyMapping.h>
#include <Resources/ResourceManager.h>



PlayerControllerComponent::PlayerControllerComponent()
{
    //
}

PlayerControllerComponent::~PlayerControllerComponent()
{
    //
}

void PlayerControllerComponent::registerEvent(const std::string& eventname, Event::InputType call, int key, uint8_t config)
{
    //
}

void PlayerControllerComponent::unregisterEvent(const std::string& eventName)
{
    //
}

void PlayerControllerComponent::loadKeyMapping(const std::string& fileName)
{
    ResourceManager::getInstance()->release(m_keyMapping);
    m_keyMapping = ResourceManager::getInstance()->getResource<KeyMapping>(fileName);
}
