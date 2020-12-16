#pragma once

#include "raylib.h"

typedef struct
{
    float scaleFactor;
    float friction;
    float baseParticleRadius;
    float repelRadius;
    float baseRepelDistance;
    float maxInteractionRadius;
    float playerThrottleAmount;
    float playerTurnAmount;
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
    config->repelRadius = 1.6 * config->baseParticleRadius;
    config->baseRepelDistance = 2 * config->repelRadius;
    config->maxInteractionRadius = 30 * config->baseParticleRadius;
    config->playerThrottleAmount = 1;
    config->playerTurnAmount = 0.01 * PI;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->sidebarWidth = sidebarWidth;
    config->height = height;
    config->backgroundTransparency = 0.7;
    config->backgroundColor = (Color){0, 0, 0, (1 - config->backgroundTransparency) * 255};
}