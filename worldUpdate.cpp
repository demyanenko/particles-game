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
    std::unordered_map<int, Particle *> *snappedParticles = world->currentSnappedParticles;
    std::unordered_map<int, Particle *> *newSnappedParticles =
        world->currentSnappedParticles == &world->snappedParticles1
            ? &world->snappedParticles2
            : &world->snappedParticles1;
    SnapPoint *activeSnapPoints = world->activeSnapPoints;

    newSnappedParticles->clear();

    world->activeSnapPointCount = 0;
    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles[i].isSnapped = false;
        particles[i].isEdge = false;
    }
    particles[0].isSnapped = true;

    if (growthMode == GrowthMode::Shedding)
    {
        world->currentSnappedParticles = newSnappedParticles;
        return;
    }

    SnapPoint snapPointQueue[SNAP_POINT_COUNT];
    int bfsPopIndex = 0;
    int bfsPushIndex = 0;
    bool bfsVisited[MAX_SNAP_KEY];
    memset(bfsVisited, false, sizeof(bfsVisited));
    bool growingSnapPointsVisited[MAX_SNAP_KEY];
    memset(growingSnapPointsVisited, false, sizeof(bfsVisited));
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
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }

        if ((*snappedParticles).count(snapPointKey) == 0)
        {
            continue;
        }

        SnapPointPos snapPointPos = snapPointGetPos(snapPoint, world);
        Particle *snappedParticle = (*snappedParticles)[snapPointKey];
        double deltaX = snappedParticle->x - snapPointPos.x;
        double deltaY = snappedParticle->y - snapPointPos.y;
        double distanceSquared = deltaX * deltaX + deltaY * deltaY;
        double snapRadius = snappedParticle->radius * config->snapPointRadius;
        if (distanceSquared >= snapRadius * snapRadius)
        {
            continue;
        }

        newSnappedParticles->insert({snapPointKey, snappedParticle});
        bfsVisited[snapPointKey] = true;
        snapPointQueue[bfsPushIndex++] = snapPoint;

        if (growthMode == GrowthMode::Maintaining)
        {
            activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
        }
    }

    int currBfsDepth = 1;
    int remainingAtCurrBfsDepth = bfsPushIndex;

    double playerSnapDistance = player->radius + 1;
    while (bfsPopIndex != bfsPushIndex && currBfsDepth <= config->maxSnapDepth)
    {
        SnapPoint sourceSnapPoint = snapPointQueue[bfsPopIndex++];

        int sourceSnapPointKey = snapPointGetKey(sourceSnapPoint);
        Particle *sourceParticle = (*snappedParticles)[sourceSnapPointKey];
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
            SnapPointPos snapPointPos = snapPointGetPos(snapPoint, world);
            double snapPointDeltaX = snapPointPos.x - player->x;
            double snapPointDeltaY = snapPointPos.y - player->y;
            double snapPointDistanceSquared =
                snapPointDeltaX * snapPointDeltaX + snapPointDeltaY * snapPointDeltaY;
            if (snapPointDistanceSquared < playerSnapDistance * playerSnapDistance)
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
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
            }

            if ((*snappedParticles).count(snapPointKey) == 0)
            {
                continue;
            }

            Particle *snappedParticle = (*snappedParticles)[snapPointKey];
            double deltaX = snappedParticle->x - snapPointPos.x;
            double deltaY = snappedParticle->y - snapPointPos.y;
            double distanceSquared = deltaX * deltaX + deltaY * deltaY;
            double snapRadius = snappedParticle->radius * config->snapPointRadius;
            if (distanceSquared >= snapRadius * snapRadius)
            {
                continue;
            }

            newSnappedParticles->insert({snapPointKey, snappedParticle});
            if (growthMode == GrowthMode::Maintaining &&
                currBfsDepth + 1 <= config->maxSnapDepth)
            {
                activeSnapPoints[world->activeSnapPointCount++] = snapPoint;
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

    world->currentSnappedParticles = newSnappedParticles;
}

void applySnapPoints(World *world, GrowthMode growthMode)
{
    Config *config = &world->config;
    Particle *player = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    std::unordered_map<int, Particle *> *snappedParticles = world->currentSnappedParticles;
    SnapPoint *activeSnapPoints = world->activeSnapPoints;

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

    for (int i = 0; i < world->activeSnapPointCount; i++)
    {
        SnapPoint snapPoint = activeSnapPoints[i];
        SnapPointPos snapPointPos = snapPointGetPos(snapPoint, world);

        Particle *closestParticle = NULL;
        double minDistanceSquared = config->snapPointRadius * config->snapPointRadius;
        for (int j = 0; j < particleAroundPlayerCount; j++)
        {
            Particle *particle = particlesAroundPlayer[j];
            if (!particleTypes[particle->type].isSnappable)
            {
                continue;
            }

            double deltaX = snapPointPos.x - particle->x;
            double deltaY = snapPointPos.y - particle->y;
            double distanceSquared = deltaX * deltaX + deltaY * deltaY;
            if (distanceSquared >= config->snapPointRadius * config->snapPointRadius)
            {
                continue;
            }

            if (distanceSquared < minDistanceSquared)
            {
                closestParticle = particle;
                minDistanceSquared = distanceSquared;
            }
        }

        if (closestParticle == NULL)
        {
            continue;
        }

        int snapPointKey = snapPointGetKey(snapPoint);
        if (growthMode == GrowthMode::Growing &&
            (*snappedParticles).count(snapPointKey) == 0)
        {
            snappedParticles->insert({snapPointKey, closestParticle});
        }

        double deltaX = snapPointPos.x - closestParticle->x;
        double deltaY = snapPointPos.y - closestParticle->y;
        double minDistance = sqrt(minDistanceSquared);
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

Particle *popClosestSnappedParticle(World *world, double x, double y)
{
    std::unordered_map<int, Particle *> *snappedParticles = world->currentSnappedParticles;
    double closestDistanceSq = 10000000;
    auto closestIter = snappedParticles->end();
    for (auto it = snappedParticles->begin(); it != snappedParticles->end(); it++)
    {
        Particle *testParticle = it->second;
        double testDistance = pow((testParticle->x - x), 2) + pow((testParticle->y - y), 2);
        if (testDistance < closestDistanceSq && testParticle->isEdge)
        {
            closestDistanceSq = testDistance;
            closestIter = it;
        }
    }
    Particle *out = nullptr;
    if (closestIter != snappedParticles->end())
    {
        out = closestIter->second;
        snappedParticles->erase(closestIter);
    }
    return out;
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

void worldUpdate(
    World *world, int throttleDelta, int angleDelta, GrowthMode growthMode, bool isPlayerShooting,
    double mouseX, double mouseY, double *outUpdateParticleInteractionsTime,
    double *outUpdateSnappedParticlesTime, double *outApplySnapPointsTime)
{
    *outUpdateParticleInteractionsTime = 0;
    *outUpdateSnappedParticlesTime = 0;
    *outApplySnapPointsTime = 0;

    Particle *particles = world->particles;
    ParticleType *particleTypes = world->particleTypes;
    Config *config = &world->config;
    ParticleGrid *particleGrid = &world->particleGrid;

    for (int t = 0; t < config->physicsStepsPerFrame; t++)
    {
        world->playerAngle = fmod(
            world->playerAngle + angleDelta * world->config.playerTurnAmount * config->dt, PI * 2);
        world->playerAngleCos = cos(world->playerAngle);
        world->playerAngleSin = sin(world->playerAngle);

        double throttle = throttleDelta * world->config.playerThrottleAmount;
        world->particles[0].xv += world->playerAngleCos * throttle * config->dt;
        world->particles[0].yv += -world->playerAngleSin * throttle * config->dt;

        double updateParticleInteractionsStart = GetTime();
        for (int i = 0; i < PARTICLE_COUNT; i++)
        {
            Particle *particle = particles + i;
            ParticleType *particleType = particleTypes + particle->type;
            particle->xv *= (1.0 - particleType->friction * config->dt);
            particle->yv *= (1.0 - particleType->friction * config->dt);
            particle->x += particle->xv * config->dt;
            particle->y += particle->yv * config->dt;

            updateParticleInteractions(world, particleType, i);
            for (int j = 0; j < particleType->steps; j++)
            {
                particle->xa[j] = particle->xa[j + 1];
                particle->ya[j] = particle->ya[j + 1];
            }
        }
        double updateParticleInteractionsEnd = GetTime();
        *outUpdateParticleInteractionsTime +=
            updateParticleInteractionsEnd - updateParticleInteractionsStart;

        double updateSnappedParticlesStart = GetTime();
        updateSnappedParticles(world, growthMode);
        double updateSnappedParticlesEnd = GetTime();
        *outUpdateSnappedParticlesTime += updateSnappedParticlesEnd - updateSnappedParticlesStart;
        applySnapPoints(world, growthMode);
        double applySnapPointsEnd = GetTime();
        *outApplySnapPointsTime += applySnapPointsEnd - updateSnappedParticlesEnd;

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

    double now = GetTime();
    if (isPlayerShooting && now > world->playerLastShot + config->playerReloadTime)
    {
        Particle *particleToShoot = popClosestSnappedParticle(world, mouseX, mouseY);
        if (particleToShoot != NULL)
        {
            particleToShoot->type = 2;
            double deltaX = mouseX - particleToShoot->x;
            double deltaY = mouseY - particleToShoot->y;
            double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            particleToShoot->xv = (mouseX - particleToShoot->x) / distance * 1000;
            particleToShoot->yv = (mouseY - particleToShoot->y) / distance * 1000;
            particleToShoot->isSnapped = false;
            world->playerLastShot = now;
        }
    }
}