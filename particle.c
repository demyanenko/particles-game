#pragma once

#define MAX_STEPS 4

typedef struct
{
    int type;
    float radius;
    float mass;
    float x;
    float y;
    float xv;
    float yv;
    float xa[MAX_STEPS];
    float ya[MAX_STEPS];
    int cellIndex;
    int posWithinCell;
} Particle;

void particleInit(Particle *particle, int type, float x, float y, float size)
{
    particle->type = type;
    particle->radius = size;
    particle->mass = size * size;
    particle->x = x;
    particle->y = y;
    particle->xv = 0;
    particle->yv = 0;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        particle->xa[i] = 0;
        particle->ya[i] = 0;
    }
    particle->cellIndex = -1;
    particle->posWithinCell = -1;
}