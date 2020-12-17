#pragma once

#include <math.h>
#include <stdlib.h>
#include "particle.cpp"
#include "util.cpp"

struct ParticleCell
{
    Particle **particles;
    int count;
    int capacity;
};

void particleCellInit(ParticleCell *particleCell, int capacity)
{
    particleCell->particles = (Particle **)malloc(sizeof(Particle *) * capacity);
    particleCell->count = 0;
    particleCell->capacity = capacity;
}

void particleCellAdd(
    ParticleCell *particleCell, Particle *particles, Particle *particle, int *particlePosWithinCell)
{
    if (particleCell->count == particleCell->capacity)
    {
        abortWithMessage("Particle cell capacity exceeded");
    }

    int particleIndex = particle - particles;
    particlePosWithinCell[particleIndex] = particleCell->count;
    particleCell->particles[particleCell->count] = particle;
    particleCell->count++;
}

void particleCellRemove(
    ParticleCell *particleCell, Particle *particles, Particle *particle, int *particlePosWithinCell)
{
    int particleIndex = particle - particles;
    int posWithinCell = particlePosWithinCell[particleIndex];
    if (posWithinCell >= particleCell->count)
    {
        abortWithMessage("Position out of bounds");
    }

    int movedParticleIndex = particleCell->particles[particleCell->count - 1] - particles;
    particleCell->particles[posWithinCell] =
        particleCell->particles[particleCell->count - 1];
    particleCell->count--;
    particlePosWithinCell[movedParticleIndex] = posWithinCell;
}

struct ParticleCellCoord
{
    int column;
    int row;
};

struct ParticleGrid
{
    int columnCount;
    int rowCount;
    int cellSize;
    ParticleCell *particleCells;
};

ParticleCellCoord particleGridGetCellCoord(ParticleGrid *particleGrid, double x, double y)
{
    int column = (int)(x / particleGrid->cellSize);
    int row = (int)(y / particleGrid->cellSize);
    int clampedColumn = max(0, min(particleGrid->columnCount - 1, column));
    int clampedRow = max(0, min(particleGrid->rowCount - 1, row));
    return ParticleCellCoord({clampedColumn, clampedRow});
}

int particleGridCellCoordToIndex(ParticleGrid *particleGrid, ParticleCellCoord cellCoord)
{
    return cellCoord.row * particleGrid->columnCount + cellCoord.column;
}

ParticleCellCoord particleGridCellIndexToCoord(ParticleGrid *particleGrid, int particleCellIndex)
{
    return ParticleCellCoord({particleCellIndex % particleGrid->columnCount,
                              particleCellIndex / particleGrid->columnCount});
}

int particleGridFindCellIndex(ParticleGrid *particleGrid, Particle *particle)
{
    ParticleCellCoord cellCoord = particleGridGetCellCoord(particleGrid, particle->x, particle->y);
    return particleGridCellCoordToIndex(particleGrid, cellCoord);
}

void particleGridInit(
    ParticleGrid *particleGrid, int width, int height, int cellSize, Particle *particles,
    int *particleCellIndices, int *particlePosWithinCell, int particleCount)
{
    particleGrid->columnCount = ceilf((float)(width) / cellSize);
    particleGrid->rowCount = ceilf((float)(height) / cellSize);
    particleGrid->cellSize = cellSize;
    int cellCount = particleGrid->columnCount * particleGrid->rowCount;
    particleGrid->particleCells = (ParticleCell *)malloc(sizeof(ParticleCell) * cellCount);
    for (int i = 0; i < cellCount; i++)
    {
        particleCellInit(particleGrid->particleCells + i, particleCount);
    }
    for (int i = 0; i < particleCount; i++)
    {
        Particle *particle = particles + i;
        int cellIndex = particleGridFindCellIndex(particleGrid, particle);
        particleCellIndices[i] = cellIndex;
        particleCellAdd(
            particleGrid->particleCells + cellIndex, particles, particle, particlePosWithinCell);
    }
}

#define MAX_NEIGHBOR_CELLS 9
// #define MAX_NEIGHBOR_CELLS 25

int particleGridGetNeighborhoodIndices(ParticleGrid *particleGrid, ParticleCellCoord centerCellCoord, int *outIndices)
{
    int cellCount = 0;
    // for (int xOffset = -2; xOffset <= 2; xOffset++)
    // {
    //     for (int yOffset = -2; yOffset <= 2; yOffset++)
    for (int xOffset = -1; xOffset <= 1; xOffset++)
    {
        for (int yOffset = -1; yOffset <= 1; yOffset++)
        {
            bool isXWithinBounds = centerCellCoord.column + xOffset >= 0 &&
                                   centerCellCoord.column + xOffset < particleGrid->columnCount;

            bool isYWithinBounds = centerCellCoord.row + yOffset >= 0 &&
                                   centerCellCoord.row + yOffset < particleGrid->rowCount;

            if (!isXWithinBounds || !isYWithinBounds)
            {
                continue;
            }

            ParticleCellCoord cellCoord = ParticleCellCoord({centerCellCoord.column + xOffset,
                                                             centerCellCoord.row + yOffset});
            outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, cellCoord);
        }
    }

    return cellCount;
}

void particleGridUpdateCell(
    ParticleGrid *particleGrid, Particle *particles, Particle *particle, int *particleCellIndex,
    int *particlePosWithinCell)
{
    ParticleCell *particleCells = particleGrid->particleCells;
    int newCellIndex = particleGridFindCellIndex(particleGrid, particle);
    if (newCellIndex != *particleCellIndex)
    {
        particleCellRemove(particleCells + *particleCellIndex, particles, particle, particlePosWithinCell);
        particleCellAdd(particleCells + newCellIndex, particles, particle, particlePosWithinCell);
        *particleCellIndex = newCellIndex;
    }
}