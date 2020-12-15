#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "particle.c"
#include "particleGrid.c"
#include "particleTypes.c"

typedef struct
{
    float scaleFactor;
    float friction;
    float particleRadius;
    float repelRadius;
    float repelDistance;
    float maxInteractionRadius;
    int width;
    int height;
    float backgroundTransparency;
    Color backgroundColor;
} Config;

void configInit(Config *config, float scaleFactor, int width, int height)
{
    config->friction = 0.2;
    config->particleRadius = 2.7;
    config->repelRadius = 1.6 * config->particleRadius;
    config->repelDistance = 2 * config->repelRadius;
    config->maxInteractionRadius = 30 * config->particleRadius;
    config->scaleFactor = scaleFactor;
    config->width = width;
    config->height = height;
    config->backgroundTransparency = 0.7;
    config->backgroundColor = (Color){0, 0, 0, (1 - config->backgroundTransparency) * 255};
}

#define PARTICLE_COUNT 2000

typedef struct
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
    ParticleGrid particleGrid;

} World;

void worldInit(World *world, float scaleFactor, int width, int height)
{
    configInit(&world->config, scaleFactor, width, height);

    initParticleTypes(world->particleTypes);
    float maxInteractionRadius = 0;
    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            float radius = world->particleTypes[i].radius[j];
            if (radius > maxInteractionRadius)
            {
                maxInteractionRadius = radius;
            }
        }
    }

    srand(time(0));
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        int particleType = i * PARTICLE_TYPE_COUNT / PARTICLE_COUNT;
        float x = (float)(rand()) / RAND_MAX * width;
        float y = (float)(rand()) / RAND_MAX * height;
        particleInit(world->particles + i, particleType, x, y);
    }

    particleGridInit(
        &world->particleGrid, width, height, ceilf(maxInteractionRadius), world->particles,
        PARTICLE_COUNT);
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
            float interactionRadius = particleType->radius[otherParticle->type];
            if (distanceSquared < interactionRadius * interactionRadius)
            {
                if (distanceSquared == 0)
                {
                    printf("Warning: particles occupy exactly same spot\n");
                    continue;
                }

                float distance = sqrtf(distanceSquared);
                if (distance < config->repelDistance)
                {
                    float repelX = deltaX / distance;
                    float repelY = deltaY / distance;
                    float repelAmount = 1.0 - distance / config->repelDistance;
                    particle->xv -= repelX * repelAmount;
                    particle->yv -= repelY * repelAmount;
                }
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
        particle->xf[lastStepIndex] = sumXf / interactionCount;
        particle->yf[lastStepIndex] = sumYf / interactionCount;
    }
    else
    {
        particle->xf[lastStepIndex] = 0;
        particle->yf[lastStepIndex] = 0;
    }
}

void updateWallCollisions(Particle *particle, Config *config)
{
    if (particle->x < config->particleRadius)
    {
        particle->x = config->particleRadius;
        if (particle->xv < 0)
        {
            particle->xv *= -1;
        }
    }
    else if (particle->x > config->width - config->particleRadius)
    {
        particle->x = config->width - config->particleRadius;
        if (particle->xv > 0)
        {
            particle->xv *= -1;
        }
    }

    if (particle->y < config->particleRadius)
    {
        particle->y = config->particleRadius;
        if (particle->yv < 0)
        {
            particle->yv *= -1;
        }
    }
    else if (particle->y > config->height - config->particleRadius)
    {
        particle->y = config->height - config->particleRadius;
        if (particle->yv > 0)
        {
            particle->yv *= -1;
        }
    }
}

void worldUpdate(World *world)
{
    Particle *particles = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    Config *config = &world->config;
    ParticleGrid *particleGrid = &world->particleGrid;

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        Particle *particle = particles + i;
        particle->xv *= (1.0 - config->friction);
        particle->yv *= (1.0 - config->friction);

        int oldCellIndex = particleGridFindCellIndex(particleGrid, particle);
        particle->x += particle->xv;
        particle->y += particle->yv;
        particleGridUpdateCell(particleGrid, particle, i, oldCellIndex);

        ParticleType *particleType = particleTypes + particle->type;
        updateParticleInteractions(world, particleType, i);
        for (int j = 0; j < particleType->steps; j++)
        {
            particle->xf[j] = particle->xf[j + 1];
            particle->yf[j] = particle->yf[j + 1];
        }
        particle->xv += particle->xf[0];
        particle->yv += particle->yf[0];
        updateWallCollisions(&particles[i], config);
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
            world->config.particleRadius * scaleFactor,
            world->particleTypes[particle->type].color);
    }
}