#pragma once
#include "raylib.h"

#define PARTICLE_TYPE_COUNT 3

struct ParticleType
{
    float force[PARTICLE_TYPE_COUNT];
    float radius[PARTICLE_TYPE_COUNT];
    float steps;
    float friction;
    Color color;
};

void initParticleTypes(ParticleType *particleTypes)
{
    // 0 is player
    particleTypes[0].color = (Color){127, 127, 127, 255};
    particleTypes[1].color = (Color){255, 255, 0, 127};
    particleTypes[2].color = (Color){255, 0, 0, 127};

    particleTypes[0].force[1] = 0;
    particleTypes[0].radius[1] = 1;

    particleTypes[0].force[2] = 0;
    particleTypes[0].radius[2] = 0;

    particleTypes[0].friction = 30;
    particleTypes[0].steps = 0;

    particleTypes[1].force[0] = 1800;
    particleTypes[1].radius[0] = 18.5;

    particleTypes[1].force[1] = 36;
    particleTypes[1].radius[1] = 12;

    particleTypes[1].force[2] = 36;
    particleTypes[1].radius[2] = 12;

    particleTypes[1].friction = 30;
    particleTypes[1].steps = 0;

    particleTypes[2].force[0] = 1800;
    particleTypes[2].radius[0] = 18.5;

    particleTypes[2].force[1] = 36;
    particleTypes[2].radius[1] = 12;

    particleTypes[2].force[2] = 0;
    particleTypes[2].radius[2] = 0;

    particleTypes[2].friction = 2;
    particleTypes[2].steps = 0;
}