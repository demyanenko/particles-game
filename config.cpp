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

#define DEMO_2

#ifdef DEMO_1
#define BOT_COUNT 0
#define SCALE_FACTOR 8
#define WORLD_SIZE 250
#define PARTICLE_COUNT 250
#define PARTICLE_TYPE_COUNT 3
#define NEW_PARTICLE_GEN 0
#endif

#ifdef DEMO_2
#define BOT_COUNT 2
#define SCALE_FACTOR 5
#define WORLD_SIZE 400
#define PARTICLE_COUNT 640
#define PARTICLE_TYPE_COUNT 3
#define NEW_PARTICLE_GEN 0
#endif

#ifdef DEMO_3
#define BOT_COUNT 1
#define SCALE_FACTOR 5
#define WORLD_SIZE 400
#define PARTICLE_COUNT 640
#define PARTICLE_TYPE_COUNT 6
#define NEW_PARTICLE_GEN 1
#endif

#define PHYSICS_STEPS_PER_FRAME 12
#define MAX_SNAP_DEPTH 20

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
    config->playerReloadTime = 0.2;
    config->bulletSpeed = 800;
    config->botCount = BOT_COUNT;
    config->botMinSnappedParticles = 20;
    config->botMaxSnappedParticles = 60;
    config->botMinAttackDistance = 100;
    config->botMaxAttackDistance = 150;
    config->botWaypointChangePeriod = 20;
}