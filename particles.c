#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

#define MAX_STEPS 4

typedef struct
{
    int type;
    float x;
    float y;
    float xv;
    float yv;
    float xf[MAX_STEPS];
    float yf[MAX_STEPS];
} Particle;

void particleInit(Particle *particle, int type, float x, float y)
{
    particle->type = type;
    particle->x = x;
    particle->y = y;
    particle->xv = 0;
    particle->yv = 0;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        particle->xf[i] = 0;
        particle->yf[i] = 0;
    }
}

#define PARTICLE_COUNT 600

typedef struct
{
    Config config;
    ParticleType particleTypes[PARTICLE_TYPE_COUNT];
    Particle particles[PARTICLE_COUNT];
} World;

void worldInit(World *world, float scaleFactor, int width, int height)
{
    configInit(&world->config, scaleFactor, width, height);
    initParticleTypes(world->particleTypes);
    srand(time(0));
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        int particleType = i * PARTICLE_TYPE_COUNT / PARTICLE_COUNT;
        float x = (float)(rand()) / RAND_MAX * width;
        float y = (float)(rand()) / RAND_MAX * height;
        particleInit(world->particles + i, particleType, x, y);
    }
}

void updateParticleInteractions(
    Particle *particles, ParticleType *particleType, Config *config, int particleIndex)
{
    float sumXf = 0;
    float sumYf = 0;
    int interactionCount = 0;
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        if (i == particleIndex)
        {
            continue;
        }

        float deltaX = particles[i].x - particles[particleIndex].x;
        float deltaY = particles[i].y - particles[particleIndex].y;
        float distanceSquared = deltaX * deltaX + deltaY * deltaY;
        float interactionRadius = particleType->radius[particles[i].type];
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
                particles[particleIndex].xv -= repelX * repelAmount;
                particles[particleIndex].yv -= repelY * repelAmount;
            }
            float interactionForce = particleType->force[particles[i].type];
            sumXf += deltaX / distance * interactionForce;
            sumYf += deltaY / distance * interactionForce;
            interactionCount++;
        }
    }

    int lastStepIndex = particleType->steps;
    if (interactionCount > 0)
    {
        particles[particleIndex].xf[lastStepIndex] = sumXf / interactionCount;
        particles[particleIndex].yf[lastStepIndex] = sumYf / interactionCount;
    }
    else
    {
        particles[particleIndex].xf[lastStepIndex] = 0;
        particles[particleIndex].yf[lastStepIndex] = 0;
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
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].xv *= (1.0 - config->friction);
        particles[i].yv *= (1.0 - config->friction);
        particles[i].x += particles[i].xv;
        particles[i].y += particles[i].yv;
        ParticleType *particleType = particleTypes + particles[i].type;
        updateParticleInteractions(particles, particleType, config, i);
        for (int j = 0; j < particleType->steps; j++)
        {
            particles[i].xf[j] = particles[i].xf[j + 1];
            particles[i].yf[j] = particles[i].yf[j + 1];
        }
        particles[i].xv += particles[i].xf[0];
        particles[i].yv += particles[i].yf[0];
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