#pragma once

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
    int posWithinCell;
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
    particle->posWithinCell = -1;
}