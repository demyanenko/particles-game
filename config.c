#pragma once

#include "raylib.h"

typedef struct
{
    float friction;
    float baseParticleRadius;
    float baseRepelRadius;
    float repelFactor;
    float attractionPointRadius;
    float attractionPointForce;
    float playerThrottleAmount;
    float playerTurnAmount;
    float scaleFactor;
    int width;
    int sidebarWidth;
    int height;
    float backgroundTransparency;
    Color backgroundColor;
} Config;

void configInit(Config *config, float scaleFactor, int width, int sidebarWidth, int height)
{
    config->friction = 0.2;
    config->baseParticleRadius = 2.7;
    config->baseRepelRadius = 4;
    config->repelFactor = 1;
    config->attractionPointRadius = 1.5 * config->baseParticleRadius;
    config->attractionPointForce = 1;
    config->playerThrottleAmount = 1;
    config->playerTurnAmount = 0.01 * PI;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->sidebarWidth = sidebarWidth;
    config->height = height;
    config->backgroundTransparency = 0.7;
    config->backgroundColor = (Color){0, 0, 0, (1 - config->backgroundTransparency) * 255};
}