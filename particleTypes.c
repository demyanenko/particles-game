#pragma once
#include "raylib.h"

#define PARTICLE_TYPE_COUNT 3

typedef struct
{
    float repelRadius[PARTICLE_TYPE_COUNT];
    float force[PARTICLE_TYPE_COUNT];
    float radius[PARTICLE_TYPE_COUNT];
    float inheritSpeed[PARTICLE_TYPE_COUNT];
    float steps;
    Color color;
} ParticleType;

void initParticleTypes(ParticleType *particleTypes)
{
    particleTypes[0].color = (Color){127, 127, 127, 255};
    particleTypes[1].color = (Color){127, 127, 127, 255};
    particleTypes[2].color = (Color){255, 255, 0, 255};

    // particleTypes[0].repelRadius[0] = 10;
    // particleTypes[0].force[0] = 0;
    // particleTypes[0].radius[0] = 10;

    particleTypes[0].repelRadius[2] = 0;
    particleTypes[0].force[2] = 0;
    particleTypes[0].radius[2] = 1;
    particleTypes[0].inheritSpeed[2] = 0;

    particleTypes[0].steps = 0;

    particleTypes[1].repelRadius[2] = 0;
    particleTypes[1].force[2] = 0;
    particleTypes[1].radius[2] = 1;
    particleTypes[1].inheritSpeed[2] = 0;
    
    particleTypes[1].steps = 0;

    particleTypes[2].repelRadius[0] = 6;
    particleTypes[2].force[0] = 0.5;
    particleTypes[2].radius[0] = 50;
    particleTypes[2].inheritSpeed[0] = 0;
    
    particleTypes[2].repelRadius[1] = 6;
    particleTypes[2].force[1] = 2.5;
    particleTypes[2].radius[1] = 100;
    particleTypes[2].inheritSpeed[1] = 1;

    particleTypes[2].repelRadius[2] = 3;
    particleTypes[2].force[2] = 0.5;
    particleTypes[2].radius[2] = 16.2;
    particleTypes[2].inheritSpeed[2] = 0;

    particleTypes[2].steps = 0;
}