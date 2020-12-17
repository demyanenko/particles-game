#pragma once

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
    float scaleFactor;
    int width;
    int height;
    float backgroundTransparency;
    Color backgroundColor;
    double playerReloadTime;
};

#define MAX_SNAP_DEPTH 20

void configInit(Config *config, float scaleFactor, int width, int height)
{
    config->physicsStepsPerFrame = 12;
    config->dt = 1.0 / 60 / config->physicsStepsPerFrame;
    config->baseRepelRadius = 1.5;
    config->baseRepelFactor = 3600;
    config->snapPointRadius = sqrt(2);
    config->snapPointForce = 5000;
    config->maxSnapDepth = MAX_SNAP_DEPTH;
    config->playerThrottleAmount = 1800;
    config->playerTurnAmount = 0.6 * PI;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->height = height;
    config->backgroundTransparency = 0.1;
    unsigned char backgroundAlpha = (1 - config->backgroundTransparency) * 255;
    config->backgroundColor = Color({0, 0, 0, backgroundAlpha});
    config->playerReloadTime = 0.3;
}