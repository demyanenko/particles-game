#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bot.cpp"
#include "config.cpp"
#include "particle.cpp"
#include "particleGrid.cpp"
#include "particleTypes.cpp"
#include "player.cpp"

#define PLAYER_COUNT (1 + BOT_COUNT)

struct World
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    ParticleGrid particleGrid;
    int particleCellIndices[PARTICLE_COUNT];
    int particlePosWithinCell[PARTICLE_COUNT];
    ParticleGrid particleSnapGrid;
    int particleSnapCellIndices[PARTICLE_COUNT];
    int particlePosWithinSnapCell[PARTICLE_COUNT];
    Player players[PLAYER_COUNT];
    Bot bots[BOT_COUNT];
    int humanScore;
    int botScore;
};

void worldInit(World *world, float scaleFactor, int width, int height)
{
    configInit(&world->config, scaleFactor, width, height);

    initParticleTypes(world->particleTypes);

    int playerSize = world->config.playerSize;
    int playerHp = world->particleTypes[PLAYER_PARTICLE_TYPE].defaultHp;
    particleInit(
        world->particles, PLAYER_PARTICLE_TYPE, width / 2, height / 2, playerSize, playerHp);

    for (int i = 1; i < 1 + world->config.botCount; i++)
    {
        double x = getRandomDouble() * width;
        double y = getRandomDouble() * height;
        particleInit(world->particles + i, PLAYER_PARTICLE_TYPE, x, y, playerSize, playerHp);
    }

    for (int i = 1 + world->config.botCount; i < PARTICLE_COUNT; i++)
    {
        int particleTypeIndex;
        if (NEW_PARTICLE_GEN)
        {
            double random = getRandomDouble();
            if (random < 0.98)
            {
                particleTypeIndex = ARMOR_PARTICLE_TYPE;
            }
            else if (random < 0.99)
            {
                particleTypeIndex = SHIELD_PARTICLE_TYPE;
            }
            else
            {
                particleTypeIndex = IDLE_MISSILE_PARTICLE_TYPE;
            }
        }
        else
        {
            particleTypeIndex = ARMOR_PARTICLE_TYPE;
        }

        double x = getRandomDouble() * width;
        double y = getRandomDouble() * height;
        ParticleType *particleType = world->particleTypes + particleTypeIndex;
        particleInit(world->particles + i, particleTypeIndex, x, y, 1, particleType->defaultHp);
    }

    double maxInteractionRadius = 0;
    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            double radius = world->particleTypes[i].radius[j];
            if (radius > maxInteractionRadius)
            {
                maxInteractionRadius = radius;
            }
        }
    }
    particleGridInit(
        &world->particleGrid, width, height, ceil(maxInteractionRadius), world->particles,
        world->particleCellIndices, world->particlePosWithinCell, PARTICLE_COUNT);

    particleGridInit(
        &world->particleSnapGrid, width, height, ceil(world->config.snapPointRadius),
        world->particles, world->particleSnapCellIndices, world->particlePosWithinSnapCell, PARTICLE_COUNT);

    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        playerInit(world->players + i, world->particles + i);
    }

    for (int i = 0; i < BOT_COUNT; i++)
    {
        botInit(world->bots + i, world->players + i + 1, &world->config);
    }

    world->humanScore = 0;
    world->botScore = 0;
}

struct SnapPointPos
{
    double x;
    double y;
};

SnapPointPos snapPointGetPos(SnapPoint snapPoint, Player *player)
{
    double snapStep = 3;
    double playerSideAngleCos = player->angleSin;
    double playerSideAngleSin = -player->angleCos;
    double snapPointX = player->particle->x;
    double snapPointY = player->particle->y;
    snapPointX += snapPoint.u * playerSideAngleCos * snapStep;
    snapPointY -= snapPoint.u * playerSideAngleSin * snapStep;
    snapPointX += snapPoint.v * player->angleCos * snapStep;
    snapPointY -= snapPoint.v * player->angleSin * snapStep;
    return {snapPointX, snapPointY};
}