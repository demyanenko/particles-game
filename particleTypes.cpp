#pragma once

#include "config.cpp"
#include "raylib.h"

#define PARTICLE_TYPE_COUNT 3
#define PLAYER_PARTICLE_TYPE 0
#define ARMOR_PARTICLE_TYPE 1
#define BULLET_PARTICLE_TYPE 2

struct ParticleType
{
    float force[PARTICLE_TYPE_COUNT];
    float radius[PARTICLE_TYPE_COUNT];
    float steps;
    float friction;
    bool isSnappable;
    int defaultHp;
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

    double maxForce = 3600 * (0.5 + 2.0 * getRandomDouble());
    double maxInteractionRadius = 30 * (0.2 + 0.8 * getRandomDouble());
    bool useSinForce = (rand() & 4) == 0;
    for (int i = 1; i < PARTICLE_TYPE_COUNT; i++)
    {
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            double randForce = useSinForce
                ? sin(PI * getRandomDouble())
                : getRandomDouble();
            particleTypes[i].force[j] = maxForce * (-1.0 + 2.0 * randForce);
            particleTypes[i].radius[j] = config->baseRepelRadius + maxInteractionRadius * getRandomDouble();
        }

        particleTypes[i].friction = regularFriction;
        particleTypes[i].steps = floor(MAX_STEPS * getRandomDouble()) * PHYSICS_STEPS_PER_FRAME;
        particleTypes[i].isSnappable = (3.0 * getRandomDouble()) <= 2;
    }
}

void initParticleTypes(ParticleType *particleTypes)
{
    float regularFriction = 30;

    particleTypes[0].color = Color({127, 127, 127, 255}); // Player
    particleTypes[1].color = Color({255, 255, 0, 255});   // Armor
    particleTypes[2].color = Color({255, 0, 0, 255});     // Bullet

    particleTypes[0].force[0] = 0;
    particleTypes[0].force[1] = 0;
    particleTypes[0].force[2] = 0;

    particleTypes[0].radius[0] = 1;
    particleTypes[0].radius[1] = 1;
    particleTypes[0].radius[2] = 1;

    particleTypes[0].friction = regularFriction;
    particleTypes[0].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[0].isSnappable = true;
    particleTypes[0].defaultHp = 10;

    particleTypes[1].force[0] = 1800;
    particleTypes[1].force[1] = 36;
    particleTypes[1].force[2] = 36;

    particleTypes[1].radius[0] = 18.5;
    particleTypes[1].radius[1] = 12;
    particleTypes[1].radius[2] = 12;

    particleTypes[1].friction = regularFriction;
    particleTypes[1].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[1].isSnappable = true;
    particleTypes[1].defaultHp = 1;

    particleTypes[2].force[0] = 1800;
    particleTypes[2].force[1] = 36;
    particleTypes[2].force[2] = 0;

    particleTypes[2].radius[0] = 18.5;
    particleTypes[2].radius[1] = 12;
    particleTypes[2].radius[2] = 1;

    particleTypes[2].friction = 0;
    particleTypes[2].steps = 0 * PHYSICS_STEPS_PER_FRAME;
    particleTypes[2].isSnappable = false;
    particleTypes[2].defaultHp = 1;
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