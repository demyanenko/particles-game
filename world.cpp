#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.cpp"
#include "particle.cpp"
#include "particleGrid.cpp"
#include "particleTypes.cpp"

#define PARTICLE_COUNT 250
#define SNAP_POINT_COUNT (PARTICLE_COUNT * 4 + 2)

enum class GrowthMode
{
    Growing,
    Maintaining,
    Shedding
};

struct ActiveSnapPoint
{
    int index;
    double x;
    double y;
};

struct World
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    Particle *snappedParticles[SNAP_POINT_COUNT];
    ActiveSnapPoint activeSnapPoints[SNAP_POINT_COUNT];
    int activeSnapPointCount;
    ParticleGrid particleGrid;
    double playerAngle;
};

void worldInit(World *world, float scaleFactor, int width, int sidebarWidth, int height)
{
    configInit(&world->config, scaleFactor, width, sidebarWidth, height);

    initParticleTypes(world->particleTypes);
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

    particleInit(world->particles, 0, width / 2, height / 2, 4);

    srand(time(0));
    for (int i = 1; i < PARTICLE_COUNT; i++)
    {
        int particleType = 1;
        double x = double(rand()) / RAND_MAX * width;
        double y = double(rand()) / RAND_MAX * height;
        particleInit(world->particles + i, particleType, x, y, 1);
    }

    for (int i = 0; i < SNAP_POINT_COUNT; i++)
    {
        world->snappedParticles[i] = NULL;
    }

    world->activeSnapPointCount = 0;

    particleGridInit(
        &world->particleGrid, width, height, ceilf(maxInteractionRadius), world->particles,
        PARTICLE_COUNT);

    world->playerAngle = PI / 2;
}