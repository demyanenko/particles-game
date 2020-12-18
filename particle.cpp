#pragma once

#include "config.cpp"
#include "particleTypes.cpp"

#define MAX_STEPS_PER_FRAME (MAX_STEPS * PHYSICS_STEPS_PER_FRAME)

struct Particle
{
    int type;
    float radius;
    float mass;
    double x;
    double y;
    double xv;
    double yv;
    double xa[MAX_STEPS_PER_FRAME];
    double ya[MAX_STEPS_PER_FRAME];
    bool isSnapped;
    bool isEdge;
};

void particleInit(Particle *particle, int type, double x, double y, float size)
{
    particle->type = type;
    particle->radius = size;
    particle->mass = size * size;
    particle->x = x;
    particle->y = y;
    particle->xv = 0;
    particle->yv = 0;
    for (int i = 0; i < MAX_STEPS_PER_FRAME; i++)
    {
        particle->xa[i] = 0;
        particle->ya[i] = 0;
    }
    particle->isSnapped = false;
}