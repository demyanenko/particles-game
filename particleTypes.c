#pragma once
#include "raylib.h"

#define PARTICLE_TYPE_COUNT 2

typedef struct
{
    float force[PARTICLE_TYPE_COUNT];
    float radius[PARTICLE_TYPE_COUNT];
    float steps;
    Color color;
} ParticleType;

void initParticleTypes(ParticleType *particleTypes)
{
    // 0 is player
    particleTypes[0].color = (Color){127, 127, 127, 255};
    particleTypes[1].color = (Color){255, 255, 0, 255};

    particleTypes[0].force[1] = 0;
    particleTypes[0].radius[1] = 1;

    particleTypes[0].steps = 0;

    particleTypes[1].force[0] = 0.5;
    particleTypes[1].radius[0] = 50;

    particleTypes[1].force[1] = 0.5;
    particleTypes[1].radius[1] = 16.2;

    particleTypes[1].steps = 0;
}