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

#define PARTICLE_COUNT 640
#define SNAP_POINT_COUNT (PARTICLE_COUNT * 4 + 2)

struct SnapPoint
{
    int u;
    int v;
};

SnapPoint snapPointCreate(int u, int v)
{
    return SnapPoint({u, v});
}

#define MAX_SNAP_RADIUS (MAX_SNAP_DEPTH + 5)
#define MAX_SNAP_DIAMETER (2 * MAX_SNAP_RADIUS)
#define MAX_SNAP_KEY (MAX_SNAP_DIAMETER * MAX_SNAP_DIAMETER)

unsigned int snapPointGetKey(SnapPoint snapPoint)
{
    long long result = MAX_SNAP_DIAMETER * (snapPoint.u + MAX_SNAP_RADIUS) + snapPoint.v + MAX_SNAP_RADIUS;
    assertOrAbort(result >= 0, "Snap point key underflow");
    assertOrAbort(result < MAX_SNAP_KEY, "Snap point key overflow");
    return (unsigned int)result;
}

struct World
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    Particle *snappedParticles[MAX_SNAP_KEY];
    SnapPoint activeSnapPoints[SNAP_POINT_COUNT];
    int activeSnapPointCount;
    ParticleGrid particleGrid;
    int particleCellIndices[PARTICLE_COUNT];
    int particlePosWithinCell[PARTICLE_COUNT];
    ParticleGrid particleSnapGrid;
    int particleSnapCellIndices[PARTICLE_COUNT];
    int particlePosWithinSnapCell[PARTICLE_COUNT];
    double playerAngle;
    double playerAngleCos;
    double playerAngleSin;
    double playerLastShot;
};

void worldInit(World *world, float scaleFactor, int width, int height)
{
    configInit(&world->config, scaleFactor, width, height);

    initParticleTypesRandom(world->particleTypes, &world->config);

    particleInit(world->particles, 0, width / 2, height / 2, 4);
    srand(time(0));
    for (int i = 1; i < PARTICLE_COUNT; i++)
    {
        int particleType = i < 0.8 * PARTICLE_COUNT
                               ? 1
                               : i < 0.9 * PARTICLE_COUNT
                                     ? 3
                                     : 4;
        particleType = 1 + i % (PARTICLE_TYPE_COUNT - 1);
        double x = double(rand()) / RAND_MAX * width;
        double y = double(rand()) / RAND_MAX * height;
        particleInit(world->particles + i, particleType, x, y, 1);
    }

    memset(world->snappedParticles, 0, MAX_SNAP_KEY * sizeof(Particle *));

    memset(world->activeSnapPoints, 0, sizeof(world->activeSnapPoints));
    world->activeSnapPointCount = 0;

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

    world->playerAngle = PI / 2;
    world->playerAngleSin = sin(world->playerAngle);
    world->playerAngleCos = cos(world->playerAngle);
    world->playerLastShot = 0;
}

struct SnapPointPos
{
    double x;
    double y;
};

SnapPointPos snapPointGetPos(SnapPoint snapPoint, World *world)
{
    double snapStep = 3;
    double playerSideAngleCos = world->playerAngleSin;
    double playerSideAngleSin = -world->playerAngleCos;
    double snapPointX = world->particles[0].x;
    double snapPointY = world->particles[0].y;
    snapPointX += snapPoint.u * playerSideAngleCos * snapStep;
    snapPointY -= snapPoint.u * playerSideAngleSin * snapStep;
    snapPointX += snapPoint.v * world->playerAngleCos * snapStep;
    snapPointY -= snapPoint.v * world->playerAngleSin * snapStep;
    return {snapPointX, snapPointY};
}