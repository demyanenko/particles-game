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

    int cellIndices[MAX_NEIGHBOR_CELLS];
    int cellCount = particleGridGetNeighborhoodIndices(
        &world->particleGrid, world->particles, cellIndices);
    for (int i = 0; i < cellCount; i++)
    {
        ParticleCellCoord cellCoord =
            particleGridCellIndexToCoord(&world->particleGrid, cellIndices[i]);
        DrawRectangleLinesEx(
            (Rectangle){
                cellCoord.column * world->particleGrid.cellSize * scaleFactor,
                cellCoord.row * world->particleGrid.cellSize * scaleFactor,
                world->particleGrid.cellSize * scaleFactor,
                world->particleGrid.cellSize * scaleFactor},
            scaleFactor,
            (Color){255, 255, 255, 7});
    }

    for (int i = 0; i < world->activeSnapPointCount; i++)
    {
        SnapPoint snapPoint = world->activeSnapPoints[i];
        SnapPointPos snapPointPos = snapPointGetPos(snapPoint, world);
        DrawCircle(
            snapPointPos.x * scaleFactor,
            snapPointPos.y * scaleFactor,
            world->config.snapPointRadius * scaleFactor,
            (Color){255, 255, 255, 31});
    }

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        Particle *particle = world->particles + i;
        DrawCircle(
            particle->x * scaleFactor,
            particle->y * scaleFactor,
            particle->radius * scaleFactor,
            world->particleTypes[particle->type].color);
        if (particle->isSnapped)
        {
            DrawCircle(
                particle->x * scaleFactor,
                particle->y * scaleFactor,
                0.3 * scaleFactor,
                WHITE);
        }
    }

    DrawLineEx(
        (Vector2){
            float(world->particles[0].x + cos(world->playerAngle) * world->particles[0].radius) * scaleFactor,
            float(world->particles[0].y - sin(world->playerAngle) * world->particles[0].radius) * scaleFactor},
        (Vector2){
            float(world->particles[0].x) * scaleFactor,
            float(world->particles[0].y) * scaleFactor},
        0.5 * scaleFactor,
        WHITE);
}