#pragma once

#include <string.h>

#include "particle.cpp"
#include "util.cpp"

enum class GrowthMode
{
    Growing,
    Maintaining,
    Shedding
};

struct PlayerInput
{
    int throttleAmount;
    int angleDelta;
    GrowthMode growthMode;
    bool isShooting;
    double mouseX;
    double mouseY;
};

PlayerInput playerInputCreate(
    int throttleAmount, int angleDelta, GrowthMode growthMode, bool isShooting, double mouseX,
    double mouseY)
{
    return PlayerInput({throttleAmount, angleDelta, growthMode, isShooting, mouseX, mouseY});
}

struct SnapPoint
{
    int u;
    int v;
};

SnapPoint snapPointCreate(int u, int v)
{
    return SnapPoint({u, v});
}

#define MAX_SNAP_RADIUS (MAX_SNAP_DEPTH + 5)
#define MAX_SNAP_DIAMETER (2 * MAX_SNAP_RADIUS)
#define MAX_SNAP_KEY (MAX_SNAP_DIAMETER * MAX_SNAP_DIAMETER)

unsigned int snapPointGetKey(SnapPoint snapPoint)
{
    long long result = MAX_SNAP_DIAMETER * (snapPoint.u + MAX_SNAP_RADIUS) + snapPoint.v + MAX_SNAP_RADIUS;
    assertOrAbort(result >= 0, "Snap point key underflow");
    assertOrAbort(result < MAX_SNAP_KEY, "Snap point key overflow");
    return (unsigned int)result;
}

struct Player
{
    Particle *particle;
    Particle *snappedParticles[MAX_SNAP_KEY];
    SnapPoint activeSnapPoints[MAX_SNAP_KEY];
    int activeSnapPointCount;
    double angle;
    double angleCos;
    double angleSin;
    double lastShotTime;
};

void playerInit(Player *player, Particle *particle)
{
    player->particle = particle;
    memset(player->snappedParticles, 0, MAX_SNAP_KEY * sizeof(Particle *));
    memset(player->activeSnapPoints, 0, sizeof(player->activeSnapPoints));
    player->activeSnapPointCount = 0;
    player->angle = PI / 2;
    player->angleSin = sin(player->angle);
    player->angleCos = cos(player->angle);
    player->lastShotTime = 0;
}

void playerApplyInput(Player *player, PlayerInput playerInput, Config *config)
{
    player->angle = fmod(
        player->angle + playerInput.angleDelta * config->playerTurnAmount * config->dt, PI * 2);
    player->angleCos = cos(player->angle);
    player->angleSin = sin(player->angle);

    double throttle = playerInput.throttleAmount * config->playerThrottleAmount;
    player->particle->xv += player->angleCos * throttle * config->dt;
    player->particle->yv += -player->angleSin * throttle * config->dt;
}