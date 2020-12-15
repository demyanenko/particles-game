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
    particleTypes[0].color = (Color){200, 150, 30, 255};
    particleTypes[1].color = (Color){30, 190, 30, 255};

    particleTypes[0].force[0] = 3.4426561966786657;
    particleTypes[0].radius[0] = 53.44378619860082;
    particleTypes[0].force[1] = -2.869507890058424;
    particleTypes[0].radius[1] = 71.45572896784721;
    particleTypes[0].steps = 3;
    particleTypes[1].force[0] = 3.261176823902879;
    particleTypes[1].radius[0] = 58.82597659710637;
    particleTypes[1].force[1] = -0.8682907675914784;
    particleTypes[1].radius[1] = 71.22823268411284;
    particleTypes[1].steps = 1;
}