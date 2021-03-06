#pragma once

#include "world.cpp"

void worldRender(World *world)
{
    float scaleFactor = world->config.scaleFactor;

    DrawRectangle(
        0, 0,
        world->config.width * scaleFactor,
        world->config.height * scaleFactor,
        world->config.backgroundColor);

    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        Player *player = world->players + i;
        for (int j = 0; j < player->activeSnapPointCount; j++)
        {
            SnapPoint snapPoint = player->activeSnapPoints[j];
            SnapPointPos snapPointPos = snapPointGetPos(snapPoint, player);
            DrawCircle(
                snapPointPos.x * scaleFactor,
                snapPointPos.y * scaleFactor,
                world->config.snapPointRadius * scaleFactor,
                Color({255, 255, 255, 31}));
        }
    }

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        Particle *particle = world->particles + i;
        Color particleTypeColor = world->particleTypes[particle->type].color;
        Color particleColor = particle->isEdge
                                  ? Color({(unsigned char)((particleTypeColor.r + 255) / 2),
                                           (unsigned char)(particleTypeColor.g / 2),
                                           (unsigned char)(particleTypeColor.b / 2),
                                           particleTypeColor.a})
                                  : particleTypeColor;
        DrawCircle(
            particle->x * scaleFactor,
            particle->y * scaleFactor,
            particle->radius * scaleFactor,
            particleColor);
        if (particle->isSnapped)
        {
            DrawCircle(
                particle->x * scaleFactor,
                particle->y * scaleFactor,
                0.3 * scaleFactor,
                WHITE);
        }
    }

    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        Player *player = world->players + i;
        DrawLineEx(
            Vector2({float(world->particles[i].x + cos(player->angle) * world->particles[i].radius) * scaleFactor,
                     float(world->particles[i].y - sin(player->angle) * world->particles[i].radius) * scaleFactor}),
            Vector2({float(world->particles[i].x) * scaleFactor,
                     float(world->particles[i].y) * scaleFactor}),
            0.5 * scaleFactor,
            WHITE);
        double damageFraction = 1.0 - 1.0 * world->particles[i].hp / world->particleTypes[PLAYER_PARTICLE_TYPE].defaultHp;
        DrawCircle(
            int(world->particles[i].x * scaleFactor),
            int(world->particles[i].y * scaleFactor),
            damageFraction * world->particles[i].radius * scaleFactor,
            Color({255, 0, 0, 127}));
    }

    Particle *playerParticle = world->particles;
    DrawCircle(
        int(playerParticle->x * scaleFactor),
        int(playerParticle->y * scaleFactor),
        playerParticle->radius *scaleFactor,
        Color({0, 0, 255, 63}));

    if (world->config.botCount > 0)
    {
#ifdef WASM
        float sizeAndOffset = 7 * scaleFactor;
        Color scoreColor = {255, 255, 255, 192};
        char scoreLine[100];
        snprintf(scoreLine, 100, "HUMAN: %i", world->humanScore);
        DrawText(scoreLine, sizeAndOffset, sizeAndOffset, sizeAndOffset, scoreColor);
        const char *botScoreFormat = world->config.botCount == 1 ? "BOT: %i" : "BOTS: %i";
        snprintf(scoreLine, 100, botScoreFormat, world->botScore);
        DrawText(scoreLine, sizeAndOffset, 2.5 * sizeAndOffset, sizeAndOffset, scoreColor);
#else
        DrawRectangle(world->config.width * scaleFactor + 50, 600, 300, 100, BLACK);
        char scoreLine[100];
        snprintf(scoreLine, 100, "HUMAN: %i", world->humanScore);
        DrawText(scoreLine, world->config.width * scaleFactor + 50, 50, 30, WHITE);
        snprintf(scoreLine, 100, "BOTS : %i", world->botScore);
        DrawText(scoreLine, world->config.width * scaleFactor + 50, 50, 30, WHITE);
#endif
    }
}