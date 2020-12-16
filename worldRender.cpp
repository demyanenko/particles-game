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

    int cellWidth = world->config.sidebarWidth / PARTICLE_TYPE_COUNT * 2;
    int cellHeight = world->config.sidebarWidth / PARTICLE_TYPE_COUNT / 2;
    char stepsStr[2];
    int stepsFontSize = 10;
    int stepsOffset = 5;
    for (int i = 0; i < PARTICLE_TYPE_COUNT; i++)
    {
        int cellX = world->config.width + (i / 2) * cellWidth;
        int steps = world->particleTypes[i].steps;
        snprintf(stepsStr, 2, "%i", steps);
        for (int j = 0; j < PARTICLE_TYPE_COUNT; j++)
        {
            int cellY = (i % 2 * PARTICLE_TYPE_COUNT + j) * cellHeight;
            double interactionRadius = world->particleTypes[i].radius[j];
            double force = world->particleTypes[i].force[j];
            DrawCircle(
                (cellX + cellWidth / 2) * scaleFactor,
                (cellY + cellHeight / 2) * scaleFactor,
                scaleFactor,
                world->particleTypes[i].color);
            DrawCircle(
                (cellX + cellWidth / 2 + interactionRadius) * scaleFactor,
                (cellY + cellHeight / 2) * scaleFactor,
                scaleFactor,
                world->particleTypes[j].color);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    float(cellY + cellHeight / 2 - force * 0.05f) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawLineEx(
                (Vector2){
                    float(cellX + cellWidth / 2 + force * 0.9f) * scaleFactor,
                    float(cellY + cellHeight / 2 + force * 0.05f) * scaleFactor},
                (Vector2){
                    float(cellX + cellWidth / 2 + force) * scaleFactor,
                    float(cellY + cellHeight / 2) * scaleFactor},
                scaleFactor,
                GRAY);
            DrawText(
                stepsStr,
                (cellX + cellWidth / 2 + force * 0.5f - stepsFontSize / 2) * scaleFactor,
                (cellY + cellHeight / 2 - stepsFontSize - stepsOffset) * scaleFactor,
                stepsFontSize * scaleFactor,
                GRAY);
        }
    }
}