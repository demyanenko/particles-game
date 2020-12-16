#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.cpp"
#include "hashMap.cpp"
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

struct SnapPoint
{
    int u;
    int v;
};

SnapPoint snapPointCreate(int u, int v)
{
    return (SnapPoint){u, v};
}

int snapPointGetKey(SnapPoint snapPoint)
{
    return 10000 * snapPoint.u + snapPoint.v;
}

struct World
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    HashMap<int, Particle *> snappedParticles;
    SnapPoint activeSnapPoints[SNAP_POINT_COUNT];
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

    hashMapInit(&world->snappedParticles);

    world->activeSnapPointCount = 0;

    particleGridInit(
        &world->particleGrid, width, height, ceilf(maxInteractionRadius), world->particles,
        PARTICLE_COUNT);

    world->playerAngle = PI / 2;
}

struct SnapPointPos
{
    double x;
    double y;
};

SnapPointPos snapPointGetPos(SnapPoint snapPoint, World *world)
{
    double snapStep = 3;
    double fwdAngle = world->playerAngle;
    double sideAngle = world->playerAngle - PI / 2;
    double snapPointX = world->particles[0].x;
    double snapPointY = world->particles[0].y;
    snapPointX += snapPoint.u * cos(sideAngle) * snapStep;
    snapPointY -= snapPoint.u * sin(sideAngle) * snapStep;
    snapPointX += snapPoint.v * cos(fwdAngle) * snapStep;
    snapPointY -= snapPoint.v * sin(fwdAngle) * snapStep;
    return {snapPointX, snapPointY};
}