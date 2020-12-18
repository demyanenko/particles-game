#pragma once

#include <math.h>
#include "raylib.h"

struct Config
{
    int physicsStepsPerFrame;
    double dt;
    float baseRepelRadius;
    float baseRepelFactor;
    float snapPointRadius;
    float snapPointForce;
    int maxSnapDepth;
    float playerThrottleAmount;
    float playerTurnAmount;
    int playerSize;
    float scaleFactor;
    int width;
    int height;
    float backgroundTransparency;
    Color backgroundColor;
    double playerReloadTime;
    double bulletSpeed;
    int botCount;
    int botMinSnappedParticles;
    int botMaxSnappedParticles;
    double botMinAttackDistance;
    double botMaxAttackDistance;
    double botWaypointChangePeriod;
};

#define PHYSICS_STEPS_PER_FRAME 12
#define MAX_SNAP_DEPTH 20
#define BOT_COUNT 2

void configInit(Config *config, float scaleFactor, int width, int height)
{
    config->physicsStepsPerFrame = PHYSICS_STEPS_PER_FRAME;
    config->dt = 1.0 / 60 / config->physicsStepsPerFrame;
    config->baseRepelRadius = 1.5;
    config->baseRepelFactor = 3600;
    config->snapPointRadius = sqrt(2);
    config->snapPointForce = 7500;
    config->maxSnapDepth = MAX_SNAP_DEPTH;
    config->playerThrottleAmount = 1800;
    config->playerTurnAmount = 0.6 * PI;
    config->playerSize = 4;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->height = height;
    config->backgroundTransparency = 0.1;
    unsigned char backgroundAlpha = (1 - config->backgroundTransparency) * 255;
    config->backgroundColor = Color({0, 0, 0, backgroundAlpha});
    config->playerReloadTime = 0.1;
    config->bulletSpeed = 500;
    config->botCount = BOT_COUNT;
    config->botMinSnappedParticles = 20;
    config->botMaxSnappedParticles = 60;
    config->botMinAttackDistance = 100;
    config->botMaxAttackDistance = 150;
    config->botWaypointChangePeriod = 20;
}