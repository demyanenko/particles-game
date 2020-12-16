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

#define PARTICLE_COUNT 500
#define SNAP_POINT_COUNT (PARTICLE_COUNT * 4 + 2)

struct World
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    Particle *snappedParticles[SNAP_POINT_COUNT];
    ParticleGrid particleGrid;
    float playerAngle;
};

#define MAX_PARTICLE_RADIUS 1

void worldInit(World *world, float scaleFactor, int width, int sidebarWidth, int height)
{
    configInit(&world->config, scaleFactor, width, sidebarWidth, height);

    initParticleTypes(world->particleTypes);
    float maxInteractionRadius = 0;
    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            float radius = world->particleTypes[i].radius[j] * MAX_PARTICLE_RADIUS;
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
        float x = (float)(rand()) / RAND_MAX * width;
        float y = (float)(rand()) / RAND_MAX * height;
        float size = 1 + powf((float)(rand()) / RAND_MAX, 5) * (MAX_PARTICLE_RADIUS - 1);
        particleInit(world->particles + i, particleType, x, y, size);
    }

    for (int i = 0; i < SNAP_POINT_COUNT; i++)
    {
        world->snappedParticles[i] = NULL;
    }

    particleGridInit(
        &world->particleGrid, width, height, ceilf(maxInteractionRadius), world->particles,
        PARTICLE_COUNT);

    world->playerAngle = PI / 2;
}

void worldMovePlayer(World *world, int throttleDelta, int angleDelta)
{
    world->playerAngle = fmodf(world->playerAngle + angleDelta * world->config.playerTurnAmount, PI * 2);
    float throttle = throttleDelta * world->config.playerThrottleAmount;
    world->particles[0].xv = cosf(world->playerAngle) * throttle;
    world->particles[0].yv = -sinf(world->playerAngle) * throttle;
}

void updateParticleInteractions(World *world, ParticleType *particleType, int particleIndex)
{
    Particle *particles = world->particles;
    Config *config = &world->config;
    Particle *particle = particles + particleIndex;

    float sumXf = 0;
    float sumYf = 0;
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

            float deltaX = otherParticle->x - particle->x;
            float deltaY = otherParticle->y - particle->y;
            float distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (distanceSquared == 0)
            {
                printf("Warning: particles occupy exactly same spot\n");
                continue;
            }

            float repelDistance = config->baseRepelRadius * (particle->radius + otherParticle->radius);
            if (!particle->isSnapped && distanceSquared < repelDistance * repelDistance)
            {
                float distance = sqrtf(distanceSquared);
                float repelX = deltaX / distance;
                float repelY = deltaY / distance;
                float repelAmount = config->baseRepelFactor * (1.0 - distance / repelDistance);
                particle->xv -= repelX * repelAmount * config->dt;
                particle->yv -= repelY * repelAmount * config->dt;
            }

            float interactionRadius = particleType->radius[otherParticle->type];
            if (distanceSquared < interactionRadius * interactionRadius)
            {
                float distance = sqrtf(distanceSquared);
                float interactionForce = particleType->force[otherParticle->type];
                sumXf += deltaX / distance * interactionForce;
                sumYf += deltaY / distance * interactionForce;
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

struct ActiveSnapPoint
{
    int index;
    float x;
    float y;
};

void updateSnappedParticles(
    World *world, ActiveSnapPoint *activeSnapPoints, int *outActiveSnapPointCount,
    bool isPlayerAttractive)
{
    Config *config = &world->config;
    Particle *particles = world->particles;
    Particle *player = particles;
    Particle **snappedParticles = world->snappedParticles;

    Particle *newSnappedParticles[SNAP_POINT_COUNT];
    for (int i = 0; i < SNAP_POINT_COUNT; i++)
    {
        newSnappedParticles[i] = NULL;
    }

    Particle *snappedParticleQueue[PARTICLE_COUNT];
    int bfsPopIndex = 0;
    int bfsPushIndex = 0;
    bool bfsVisited[PARTICLE_COUNT];
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        bfsVisited[i] = false;
        particles[i].isSnapped = false;
    }
    particles[0].isSnapped = true;

    float playerSnapDistance = (player->radius + 2) * config->baseParticleRadius;
    for (int i = 0; i < 6; i++)
    {
        float snapPointAngle = world->playerAngle + PI / 6 + i * PI / 3;
        float snapPointX = player->x + playerSnapDistance * cosf(snapPointAngle);
        float snapPointY = player->y - playerSnapDistance * sinf(snapPointAngle);
        ActiveSnapPoint snapPoint = {i, snapPointX, snapPointY};
        if (isPlayerAttractive)
        {
            activeSnapPoints[*outActiveSnapPointCount] = snapPoint;
            *outActiveSnapPointCount += 1;
        }

        Particle *snappedParticle = snappedParticles[i];
        if (snappedParticle == NULL)
        {
            continue;
        }

        float deltaX = snappedParticle->x - snapPointX;
        float deltaY = snappedParticle->y - snapPointY;
        float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance >= snappedParticle->radius * config->snapPointRadius)
        {
            continue;
        }

        newSnappedParticles[i] = snappedParticle;
        snappedParticleQueue[bfsPushIndex++] = snappedParticle;

        if (!isPlayerAttractive)
        {
            activeSnapPoints[*outActiveSnapPointCount] = snapPoint;
            *outActiveSnapPointCount += 1;
        }
    }

    while (bfsPopIndex != bfsPushIndex)
    {
        Particle *sourceParticle = snappedParticleQueue[bfsPopIndex++];
        sourceParticle->isSnapped = true;
        int sourceParticleIndex = (sourceParticle - particles) / sizeof(Particle *);
        for (int i = 0; i < 4; i++)
        {
            float snapPointAngle = world->playerAngle + i * PI / 2;
            float snapPointDistance = (sourceParticle->radius * 3) * config->baseParticleRadius;
            float snapPointX = sourceParticle->x + snapPointDistance * cosf(snapPointAngle);
            float snapPointY = sourceParticle->y - snapPointDistance * sinf(snapPointAngle);
            float playerDeltaX = snapPointX - player->x;
            float playerDeltaY = snapPointY - player->y;
            float playerDistance = sqrtf(playerDeltaX * playerDeltaX + playerDeltaY * playerDeltaY);
            if (playerDistance < playerSnapDistance)
            {
                continue;
            }

            int snapPointIndex = 2 + sourceParticleIndex * 4 + i;
            ActiveSnapPoint snapPoint = {snapPointIndex, snapPointX, snapPointY};
            if (isPlayerAttractive)
            {
                activeSnapPoints[*outActiveSnapPointCount] = snapPoint;
                *outActiveSnapPointCount += 1;
            }

            Particle *attractedParticle = snappedParticles[snapPointIndex];
            if (attractedParticle == NULL)
            {
                continue;
            }

            float deltaX = attractedParticle->x - snapPointX;
            float deltaY = attractedParticle->y - snapPointY;
            float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            if (distance >= attractedParticle->radius * config->snapPointRadius)
            {
                continue;
            }

            newSnappedParticles[snapPointIndex] = attractedParticle;
            if (!isPlayerAttractive)
            {
                activeSnapPoints[*outActiveSnapPointCount] = snapPoint;
                *outActiveSnapPointCount += 1;
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

void applySnapPoints(
    World *world, ActiveSnapPoint *activeSnapPoints, int activeSnapPointCount,
    bool isPlayerAttractive)
{
    Config *config = &world->config;
    Particle *player = world->particles;
    Particle **snappedParticles = world->snappedParticles;

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

        float sumXf = 0;
        float sumYf = 0;
        int interactionCount = 0;
        for (int j = 0; j < activeSnapPointCount; j++)
        {
            ActiveSnapPoint snapPoint = activeSnapPoints[j];
            float deltaX = snapPoint.x - particle->x;
            float deltaY = snapPoint.y - particle->y;
            float distance = sqrtf(deltaX * deltaX + deltaY * deltaY);
            float interactionRadius = config->baseParticleRadius;
            if (distance >= interactionRadius)
            {
                continue;
            }

            if (isPlayerAttractive && snappedParticles[snapPoint.index] == NULL)
            {
                snappedParticles[snapPoint.index] = particle;
            }

            sumXf += deltaX / distance * config->snapPointForce;
            sumYf += deltaY / distance * config->snapPointForce;
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
    float wallOffset = config->baseParticleRadius * particle->radius;
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

void worldUpdate(World *world, bool isPlayerAttractive)
{
    Particle *particles = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    Config *config = &world->config;
    ParticleGrid *particleGrid = &world->particleGrid;

    for (int t = 0; t < config->physicsStepsPerFrame; t++)
    {
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

        ActiveSnapPoint activeSnapPoints[SNAP_POINT_COUNT];
        int activeSnapPointCount = 0;
        updateSnappedParticles(
            world, activeSnapPoints, &activeSnapPointCount, isPlayerAttractive);
        applySnapPoints(
            world, activeSnapPoints, activeSnapPointCount, isPlayerAttractive);

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
            (world->particles[0].x + cos(world->playerAngle) * 10) * scaleFactor,
            (world->particles[0].y - sin(world->playerAngle) * 10) * scaleFactor},
        (Vector2){
            (world->particles[0].x) * scaleFactor,
            (world->particles[0].y) * scaleFactor},
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
            float interactionRadius = world->particleTypes[i].radius[j];
            float force = world->particleTypes[i].force[j];
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
                    (cellX + cellWidth / 2) * scaleFactor,
                    (cellY + cellHeight / 2) * scaleFactor},
                (Vector2){
                    (cellX + cellWidth / 2 + force) * scaleFactor,
                    (cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    (cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    (cellY + cellHeight / 2 - force * 0.05f) * scaleFactor},
                (Vector2){
                    (cellX + cellWidth / 2 + force) * scaleFactor,
                    (cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    (cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    (cellY + cellHeight / 2 + force * 0.05f) * scaleFactor},
                (Vector2){
                    (cellX + cellWidth / 2 + force) * scaleFactor,
                    (cellY + cellHeight / 2) * scaleFactor},
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