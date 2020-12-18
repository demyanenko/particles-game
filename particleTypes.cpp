#pragma once

#include "config.cpp"
#include "raylib.h"

#define PARTICLE_TYPE_COUNT 7

struct ParticleType
{
    float force[PARTICLE_TYPE_COUNT];
    float radius[PARTICLE_TYPE_COUNT];
    float steps;
    float friction;
    bool isSnappable;
    Color color;
};

#define MAX_STEPS 4

void initParticleTypesRandom(ParticleType *particleTypes, Config *config)
{
    float regularFriction = 30;

    particleTypes[0].color = Color({127, 127, 127, 255}); // Player
    particleTypes[1].color = Color({191, 0, 0, 255});
    particleTypes[2].color = Color({0, 191, 0, 255});
    particleTypes[3].color = Color({0, 0, 191, 255});
    particleTypes[4].color = Color({191, 191, 0, 255});
    particleTypes[5].color = Color({0, 191, 191, 255});
    particleTypes[6].color = Color({191, 0, 191, 255});

    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        particleTypes[0].force[i] = 0;
        particleTypes[0].radius[i] = 40;
    }

    particleTypes[0].friction = regularFriction;
    particleTypes[0].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[0].isSnappable = true;

    srand(time(0));
    double maxForce = 3600 * (0.5 + 2.0 * rand() / RAND_MAX);
    double maxInteractionRadius = 30 * (0.2 + 0.8 * rand() / RAND_MAX);
    bool useSinForce = (rand() & 4) == 0;
    for (int i = 1; i < PARTICLE_TYPE_COUNT; i++)
    {
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            double randForce = useSinForce
                ? sin(rand() * PI / RAND_MAX)
                : rand() * PI / RAND_MAX;
            particleTypes[i].force[j] = maxForce * (-1.0 + 2.0 * randForce);
            particleTypes[i].radius[j] = config->baseRepelRadius + maxInteractionRadius * rand() / RAND_MAX;
        }

        particleTypes[i].friction = regularFriction;
        particleTypes[i].steps = floor(MAX_STEPS * rand() / RAND_MAX) * PHYSICS_STEPS_PER_FRAME;
        particleTypes[i].isSnappable = (rand() * 3.0 / RAND_MAX) <= 2;
    }
}

void initParticleTypes(ParticleType *particleTypes)
{
    float regularFriction = 30;

    particleTypes[0].color = Color({127, 127, 127, 255}); // Player
    particleTypes[1].color = Color({255, 255, 0, 255});   // Armor
    particleTypes[2].color = Color({255, 0, 0, 255});     // Bullet
    particleTypes[3].color = Color({200, 150, 30, 255});  // Orange
    particleTypes[4].color = Color({30, 190, 30, 255});   // Green

    particleTypes[0].force[0] = 0;
    particleTypes[0].force[1] = 0;
    particleTypes[0].force[2] = 0;
    particleTypes[0].force[3] = 0;
    particleTypes[0].force[4] = 0;

    particleTypes[0].radius[0] = 1;
    particleTypes[0].radius[1] = 1;
    particleTypes[0].radius[2] = 1;
    particleTypes[0].radius[3] = 1;
    particleTypes[0].radius[4] = 1;

    particleTypes[0].friction = regularFriction;
    particleTypes[0].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[0].isSnappable = true;

    particleTypes[1].force[0] = 1800;
    particleTypes[1].force[1] = 36;
    particleTypes[1].force[2] = 36;
    particleTypes[1].force[3] = 0;
    particleTypes[1].force[4] = 0;

    particleTypes[1].radius[0] = 18.5;
    particleTypes[1].radius[1] = 12;
    particleTypes[1].radius[2] = 12;
    particleTypes[1].radius[3] = 1;
    particleTypes[1].radius[4] = 1;

    particleTypes[1].friction = regularFriction;
    particleTypes[1].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[1].isSnappable = true;

    particleTypes[2].force[0] = 1800;
    particleTypes[2].force[1] = 36;
    particleTypes[2].force[2] = 0;
    particleTypes[2].force[3] = 0;
    particleTypes[2].force[4] = 0;

    particleTypes[2].radius[0] = 18.5;
    particleTypes[2].radius[1] = 12;
    particleTypes[2].radius[2] = 1;
    particleTypes[2].radius[3] = 1;
    particleTypes[2].radius[4] = 1;

    particleTypes[2].friction = 2;
    particleTypes[2].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[2].isSnappable = false;

    particleTypes[3].force[0] = 0;
    particleTypes[3].force[1] = 0;
    particleTypes[3].force[2] = 0;
    particleTypes[3].force[3] = 0 * 1800;
    particleTypes[3].force[4] = -2.869507890058424 * 1800;

    particleTypes[3].radius[0] = 1;
    particleTypes[3].radius[1] = 1;
    particleTypes[3].radius[2] = 1;
    particleTypes[3].radius[3] = 58.44378619860082 / 2.7;
    particleTypes[3].radius[4] = 9.923088760356242 / 2.7;

    particleTypes[3].friction = regularFriction;
    particleTypes[3].steps = 3 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[3].isSnappable = false;

    particleTypes[4].force[0] = 0;
    particleTypes[4].force[1] = 0;
    particleTypes[4].force[2] = 0;
    particleTypes[4].force[3] = 3.261176823902879 * 1800;
    particleTypes[4].force[4] = -0.8682907675914784 * 1800;

    particleTypes[4].radius[0] = 1;
    particleTypes[4].radius[1] = 1;
    particleTypes[4].radius[2] = 1;
    particleTypes[4].radius[3] = 53.82597659710637 / 2.7;
    particleTypes[4].radius[4] = 71.22823268411284 / 2.7;

    particleTypes[4].friction = regularFriction;
    particleTypes[4].steps = 1 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[4].isSnappable = false;
}

void saveParticleTypes(ParticleType *particleTypes, int slot)
{
    char filename[10];
    snprintf(filename, 10, "save%i.sav", slot);
    FILE *file = fopen(filename, "w");

    fprintf(file, "%i\n", PARTICLE_TYPE_COUNT);

    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        fprintf(file, "%f - %i_friction\n", particleTypes[i].friction, i);
        fprintf(file, "%i - %i_steps\n", int(particleTypes[i].steps / PHYSICS_STEPS_PER_FRAME), i);
        fprintf(file, "%i - %i_isSnappable\n", particleTypes[i].isSnappable, i);
        
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            fprintf(file, "%f - force_%i<-%i\n", particleTypes[i].force[j], i, j);
            fprintf(file, "%f - radius_%i<-%i\n", particleTypes[i].radius[j], i, j);
        }
    }

    fclose(file);
    printf("Saved %s\n", filename);
}

void loadParticleTypes(ParticleType *particleTypes, int slot)
{
    char filename[10];
    snprintf(filename, 10, "save%i.sav", slot);
    FILE *file = fopen(filename, "r");

    int particleTypeCount;
    fscanf(file, "%i\n", &particleTypeCount);
    assertOrAbort(particleTypeCount == PARTICLE_TYPE_COUNT, "Particle type count mismatch");

    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        fscanf(file, "%f - %*s\n", &particleTypes[i].friction);
        int steps;
        fscanf(file, "%i - %*s\n", &steps);
        particleTypes[i].steps = steps * PHYSICS_STEPS_PER_FRAME;
        int isSnappable;
        fscanf(file, "%i - %*s\n", &isSnappable);
        particleTypes[i].isSnappable = isSnappable;
        
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            float force;
            fscanf(file, "%f - %*s\n", &force);
            particleTypes[i].force[j] = force;
            float radius;
            fscanf(file, "%f - %*s\n", &radius);
            particleTypes[i].radius[j] = radius;
        }
    }

    fclose(file);
    printf("Loaded %s\n", filename);
}