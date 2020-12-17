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
#include <unordered_map>
#include <unordered_set>

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
    return SnapPoint({u, v});
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
    std::unordered_map<int, Particle *> snappedParticles1;
    std::unordered_map<int, Particle *> snappedParticles2;
    std::unordered_map<int, Particle *> *currentSnappedParticles;
    std::unordered_set<int> bfsVisited;
    std::unordered_set<int> growingSnapPointsVisited;
    SnapPoint activeSnapPoints[SNAP_POINT_COUNT];
    int activeSnapPointCount;
    ParticleGrid particleGrid;
    double playerAngle;
    double playerLastShot;
};

Particle *popClosestSnappedParticle(World *world, double x, double y)
{
    std::unordered_map<int, Particle *> *snappedParticles = world->currentSnappedParticles;
    double closestDistanceSq = 10000000;
    auto closestIter = snappedParticles->end();
    for (auto it = snappedParticles->begin(); it != snappedParticles->end(); it++)
    {
        Particle *testParticle = it->second;
        double testDistance = pow((testParticle->x - x), 2) + pow((testParticle->y - y), 2);
        if (testDistance < closestDistanceSq && testParticle->isEdge)
        {
            closestDistanceSq = testDistance;
            closestIter = it;
        }
    }
    Particle *out = nullptr;
    if (closestIter != snappedParticles->end())
    {
        out = closestIter->second;
        snappedParticles->erase(closestIter);
    }
    return out;
}

void worldInit(World *world, float scaleFactor, int width, int height)
{
    configInit(&world->config, scaleFactor, width, height);

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

    world->currentSnappedParticles = &world->snappedParticles1;
    world->activeSnapPointCount = 0;

    particleGridInit(
        &world->particleGrid, width, height, ceilf(maxInteractionRadius), world->particles,
        PARTICLE_COUNT);

    world->playerAngle = PI / 2;
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