#pragma once

#include <glm/glm.hpp>

enum FrogAnimation
{
    Sit,
    Lick,
    Eat
};

struct FrogState {
    glm::vec2 pos = { 0, 0 };
    glm::vec2 velocity = { 0, 0 };
    FrogAnimation currentAnimation = FrogAnimation::Sit;
    bool flyCaught = false;
    int fliesCaught = 0;
};
