#pragma once

#include "world.cpp"

void updateParticleInteractions(World *world, ParticleType *particleType, int particleIndex)
{
    Particle *particles = world->particles;
    Config *config = &world->config;
    Particle *particle = particles + particleIndex;

    double sumXf = 0;
    double sumYf = 0;
    int interactionCount = 0;

    int cellIndices[MAX_NEIGHBOR_CELLS];
    int cellCount = particleGridGetNeighborhoodIndices(&world->particleGrid, particle, cellIndices);
    for (int i = 0; i < cellCount; i++)
    {
        ParticleCell *particleCell = world->particleGrid.particleCells + cellIndices[i];
        for (int j = 0; j < particleCell->count; j++)
        {
            Particle *otherParticle = particleCell->particles[j];
            if (otherParticle == particle)
            {
                continue;
            }

            double deltaX = otherParticle->x - particle->x;
            double deltaY = otherParticle->y - particle->y;
            double distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (distanceSquared == 0)
            {
                printf("Warning: particles occupy exactly same spot\n");
                continue;
            }

            double repelDistance = config->baseRepelRadius * (particle->radius + otherParticle->radius);
            if (!particle->isSnapped && distanceSquared < repelDistance * repelDistance)
            {
                double distance = sqrt(distanceSquared);
                double repelX = deltaX / distance;
                double repelY = deltaY / distance;
                double repelAmount = config->baseRepelFactor * (1.0 - distance / repelDistance);
                particle->xv -= double(repelX) * repelAmount * config->dt;
                particle->yv -= double(repelY) * repelAmount * config->dt;
            }

            double interactionRadius = particleType->radius[otherParticle->type];
            if (!(particle->isSnapped && otherParticle->isSnapped) &&
                distanceSquared < interactionRadius * interactionRadius)
            {
                double distance = sqrt(distanceSquared);
                double interactionForce = particleType->force[otherParticle->type];
                sumXf += double(deltaX) / distance * interactionForce;
                sumYf += double(deltaY) / distance * interactionForce;
                interactionCount++;
            }
        }
    }

    int lastStepIndex = particleType->steps;
    if (interactionCount > 0)
    {
        particle->xa[lastStepIndex] = sumXf / interactionCount / particle->mass;
        particle->ya[lastStepIndex] = sumYf / interactionCount / particle->mass;
    }
    else
    {
        particle->xa[lastStepIndex] = 0;
        particle->ya[lastStepIndex] = 0;
    }
}

void updateSnappedParticles(World *world, GrowthMode growthMode)
{
    Config *config = &world->config;
    Particle *particles = world->particles;
    Particle *player = particles;
    Particle **snappedParticles = world->snappedParticles;
    ActiveSnapPoint *activeSnapPoints = world->activeSnapPoints;

    world->activeSnapPointCount = 0;
    Particle *newSnappedParticles[SNAP_POINT_COUNT];
    for (int i = 0; i < SNAP_POINT_COUNT; i++)
    {
        newSnappedParticles[i] = NULL;
    }
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].isSnapped = false;
    }
    particles[0].isSnapped = true;

    if (growthMode == GrowthMode::Shedding)
    {
        memcpy(snappedParticles, newSnappedParticles, SNAP_POINT_COUNT * sizeof(Particle *));
        return;
    }

    Particle *snappedParticleQueue[PARTICLE_COUNT];
    int bfsPopIndex = 0;
    int bfsPushIndex = 0;
    bool bfsVisited[PARTICLE_COUNT];
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        bfsVisited[i] = false;
    }

    double playerSnapDistance = player->radius + 1;
    int playerSnapDeltaHs[6] = {-1, -1, -1, 1, 1, 1};
    int playerSnapDeltaVs[6] = {-2, 0, 2, 2, 0, -2};
    double snapStep = 3;
    for (int i = 0; i < 6; i++)
    {
        double snapPointX = player->x;
        double snapPointY = player->y;
        double hAngle = world->playerAngle - PI / 2;
        double vAngle = world->playerAngle;
        while (pow(snapPointX - player->x, 2) + pow(snapPointY - player->y, 2) <
               pow(playerSnapDistance, 2))
        {
            snapPointX += playerSnapDeltaHs[i] * cos(hAngle) * snapStep;
            snapPointY -= playerSnapDeltaHs[i] * sin(hAngle) * snapStep;
            snapPointX += playerSnapDeltaVs[i] * cos(vAngle) * snapStep;
            snapPointY -= playerSnapDeltaVs[i] * sin(vAngle) * snapStep;
        }

        ActiveSnapPoint snapPoint = {i, snapPointX, snapPointY};
        if (growthMode == GrowthMode::Growing)
        {
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }

        Particle *snappedParticle = snappedParticles[i];
        if (snappedParticle == NULL)
        {
            continue;
        }

        double deltaX = snappedParticle->x - snapPointX;
        double deltaY = snappedParticle->y - snapPointY;
        double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance >= snappedParticle->radius * config->snapPointRadius)
        {
            continue;
        }

        newSnappedParticles[i] = snappedParticle;

        int snappedParticleIndex = snappedParticle - particles;
        bfsVisited[snappedParticleIndex] = true;
        snappedParticleQueue[bfsPushIndex++] = snappedParticle;

        if (growthMode == GrowthMode::Maintaining)
        {
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }
    }

    while (bfsPopIndex != bfsPushIndex)
    {
        Particle *sourceParticle = snappedParticleQueue[bfsPopIndex++];
        sourceParticle->isSnapped = true;
        int sourceParticleIndex = sourceParticle - particles;
        for (int i = 0; i < 4; i++)
        {
            double snapPointAngle = world->playerAngle + i * PI / 2;
            double snapPointX = sourceParticle->x + snapStep * cos(snapPointAngle);
            double snapPointY = sourceParticle->y - snapStep * sin(snapPointAngle);
            double playerDeltaX = snapPointX - player->x;
            double playerDeltaY = snapPointY - player->y;
            double playerDistance = sqrt(playerDeltaX * playerDeltaX + playerDeltaY * playerDeltaY);
            if (playerDistance < playerSnapDistance)
            {
                continue;
            }

            int snapPointIndex = 2 + sourceParticleIndex * 4 + i;
            ActiveSnapPoint snapPoint = {snapPointIndex, snapPointX, snapPointY};
            if (growthMode == GrowthMode::Growing)
            {
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
            }

            Particle *snappedParticle = snappedParticles[snapPointIndex];
            if (snappedParticle == NULL)
            {
                continue;
            }

            double deltaX = snappedParticle->x - snapPointX;
            double deltaY = snappedParticle->y - snapPointY;
            double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            if (distance >= snappedParticle->radius * config->snapPointRadius)
            {
                continue;
            }

            newSnappedParticles[snapPointIndex] = snappedParticle;
            if (growthMode == GrowthMode::Maintaining)
            {
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
            }

            int snappedParticleIndex = snappedParticle - particles;
            if (bfsVisited[snappedParticleIndex])
            {
                continue;
            }
            bfsVisited[snappedParticleIndex] = true;
            snappedParticleQueue[bfsPushIndex++] = snappedParticle;
        }
    }

    memcpy(snappedParticles, newSnappedParticles, SNAP_POINT_COUNT * sizeof(Particle *));

    for (int i = 1; i < PARTICLE_COUNT; i++)
    {
        Particle *particle = particles + i;
        if (bfsVisited[i] && !particle->isSnapped)
        {
            int a = 0;
        }
    }
}

void applySnapPoints(World *world, GrowthMode growthMode)
{
    Config *config = &world->config;
    Particle *player = world->particles;
    Particle **snappedParticles = world->snappedParticles;
    ActiveSnapPoint *activeSnapPoints = world->activeSnapPoints;

    Particle *particlesAroundPlayer[PARTICLE_COUNT];
    int particleAroundPlayerCount = 0;

    int cellIndices[MAX_NEIGHBOR_CELLS];
    int cellCount = particleGridGetNeighborhoodIndices(&world->particleGrid, player, cellIndices);
    for (int i = 0; i < cellCount; i++)
    {
        ParticleCell *particleCell = world->particleGrid.particleCells + cellIndices[i];
        for (int j = 0; j < particleCell->count; j++)
        {
            Particle *particle = particleCell->particles[j];
            if (particle == player)
            {
                continue;
            }
            particlesAroundPlayer[particleAroundPlayerCount++] = particle;
        }
    }

    for (int i = 0; i < particleAroundPlayerCount; i++)
    {
        Particle *particle = particlesAroundPlayer[i];

        double sumXf = 0;
        double sumYf = 0;
        int interactionCount = 0;
        for (int j = 0; j < world->activeSnapPointCount; j++)
        {
            ActiveSnapPoint snapPoint = activeSnapPoints[j];
            double deltaX = snapPoint.x - particle->x;
            double deltaY = snapPoint.y - particle->y;
            double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            double interactionRadius = 1;
            if (distance >= interactionRadius)
            {
                continue;
            }

            if (growthMode == GrowthMode::Growing && snappedParticles[snapPoint.index] == NULL)
            {
                snappedParticles[snapPoint.index] = particle;
            }

            sumXf += double(deltaX) / distance * config->snapPointForce;
            sumYf += double(deltaY) / distance * config->snapPointForce;
            interactionCount++;
        }

        if (interactionCount > 0)
        {
            particle->xa[0] += sumXf / interactionCount / particle->mass;
            particle->ya[0] += sumYf / interactionCount / particle->mass;
        }
    }
}

void updateWallCollisions(Particle *particle, Config *config)
{
    double wallOffset = particle->radius;
    if (particle->x < wallOffset)
    {
        particle->x = wallOffset;
        if (particle->xv < 0)
        {
            particle->xv *= -1;
        }
    }
    else if (particle->x > config->width - wallOffset)
    {
        particle->x = config->width - wallOffset;
        if (particle->xv > 0)
        {
            particle->xv *= -1;
        }
    }

    if (particle->y < wallOffset)
    {
        particle->y = wallOffset;
        if (particle->yv < 0)
        {
            particle->yv *= -1;
        }
    }
    else if (particle->y > config->height - wallOffset)
    {
        particle->y = config->height - wallOffset;
        if (particle->yv > 0)
        {
            particle->yv *= -1;
        }
    }
}

int compareInts(const void *a, const void *b)
{
    return *(int *)(a) - *(int *)(b);
}

void validateParticleGrid(ParticleGrid *particleGrid)
{
    int totalParticlesInCells = 0;
    int particlePositions[PARTICLE_COUNT];
    for (int i = 0; i < particleGrid->rowCount * particleGrid->columnCount; i++)
    {
        totalParticlesInCells += particleGrid->particleCells[i].count;
        for (int j = 0; j < particleGrid->particleCells[i].count; j++)
        {
            particlePositions[j] = particleGrid->particleCells[i].particles[j]->posWithinCell;
        }
        qsort(particlePositions, particleGrid->particleCells[i].count, sizeof(int), compareInts);
        for (int j = 0; j < particleGrid->particleCells[i].count - 1; j++)
        {
            if (particlePositions[j + 1] - particlePositions[j] != 1)
            {
                abortWithMessage("Particle positions inconsistent");
            }
        }
    }
    if (totalParticlesInCells != PARTICLE_COUNT)
    {
        abortWithMessage("Particle count inconsistent");
    }
}

void worldUpdate(World *world, int throttleDelta, int angleDelta, GrowthMode growthMode)
{
    Particle *particles = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    Config *config = &world->config;
    ParticleGrid *particleGrid = &world->particleGrid;

    for (int t = 0; t < config->physicsStepsPerFrame; t++)
    {
        world->playerAngle = fmodf(
            world->playerAngle + angleDelta * world->config.playerTurnAmount * config->dt, PI * 2);
        double throttle = throttleDelta * world->config.playerThrottleAmount;
        world->particles[0].xv += cos(world->playerAngle) * throttle * config->dt;
        world->particles[0].yv += -sin(world->playerAngle) * throttle * config->dt;

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            particle->xv *= (1.0 - config->friction * config->dt);
            particle->yv *= (1.0 - config->friction * config->dt);
            particle->x += particle->xv * config->dt;
            particle->y += particle->yv * config->dt;

            ParticleType *particleType = particleTypes + particle->type;
            updateParticleInteractions(world, particleType, i);
            for (int j = 0; j < particleType->steps; j++)
            {
                particle->xa[j] = particle->xa[j + 1];
                particle->ya[j] = particle->ya[j + 1];
            }
        }

        updateSnappedParticles(world, growthMode);
        applySnapPoints(world, growthMode);

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            particle->xv += particle->xa[0] * config->dt;
            particle->yv += particle->ya[0] * config->dt;
            updateWallCollisions(particle, config);
            particleGridUpdateCell(particleGrid, particle, i);
        }

        validateParticleGrid(particleGrid);
    }
}