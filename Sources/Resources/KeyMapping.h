#pragma once

#include <map>

#include "ResourceVirtual.h"
#include "Events/SimpleEvent.h"
#include "Events/ChordEvent.h"


class KeyMapping : public ResourceVirtual
{
    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        KeyMapping(const std::string& fileName);

        const std::multimap<std::string, Event*>& getList() const;
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        
        void initialize(const std::multimap<std::string, Event*>& list);
        void initialize(std::multimap<std::string, Event*>&& list);
        
    private:
        static std::string defaultName;

        std::multimap<std::string, Event*> mapping;
};
