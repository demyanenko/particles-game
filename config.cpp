#pragma once

#include "raylib.h"

struct Config
{
    int physicsStepsPerFrame;
    double dt;
    float friction;
    float baseParticleRadius;
    float baseRepelRadius;
    float baseRepelFactor;
    float snapPointRadius;
    float snapPointForce;
    float playerThrottleAmount;
    float playerTurnAmount;
    float scaleFactor;
    int width;
    int sidebarWidth;
    int height;
    float backgroundTransparency;
    Color backgroundColor;
};

void configInit(Config *config, float scaleFactor, int width, int sidebarWidth, int height)
{
    config->physicsStepsPerFrame = 50;
    config->dt = 1.0 / 60 / config->physicsStepsPerFrame;
    config->friction = 30;
    config->baseParticleRadius = 2.7;
    config->baseRepelRadius = 4;
    config->baseRepelFactor = 3600;
    config->snapPointRadius = 2 * config->baseParticleRadius;
    config->snapPointForce = 100000;
    config->playerThrottleAmount = 1800;
    config->playerTurnAmount = 0.6 * PI;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->sidebarWidth = sidebarWidth;
    config->height = height;
    config->backgroundTransparency = 0.7;
    unsigned char backgroundAlpha = (1 - config->backgroundTransparency) * 255;
    config->backgroundColor = (Color){0, 0, 0, backgroundAlpha};
}