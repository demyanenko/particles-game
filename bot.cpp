#pragma once

#include <stdlib.h>
#include "config.cpp"
#include "particle.cpp"
#include "player.cpp"
#include "util.cpp"

enum class BotState
{
    Grow,
    SeekEnemy,
    Attack
};

struct Bot
{
    BotState state;
    Player *player;
    double waypointX;
    double waypointY;
    double lastWaypointChangeTime;
};

void botInit(Bot *bot, Player *player, Config *config)
{
    bot->state = BotState::Grow;
    bot->player = player;
    bot->waypointX = config->width * (0.1 + 0.8 * getRandomDouble());
    bot->waypointY = config->height * (0.1 + 0.8 * getRandomDouble());
    bot->lastWaypointChangeTime = GetTime();
}

int countSnappedParticles(Bot *bot)
{
    int count = 0;
    for (int i = 0; i < MAX_SNAP_KEY; i++)
    {
        if (bot->player->snappedParticles[i] != NULL)
        {
            count++;
        }
    }
    return count;
}

bool isHumanClose(Particle *botParticle, Particle *humanPlayerParticle, Config *config)
{
    double humanDeltaX = humanPlayerParticle->x - botParticle->x;
    double humanDeltaY = humanPlayerParticle->y - botParticle->y;
    double humanDistanceSq = humanDeltaX * humanDeltaX + humanDeltaY * humanDeltaY;
    return humanDistanceSq < config->botMaxAttackDistance * config->botMaxAttackDistance;
}

void steerTowardsPoint(
    Player *botPlayer, double deltaX, double deltaY, bool *outIsTowardsPoint, int *outAngleDelta)
{
    double throttleDeltaX = botPlayer->angleCos;
    double throttleDeltaY = -botPlayer->angleSin;
    double dotProduct = throttleDeltaX * deltaX + throttleDeltaY * deltaY;
    *outIsTowardsPoint = dotProduct > 0;

    double crossProductZ = throttleDeltaX * deltaY - throttleDeltaY * deltaX;
    bool shouldTurnLeft = crossProductZ < 0;
    *outAngleDelta = -1 + 2 * shouldTurnLeft;
}

void navigateToEndpoint(Bot *bot, Config *config, bool *outIsTowardsEndpoint, int *outAngleDelta)
{
    Particle *particle = bot->player->particle;
    double waypointDeltaX = bot->waypointX - particle->x;
    double waypointDeltaY = bot->waypointY - particle->y;
    double waypointDistanceSq = waypointDeltaX * waypointDeltaX + waypointDeltaY * waypointDeltaY;
    double now = GetTime();
    if (waypointDistanceSq < particle->radius * particle->radius ||
        bot->lastWaypointChangeTime + config->botWaypointChangePeriod < now)
    {
        bot->waypointX = config->width * (0.1 + 0.8 * getRandomDouble());
        bot->waypointY = config->height * (0.1 + 0.8 * getRandomDouble());
        bot->lastWaypointChangeTime = now;
        waypointDeltaX = bot->waypointX - particle->x;
        waypointDeltaY = bot->waypointY - particle->y;
        waypointDistanceSq = waypointDeltaX * waypointDeltaX + waypointDeltaY * waypointDeltaY;
    }

    steerTowardsPoint(bot->player, waypointDeltaX, waypointDeltaY, outIsTowardsEndpoint, outAngleDelta);
}

PlayerInput botUpdate(Bot *bot, Particle *humanPlayerParticle, Config *config)
{
    Particle *particle = bot->player->particle;
    int snappedParticleCount = countSnappedParticles(bot);
    if (snappedParticleCount < config->botMinSnappedParticles)
    {
        bot->state = BotState::Grow;
    }
    else if (isHumanClose(particle, humanPlayerParticle, config))
    {
        bot->state = BotState::Attack;
    }
    else if (snappedParticleCount >= config->botMaxSnappedParticles)
    {
        bot->state = BotState::SeekEnemy;
    }
    else
    {
        bot->state = BotState::Grow;
    }

    switch (bot->state)
    {
    case BotState::Grow:
    {
        bool isTowardsEndpoint;
        int angleDelta;
        navigateToEndpoint(bot, config, &isTowardsEndpoint, &angleDelta);

        return playerInputCreate(isTowardsEndpoint, angleDelta, GrowthMode::Growing, false, 0, 0);
    }

    case BotState::SeekEnemy:
    {
        bool isTowardsEndpoint;
        int angleDelta;
        navigateToEndpoint(bot, config, &isTowardsEndpoint, &angleDelta);
        return playerInputCreate(isTowardsEndpoint, angleDelta, GrowthMode::Maintaining, false, 0, 0);
    }

    case BotState::Attack:
    {
        double humanDeltaX = humanPlayerParticle->x - particle->x;
        double humanDeltaY = humanPlayerParticle->y - particle->y;
        double humanDistanceSq = humanDeltaX * humanDeltaX + humanDeltaY * humanDeltaY;
        bool isNotTooClose = humanDistanceSq > config->botMinAttackDistance * config->botMinAttackDistance;

        bool isTowardsHuman;
        int angleDelta;
        steerTowardsPoint(bot->player, humanDeltaX, humanDeltaY, &isTowardsHuman, &angleDelta);

        int throttleAmount = isTowardsHuman && isNotTooClose;
        return playerInputCreate(
            throttleAmount, angleDelta, GrowthMode::Maintaining, true,
            humanPlayerParticle->x, humanPlayerParticle->y);
    }

    default:
        abortWithMessage("Unknown bot state");
    }
}
