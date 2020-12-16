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

void updateParticleInteractions(World *world, ParticleType *particleType, int particleIndex)
{
    Particle *particles = world->particles;
    Config *config = &world->config;
    Particle *particle = particles + particleIndex;

    double sumXf = 0;
    double sumYf = 0;
    int interactionCount = 0;

    int cellIndices[9];
    int cellCount = particleGridGetNeighborhoodIndices(&world->particleGrid, particle, cellIndices);
    for (int i = 0; i < cellCount; i++)
    {
        ParticleCell *particleCell = world->particleGrid.particleCells + cellIndices[i];
        for (int j = 0; j < particleCell->count; j++)
        {
            Particle *otherParticle = particleCell->particles[j];
            if (otherParticle == particle)
            {
                continue;
            }

            double deltaX = otherParticle->x - particle->x;
            double deltaY = otherParticle->y - particle->y;
            double distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (distanceSquared == 0)
            {
                printf("Warning: particles occupy exactly same spot\n");
                continue;
            }

            double repelDistance = config->baseRepelRadius * (particle->radius + otherParticle->radius);
            if (!particle->isSnapped && distanceSquared < repelDistance * repelDistance)
            {
                double distance = sqrt(distanceSquared);
                double repelX = deltaX / distance;
                double repelY = deltaY / distance;
                double repelAmount = config->baseRepelFactor * (1.0 - distance / repelDistance);
                particle->xv -= double(repelX) * repelAmount * config->dt;
                particle->yv -= double(repelY) * repelAmount * config->dt;
            }

            double interactionRadius = particleType->radius[otherParticle->type];
            if (distanceSquared < interactionRadius * interactionRadius)
            {
                double distance = sqrt(distanceSquared);
                double interactionForce = particleType->force[otherParticle->type];
                sumXf += double(deltaX) / distance * interactionForce;
                sumYf += double(deltaY) / distance * interactionForce;
                interactionCount++;
            }
        }
    }

    int lastStepIndex = particleType->steps;
    if (interactionCount > 0)
    {
        particle->xa[lastStepIndex] = sumXf / interactionCount / particle->mass;
        particle->ya[lastStepIndex] = sumYf / interactionCount / particle->mass;
    }
    else
    {
        particle->xa[lastStepIndex] = 0;
        particle->ya[lastStepIndex] = 0;
    }
}

void updateSnappedParticles(World *world, GrowthMode growthMode)
{
    Config *config = &world->config;
    Particle *particles = world->particles;
    Particle *player = particles;
    Particle **snappedParticles = world->snappedParticles;
    ActiveSnapPoint *activeSnapPoints = world->activeSnapPoints;

    world->activeSnapPointCount = 0;
    Particle *newSnappedParticles[SNAP_POINT_COUNT];
    for (int i = 0; i < SNAP_POINT_COUNT; i++)
    {
        newSnappedParticles[i] = NULL;
    }
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].isSnapped = false;
    }
    particles[0].isSnapped = true;

    if (growthMode == GrowthMode::Shedding)
    {
        memcpy(snappedParticles, newSnappedParticles, SNAP_POINT_COUNT * sizeof(Particle *));
        return;
    }

    Particle *snappedParticleQueue[PARTICLE_COUNT];
    int bfsPopIndex = 0;
    int bfsPushIndex = 0;
    bool bfsVisited[PARTICLE_COUNT];
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        bfsVisited[i] = false;
    }

    double playerSnapDistance = (player->radius + 2) * config->baseParticleRadius;
    for (int i = 0; i < 6; i++)
    {
        double snapPointAngle = world->playerAngle + PI / 6 + i * PI / 3;
        double snapPointX = player->x + playerSnapDistance * cos(snapPointAngle);
        double snapPointY = player->y - playerSnapDistance * sin(snapPointAngle);
        ActiveSnapPoint snapPoint = {i, snapPointX, snapPointY};
        if (growthMode == GrowthMode::Growing)
        {
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }

        Particle *snappedParticle = snappedParticles[i];
        if (snappedParticle == NULL)
        {
            continue;
        }

        double deltaX = snappedParticle->x - snapPointX;
        double deltaY = snappedParticle->y - snapPointY;
        double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance >= snappedParticle->radius * config->snapPointRadius)
        {
            continue;
        }

        newSnappedParticles[i] = snappedParticle;
        snappedParticleQueue[bfsPushIndex++] = snappedParticle;

        if (growthMode == GrowthMode::Maintaining)
        {
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }
    }

    while (bfsPopIndex != bfsPushIndex)
    {
        Particle *sourceParticle = snappedParticleQueue[bfsPopIndex++];
        sourceParticle->isSnapped = true;
        int sourceParticleIndex = (sourceParticle - particles) / sizeof(Particle *);
        for (int i = 0; i < 4; i++)
        {
            double snapPointAngle = world->playerAngle + i * PI / 2;
            double snapPointDistance = (sourceParticle->radius * 3) * config->baseParticleRadius;
            double snapPointX = sourceParticle->x + snapPointDistance * cos(snapPointAngle);
            double snapPointY = sourceParticle->y - snapPointDistance * sin(snapPointAngle);
            double playerDeltaX = snapPointX - player->x;
            double playerDeltaY = snapPointY - player->y;
            double playerDistance = sqrt(playerDeltaX * playerDeltaX + playerDeltaY * playerDeltaY);
            if (playerDistance < playerSnapDistance)
            {
                continue;
            }

            int snapPointIndex = 2 + sourceParticleIndex * 4 + i;
            ActiveSnapPoint snapPoint = {snapPointIndex, snapPointX, snapPointY};
            if (growthMode == GrowthMode::Growing)
            {
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
            }

            Particle *attractedParticle = snappedParticles[snapPointIndex];
            if (attractedParticle == NULL)
            {
                continue;
            }

            double deltaX = attractedParticle->x - snapPointX;
            double deltaY = attractedParticle->y - snapPointY;
            double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            if (distance >= attractedParticle->radius * config->snapPointRadius)
            {
                continue;
            }

            newSnappedParticles[snapPointIndex] = attractedParticle;
            if (growthMode == GrowthMode::Maintaining)
            {
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
            }

            int snappedParticleIndex = (attractedParticle - particles) / sizeof(Particle *);
            if (bfsVisited[snappedParticleIndex])
            {
                continue;
            }
            bfsVisited[snappedParticleIndex] = true;
            snappedParticleQueue[bfsPushIndex++] = attractedParticle;
        }
    }

    memcpy(snappedParticles, newSnappedParticles, SNAP_POINT_COUNT * sizeof(Particle *));
}

void applySnapPoints(World *world, GrowthMode growthMode)
{
    Config *config = &world->config;
    Particle *player = world->particles;
    Particle **snappedParticles = world->snappedParticles;
    ActiveSnapPoint *activeSnapPoints = world->activeSnapPoints;

    Particle *particlesAroundPlayer[PARTICLE_COUNT];
    int particleAroundPlayerCount = 0;

    int cellIndices[9];
    int cellCount = particleGridGetNeighborhoodIndices(&world->particleGrid, player, cellIndices);
    for (int i = 0; i < cellCount; i++)
    {
        ParticleCell *particleCell = world->particleGrid.particleCells + cellIndices[i];
        for (int j = 0; j < particleCell->count; j++)
        {
            Particle *particle = particleCell->particles[j];
            if (particle == player)
            {
                continue;
            }
            particlesAroundPlayer[particleAroundPlayerCount++] = particle;
        }
    }

    for (int i = 0; i < particleAroundPlayerCount; i++)
    {
        Particle *particle = particlesAroundPlayer[i];

        double sumXf = 0;
        double sumYf = 0;
        int interactionCount = 0;
        for (int j = 0; j < world->activeSnapPointCount; j++)
        {
            ActiveSnapPoint snapPoint = activeSnapPoints[j];
            double deltaX = snapPoint.x - particle->x;
            double deltaY = snapPoint.y - particle->y;
            double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            double interactionRadius = config->baseParticleRadius;
            if (distance >= interactionRadius)
            {
                continue;
            }

            if (growthMode == GrowthMode::Growing && snappedParticles[snapPoint.index] == NULL)
            {
                snappedParticles[snapPoint.index] = particle;
            }

            sumXf += double(deltaX) / distance * config->snapPointForce;
            sumYf += double(deltaY) / distance * config->snapPointForce;
            interactionCount++;
        }

        if (interactionCount > 0)
        {
            particle->xa[0] += sumXf / interactionCount / particle->mass;
            particle->ya[0] += sumYf / interactionCount / particle->mass;
        }
    }
}

void updateWallCollisions(Particle *particle, Config *config)
{
    double wallOffset = config->baseParticleRadius * particle->radius;
    if (particle->x < wallOffset)
    {
        particle->x = wallOffset;
        if (particle->xv < 0)
        {
            particle->xv *= -1;
        }
    }
    else if (particle->x > config->width - wallOffset)
    {
        particle->x = config->width - wallOffset;
        if (particle->xv > 0)
        {
            particle->xv *= -1;
        }
    }

    if (particle->y < wallOffset)
    {
        particle->y = wallOffset;
        if (particle->yv < 0)
        {
            particle->yv *= -1;
        }
    }
    else if (particle->y > config->height - wallOffset)
    {
        particle->y = config->height - wallOffset;
        if (particle->yv > 0)
        {
            particle->yv *= -1;
        }
    }
}

int compareInts(const void *a, const void *b)
{
    return *(int *)(a) - *(int *)(b);
}

void validateParticleGrid(ParticleGrid *particleGrid)
{
    int totalParticlesInCells = 0;
    int particlePositions[PARTICLE_COUNT];
    for (int i = 0; i < particleGrid->rowCount * particleGrid->columnCount; i++)
    {
        totalParticlesInCells += particleGrid->particleCells[i].count;
        for (int j = 0; j < particleGrid->particleCells[i].count; j++)
        {
            particlePositions[j] = particleGrid->particleCells[i].particles[j]->posWithinCell;
        }
        qsort(particlePositions, particleGrid->particleCells[i].count, sizeof(int), compareInts);
        for (int j = 0; j < particleGrid->particleCells[i].count - 1; j++)
        {
            if (particlePositions[j + 1] - particlePositions[j] != 1)
            {
                abortWithMessage("Particle positions inconsistent");
            }
        }
    }
    if (totalParticlesInCells != PARTICLE_COUNT)
    {
        abortWithMessage("Particle count inconsistent");
    }
}

void worldUpdate(World *world, int throttleDelta, int angleDelta, GrowthMode growthMode)
{
    Particle *particles = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    Config *config = &world->config;
    ParticleGrid *particleGrid = &world->particleGrid;

    for (int t = 0; t < config->physicsStepsPerFrame; t++)
    {
        world->playerAngle = fmodf(
            world->playerAngle + angleDelta * world->config.playerTurnAmount * config->dt, PI * 2);
        double throttle = throttleDelta * world->config.playerThrottleAmount;
        world->particles[0].xv += cos(world->playerAngle) * throttle * config->dt;
        world->particles[0].yv += -sin(world->playerAngle) * throttle * config->dt;

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            particle->xv *= (1.0 - config->friction * config->dt);
            particle->yv *= (1.0 - config->friction * config->dt);
            particle->x += particle->xv * config->dt;
            particle->y += particle->yv * config->dt;

            ParticleType *particleType = particleTypes + particle->type;
            updateParticleInteractions(world, particleType, i);
            for (int j = 0; j < particleType->steps; j++)
            {
                particle->xa[j] = particle->xa[j + 1];
                particle->ya[j] = particle->ya[j + 1];
            }
        }

        updateSnappedParticles(world, growthMode);
        applySnapPoints(world, growthMode);

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            particle->xv += particle->xa[0] * config->dt;
            particle->yv += particle->ya[0] * config->dt;
            updateWallCollisions(particle, config);
            particleGridUpdateCell(particleGrid, particle, i);
        }

        validateParticleGrid(particleGrid);
    }
}

void worldRender(World *world)
{
    float scaleFactor = world->config.scaleFactor;

    DrawRectangle(
        0, 0,
        world->config.width * scaleFactor,
        world->config.height * scaleFactor,
        world->config.backgroundColor);

    for (int i = 0; i < world->activeSnapPointCount; i++)
    {
        ActiveSnapPoint snapPoint = world->activeSnapPoints[i];
        DrawCircle(
            snapPoint.x * scaleFactor,
            snapPoint.y * scaleFactor,
            world->config.snapPointRadius * scaleFactor,
            (Color){255, 255, 255, 63});
    }

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        Particle *particle = world->particles + i;
        DrawCircle(
            particle->x * scaleFactor,
            particle->y * scaleFactor,
            particle->radius * world->config.baseParticleRadius * scaleFactor,
            world->particleTypes[particle->type].color);
    }

    DrawLineEx(
        (Vector2){
            float(world->particles[0].x + cos(world->playerAngle) * 10) * scaleFactor,
            float(world->particles[0].y - sin(world->playerAngle) * 10) * scaleFactor},
        (Vector2){
            float(world->particles[0].x) * scaleFactor,
            float(world->particles[0].y) * scaleFactor},
        scaleFactor,
        WHITE);

    int cellWidth = world->config.sidebarWidth / PARTICLE_TYPE_COUNT * 2;
    int cellHeight = world->config.sidebarWidth / PARTICLE_TYPE_COUNT / 2;
    char stepsStr[2];
    int stepsFontSize = 10;
    int stepsOffset = 5;
    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        int cellX = world->config.width + (i / 2) * cellWidth;
        int steps = world->particleTypes[i].steps;
        snprintf(stepsStr, 2, "%i", steps);
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            int cellY = (i % 2 * PARTICLE_TYPE_COUNT + j) * cellHeight;
            double interactionRadius = world->particleTypes[i].radius[j];
            double force = world->particleTypes[i].force[j];
            DrawCircle(
                (cellX + cellWidth / 2) * scaleFactor,
                (cellY + cellHeight / 2) * scaleFactor,
                world->config.baseParticleRadius * scaleFactor,
                world->particleTypes[i].color);
            DrawCircle(
                (cellX + cellWidth / 2 + interactionRadius) * scaleFactor,
                (cellY + cellHeight / 2) * scaleFactor,
                world->config.baseParticleRadius * scaleFactor,
                world->particleTypes[j].color);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    float(cellY + cellHeight / 2 - force * 0.05f) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    float(cellY + cellHeight / 2 + force * 0.05f) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawText(
                stepsStr,
                (cellX + cellWidth / 2 + force * 0.5f - stepsFontSize / 2) * scaleFactor,
                (cellY + cellHeight / 2 - stepsFontSize - stepsOffset) * scaleFactor,
                stepsFontSize * scaleFactor,
                GRAY);
        }
    }
}