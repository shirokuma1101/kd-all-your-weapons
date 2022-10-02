#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class CharacterObject : public ModelObject
{
public:

    CharacterObject(std::string_view name)
        : ModelObject(name)
    {}

};
