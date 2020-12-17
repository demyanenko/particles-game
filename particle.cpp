#pragma once

#define MAX_STEPS 4

struct Particle
{
    int type;
    float radius;
    float mass;
    double x;
    double y;
    double xv;
    double yv;
    double xa[MAX_STEPS];
    double ya[MAX_STEPS];
    int cellIndex;
    int posWithinCell;
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
    for (int i = 0; i < MAX_STEPS; i++)
    {
        particle->xa[i] = 0;
        particle->ya[i] = 0;
    }
    particle->cellIndex = -1;
    particle->posWithinCell = -1;
    particle->isSnapped = false;
}