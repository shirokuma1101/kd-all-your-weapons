#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class CharacterObject : public ModelObject
{
public:

    CharacterObject(std::string_view name)
        : ModelObject(name)
    {}
    virtual ~CharacterObject() {}
    
    void AddDamage(int damage) {
        m_health -= damage;
    }
    void AddHealth(int health) {
        m_health += health;
    }

protected:

    int m_health = 0;
    
};
