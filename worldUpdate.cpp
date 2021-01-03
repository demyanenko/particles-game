#pragma once

#include "world.cpp"

void updateParticleInteractions(World *world, ParticleType *particleType, int particleIndex, int *damageTaken)
{
    Particle *particles = world->particles;
    Config *config = &world->config;
    Particle *particle = particles + particleIndex;

    double sumXf = 0;
    double sumYf = 0;
    int interactionCount = 0;

    int cellIndices[MAX_NEIGHBOR_CELLS];
    ParticleCellCoord particleCellCoord = particleGridGetCellCoord(
        &world->particleGrid, particle->x, particle->y);
    int cellCount = particleGridGetNeighborhoodIndices(
        &world->particleGrid, particleCellCoord, cellIndices);
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
            double distanceSq = deltaX * deltaX + deltaY * deltaY;
            if (distanceSq == 0)
            {
                printf("Warning: particles occupy exactly same spot\n");
                continue;
            }

            double collisionDistance = particle->radius + otherParticle->radius;
            int otherParticleIndex = otherParticle - particles;
            if (distanceSq < collisionDistance * collisionDistance &&
                damageTaken[particleIndex] == 0 &&
                damageTaken[otherParticleIndex] == 0)
            {
                if (particle->type == BULLET_PARTICLE_TYPE || otherParticle->type == BULLET_PARTICLE_TYPE)
                {
                    damageTaken[particleIndex] = 1;
                    damageTaken[otherParticleIndex] = 1;
                }
                else if (particle->type == SHOT_MISSILE_PARTICLE_TYPE || otherParticle->type == SHOT_MISSILE_PARTICLE_TYPE)
                {
                    int damage = min(particle->hp, otherParticle->hp);
                    damageTaken[particleIndex] = damage;
                    damageTaken[otherParticleIndex] = damage;
                }
            }

            double repelDistance = config->baseRepelRadius * (particle->radius + otherParticle->radius);
            if (!particle->isSnapped && distanceSq < repelDistance * repelDistance)
            {
                double distance = sqrt(distanceSq);
                double repelX = deltaX / distance;
                double repelY = deltaY / distance;
                double repelAmount = config->baseRepelFactor * (1.0 - distance / repelDistance);
                particle->xv -= double(repelX) * repelAmount * config->dt;
                particle->yv -= double(repelY) * repelAmount * config->dt;
            }

            double interactionRadius = particleType->radius[otherParticle->type];
            if (!(particle->isSnapped && otherParticle->isSnapped) &&
                distanceSq < interactionRadius * interactionRadius)
            {
                double distance = sqrt(distanceSq);
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

void updateSnappedParticles(Player *player, Config *config, GrowthMode growthMode)
{
    Particle **snappedParticles = player->snappedParticles;
    SnapPoint *activeSnapPoints = player->activeSnapPoints;

    player->activeSnapPointCount = 0;

    if (growthMode == GrowthMode::Shedding)
    {
        memset(snappedParticles, 0, MAX_SNAP_KEY * sizeof(Particle *));
        return;
    }

    SnapPoint snapPointQueue[MAX_SNAP_KEY];
    int bfsPopIndex = 0;
    int bfsPushIndex = 0;

    bool bfsVisited[MAX_SNAP_KEY];
    memset(bfsVisited, false, sizeof(bfsVisited));
    bool growingSnapPointsVisited[MAX_SNAP_KEY];
    memset(growingSnapPointsVisited, false, sizeof(bfsVisited));

    Particle *newSnappedParticles[MAX_SNAP_KEY];
    memset(newSnappedParticles, 0, MAX_SNAP_KEY * sizeof(Particle *));

    SnapPoint playerSnapPoints[6] = {
        snapPointCreate(-1, -2),
        snapPointCreate(1, -2),
        snapPointCreate(2, 0),
        snapPointCreate(1, 2),
        snapPointCreate(-1, 2),
        snapPointCreate(-2, 0)};
    for (int i = 0; i < 6; i++)
    {
        SnapPoint snapPoint = playerSnapPoints[i];

        int snapPointKey = snapPointGetKey(snapPoint);
        if (growthMode == GrowthMode::Growing)
        {
            growingSnapPointsVisited[snapPointKey] = true;
            activeSnapPoints[player->activeSnapPointCount++] = snapPoint;
        }

        Particle *snappedParticle = snappedParticles[snapPointKey];
        if (snappedParticle == NULL)
        {
            continue;
        }

        SnapPointPos snapPointPos = snapPointGetPos(snapPoint, player);
        double deltaX = snappedParticle->x - snapPointPos.x;
        double deltaY = snappedParticle->y - snapPointPos.y;
        double distanceSq = deltaX * deltaX + deltaY * deltaY;
        double snapRadius = config->snapPointRadius;
        if (distanceSq >= snapRadius * snapRadius)
        {
            continue;
        }

        newSnappedParticles[snapPointKey] = snappedParticle;
        bfsVisited[snapPointKey] = true;
        snapPointQueue[bfsPushIndex++] = snapPoint;

        if (growthMode == GrowthMode::Maintaining)
        {
            activeSnapPoints[player->activeSnapPointCount++] = snapPoint;
        }
    }

    int currBfsDepth = 1;
    int remainingAtCurrBfsDepth = bfsPushIndex;

    double playerSnapDistance = player->particle->radius + 1;
    while (bfsPopIndex != bfsPushIndex && currBfsDepth <= config->maxSnapDepth)
    {
        SnapPoint sourceSnapPoint = snapPointQueue[bfsPopIndex++];

        int sourceSnapPointKey = snapPointGetKey(sourceSnapPoint);
        Particle *sourceParticle = snappedParticles[sourceSnapPointKey];
        sourceParticle->isSnapped = true;
        SnapPoint snapPoints[4] = {
            snapPointCreate(sourceSnapPoint.u - 1, sourceSnapPoint.v),
            snapPointCreate(sourceSnapPoint.u + 1, sourceSnapPoint.v),
            snapPointCreate(sourceSnapPoint.u, sourceSnapPoint.v - 1),
            snapPointCreate(sourceSnapPoint.u, sourceSnapPoint.v + 1)};
        int queueContributions = 0;
        for (int i = 0; i < 4; i++)
        {
            SnapPoint snapPoint = snapPoints[i];
            SnapPointPos snapPointPos = snapPointGetPos(snapPoint, player);
            double snapPointDeltaX = snapPointPos.x - player->particle->x;
            double snapPointDeltaY = snapPointPos.y - player->particle->y;
            double snapPointDistanceSq =
                snapPointDeltaX * snapPointDeltaX + snapPointDeltaY * snapPointDeltaY;
            if (snapPointDistanceSq < playerSnapDistance * playerSnapDistance)
            {
                continue;
            }

            int snapPointKey = snapPointGetKey(snapPoint);
            if (bfsVisited[snapPointKey])
            {
                continue;
            }

            if (growthMode == GrowthMode::Growing &&
                currBfsDepth + 1 <= config->maxSnapDepth &&
                !growingSnapPointsVisited[snapPointKey])
            {
                growingSnapPointsVisited[snapPointKey] = true;
                activeSnapPoints[player->activeSnapPointCount++] = snapPoint;
            }

            Particle *snappedParticle = snappedParticles[snapPointKey];
            if (snappedParticle == NULL)
            {
                continue;
            }

            double deltaX = snappedParticle->x - snapPointPos.x;
            double deltaY = snappedParticle->y - snapPointPos.y;
            double distanceSq = deltaX * deltaX + deltaY * deltaY;
            double snapRadius = config->snapPointRadius;
            if (distanceSq >= snapRadius * snapRadius)
            {
                continue;
            }

            newSnappedParticles[snapPointKey] = snappedParticle;
            if (growthMode == GrowthMode::Maintaining &&
                currBfsDepth + 1 <= config->maxSnapDepth)
            {
                activeSnapPoints[player->activeSnapPointCount++] = snapPoint;
            }

            bfsVisited[snapPointKey] = true;
            snapPointQueue[bfsPushIndex++] = snapPoint;
            queueContributions++;
        }

        if (queueContributions == 0)
        {
            sourceParticle->isEdge = true;
        }

        remainingAtCurrBfsDepth--;
        if (remainingAtCurrBfsDepth == 0)
        {
            currBfsDepth++;
            remainingAtCurrBfsDepth = bfsPushIndex - bfsPopIndex;
        }
    }

    memcpy(snappedParticles, newSnappedParticles, MAX_SNAP_KEY * sizeof(Particle *));
}

void applySnapPoints(World *world, Player *player, GrowthMode growthMode)
{
    Config *config = &world->config;
    ParticleType *particleTypes = world->particleTypes;
    Particle **snappedParticles = player->snappedParticles;
    SnapPoint *activeSnapPoints = player->activeSnapPoints;

    for (int i = 0; i < player->activeSnapPointCount; i++)
    {
        SnapPoint snapPoint = activeSnapPoints[i];
        SnapPointPos snapPointPos = snapPointGetPos(snapPoint, player);
        Particle *closestParticle = NULL;
        double minDistanceSq = config->snapPointRadius * config->snapPointRadius;

        int cellIndices[MAX_NEIGHBOR_CELLS];
        ParticleCellCoord snapPointCellCoord = particleGridGetCellCoord(
            &world->particleSnapGrid, snapPointPos.x, snapPointPos.y);
        int cellCount = particleGridGetNeighborhoodIndices(
            &world->particleSnapGrid, snapPointCellCoord, cellIndices);
        for (int j = 0; j < cellCount; j++)
        {
            ParticleCell *particleSnapCell = world->particleSnapGrid.particleCells + cellIndices[j];
            for (int k = 0; k < particleSnapCell->count; k++)
            {
                Particle *particle = particleSnapCell->particles[k];
                if (particle->type == PLAYER_PARTICLE_TYPE)
                {
                    continue;
                }

                if (!particleTypes[particle->type].isSnappable)
                {
                    continue;
                }

                double deltaX = snapPointPos.x - particle->x;
                double deltaY = snapPointPos.y - particle->y;
                double distanceSq = deltaX * deltaX + deltaY * deltaY;
                if (distanceSq >= config->snapPointRadius * config->snapPointRadius)
                {
                    continue;
                }

                if (distanceSq < minDistanceSq)
                {
                    closestParticle = particle;
                    minDistanceSq = distanceSq;
                }
            }
        }

        if (closestParticle == NULL)
        {
            continue;
        }

        int snapPointKey = snapPointGetKey(snapPoint);
        if (growthMode == GrowthMode::Growing && snappedParticles[snapPointKey] == NULL)
        {
            snappedParticles[snapPointKey] = closestParticle;
        }

        double deltaX = snapPointPos.x - closestParticle->x;
        double deltaY = snapPointPos.y - closestParticle->y;
        double minDistance = sqrt(minDistanceSq);
        closestParticle->xa[0] += deltaX / minDistance * config->snapPointForce / closestParticle->mass;
        closestParticle->ya[0] += deltaY / minDistance * config->snapPointForce / closestParticle->mass;
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

Particle *popClosestSnappedParticle(Player *player, double x, double y)
{
    Particle **snappedParticles = player->snappedParticles;
    double closestDistanceSq = 1e20;
    Particle **closestParticle = NULL;
    for (int i = 0; i < MAX_SNAP_KEY; i++)
    {
        Particle *testParticle = snappedParticles[i];
        if (testParticle == NULL)
        {
            continue;
        }

        if (!testParticle->isEdge)
        {
            continue;
        }

        double testDistanceSq = pow((testParticle->x - x), 2) + pow((testParticle->y - y), 2);
        if (testDistanceSq < closestDistanceSq)
        {
            closestDistanceSq = testDistanceSq;
            closestParticle = snappedParticles + i;
        }
    }

    if (closestParticle != NULL)
    {
        Particle *result = *closestParticle;
        *closestParticle = NULL;
        return result;
    }
    else
    {
        return NULL;
    }
}

void worldUpdate(
    World *world, PlayerInput humanPlayerInput, double *outUpdateParticleInteractionsTime,
    double *outUpdateSnappedParticlesTime, double *outApplySnapPointsTime)
{
    *outUpdateParticleInteractionsTime = 0;
    *outUpdateSnappedParticlesTime = 0;
    *outApplySnapPointsTime = 0;

    Particle *particles = world->particles;
    Config *config = &world->config;

    PlayerInput playerInputs[PLAYER_COUNT];
    playerInputs[0] = humanPlayerInput;
    for (int i = 0; i < BOT_COUNT; i++)
    {
        playerInputs[i + 1] = botUpdate(world->bots + i, particles, &world->config);
    }

    for (int t = 0; t < config->physicsStepsPerFrame; t++)
    {
        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            playerApplyInput(world->players + i, playerInputs[i], config);
        }

        int damageTaken[PARTICLE_COUNT];
        memset(damageTaken, 0, sizeof(damageTaken));

        double updateParticleInteractionsStart = GetTime();
        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            ParticleType *particleType = world->particleTypes + particle->type;
            particle->xv *= (1.0 - particleType->friction * config->dt);
            particle->yv *= (1.0 - particleType->friction * config->dt);
            particle->x += particle->xv * config->dt;
            particle->y += particle->yv * config->dt;
            particleGridUpdateCell(
                &world->particleGrid, world->particles, particle, world->particleCellIndices + i, world->particlePosWithinCell);
            particleGridUpdateCell(
                &world->particleSnapGrid, world->particles, particle, world->particleSnapCellIndices + i, world->particlePosWithinSnapCell);

            updateParticleInteractions(world, particleType, i, damageTaken);
            for (int j = 0; j < particleType->steps; j++)
            {
                particle->xa[j] = particle->xa[j + 1];
                particle->ya[j] = particle->ya[j + 1];
            }
        }
        double updateParticleInteractionsEnd = GetTime();
        *outUpdateParticleInteractionsTime +=
            updateParticleInteractionsEnd - updateParticleInteractionsStart;

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            particles[i].hp -= damageTaken[i];
            if (particles[i].hp <= 0)
            {
                if (i == 0)
                {
                    world->botScore++;
                }
                else if (i < 1 + BOT_COUNT)
                {
                    world->humanScore++;
                }

                int particleTypeIndex;
                if (NEW_PARTICLE_GEN)
                {
                    if (particles[i].type == BULLET_PARTICLE_TYPE)
                    {
                        particleTypeIndex = getRandomDouble() < 0.99 ? ARMOR_PARTICLE_TYPE : SHIELD_PARTICLE_TYPE;
                    }
                    else if (particles[i].type == SHOT_MISSILE_PARTICLE_TYPE)
                    {
                        particleTypeIndex = IDLE_MISSILE_PARTICLE_TYPE;
                    }
                    else
                    {
                        particleTypeIndex = particles[i].type;
                    }
                }
                else
                {
                    particleTypeIndex = particles[i].type == BULLET_PARTICLE_TYPE
                                            ? ARMOR_PARTICLE_TYPE
                                            : particles[i].type;
                }

                double x = getRandomDouble() * world->config.width;
                double y = getRandomDouble() * world->config.height;
                ParticleType *particleType = &world->particleTypes[particleTypeIndex];
                particleInit(particles + i, particleTypeIndex, x, y, particles[i].radius, particleType->defaultHp);
            }
        }

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            particles[i].isSnapped = i < PLAYER_COUNT;
            particles[i].isEdge = false;
        }

        for (int i = 0; i < PLAYER_COUNT; i++)
        {
            double updateSnappedParticlesStart = GetTime();
            updateSnappedParticles(world->players + i, config, playerInputs[i].growthMode);
            double updateSnappedParticlesEnd = GetTime();
            *outUpdateSnappedParticlesTime += updateSnappedParticlesEnd - updateSnappedParticlesStart;
            applySnapPoints(world, world->players + i, playerInputs[i].growthMode);
            double applySnapPointsEnd = GetTime();
            *outApplySnapPointsTime += applySnapPointsEnd - updateSnappedParticlesEnd;
        }

        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            particle->xv += particle->xa[0] * config->dt;
            particle->yv += particle->ya[0] * config->dt;
            updateWallCollisions(particle, config);
            particleGridUpdateCell(
                &world->particleGrid, world->particles, particle, world->particleCellIndices + i, world->particlePosWithinCell);
            particleGridUpdateCell(
                &world->particleSnapGrid, world->particles, particle, world->particleSnapCellIndices + i, world->particlePosWithinSnapCell);
        }
    }

    double now = GetTime();
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        Player *player = world->players + i;
        if (playerInputs[i].isShooting && now > player->lastShotTime + config->playerReloadTime)
        {
            Particle *particleToShoot = popClosestSnappedParticle(player, playerInputs[i].mouseX, playerInputs[i].mouseY);
            if (particleToShoot != NULL)
            {
                if (particleToShoot->type == IDLE_MISSILE_PARTICLE_TYPE)
                {
                    particleToShoot->type = SHOT_MISSILE_PARTICLE_TYPE;
                }
                else
                {
                    particleToShoot->type = BULLET_PARTICLE_TYPE;
                }
                double deltaX = playerInputs[i].mouseX - particleToShoot->x;
                double deltaY = playerInputs[i].mouseY - particleToShoot->y;
                double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
                particleToShoot->xv = (playerInputs[i].mouseX - particleToShoot->x) / distance * world->config.bulletSpeed;
                particleToShoot->yv = (playerInputs[i].mouseY - particleToShoot->y) / distance * world->config.bulletSpeed;
                particleToShoot->isSnapped = false;
                player->lastShotTime = now;
            }
        }
    }
}