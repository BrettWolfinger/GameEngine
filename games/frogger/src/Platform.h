#pragma once
#include "LaneObject.h"

class Platform : public LaneObject {
public:
    Platform(float startX, int row, int tileWidth, float speed, int direction);

    virtual bool isSafe() const { return true; }
};
