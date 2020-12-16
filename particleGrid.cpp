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

void particleCellAdd(ParticleCell *particleCell, Particle *particle)
{
    if (particleCell->count == particleCell->capacity)
    {
        abortWithMessage("Particle cell capacity exceeded");
    }

    particle->posWithinCell = particleCell->count;
    particleCell->particles[particleCell->count] = particle;
    particleCell->count++;
}

void particleCellRemove(ParticleCell *particleCell, Particle *particle)
{
    int posWithinCell = particle->posWithinCell;
    if (posWithinCell >= particleCell->count)
    {
        abortWithMessage("Position out of bounds");
    }

    particleCell->particles[posWithinCell] =
        particleCell->particles[particleCell->count - 1];
    particleCell->count--;
    particleCell->particles[posWithinCell]->posWithinCell = posWithinCell;
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

ParticleCellCoord particleGridGetCellCoord(ParticleGrid *particleGrid, Particle *particle)
{
    int column = (int)(particle->x / particleGrid->cellSize);
    int row = (int)(particle->y / particleGrid->cellSize);
    int clampedColumn = max(0, min(particleGrid->columnCount - 1, column));
    int clampedRow = max(0, min(particleGrid->rowCount - 1, row));
    return (ParticleCellCoord){clampedColumn, clampedRow};
}

int particleGridCellCoordToIndex(ParticleGrid *particleGrid, ParticleCellCoord cellCoord)
{
    return cellCoord.row * particleGrid->columnCount + cellCoord.column;
}

ParticleCellCoord particleGridCellIndexToCoord(ParticleGrid *particleGrid, int particleCellIndex)
{
    return (ParticleCellCoord){
        particleCellIndex % particleGrid->columnCount,
        particleCellIndex / particleGrid->columnCount};
}

int particleGridFindCellIndex(ParticleGrid *particleGrid, Particle *particle)
{
    ParticleCellCoord cellCoord = particleGridGetCellCoord(particleGrid, particle);
    return particleGridCellCoordToIndex(particleGrid, cellCoord);
}

void particleGridInit(
    ParticleGrid *particleGrid, int width, int height, int cellSize, Particle *particles,
    int particleCount)
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
        particle->cellIndex = cellIndex;
        particleCellAdd(particleGrid->particleCells + cellIndex, particle);
    }
}

#define MAX_NEIGHBOR_CELLS 9
// #define MAX_NEIGHBOR_CELLS 25

int particleGridGetNeighborhoodIndices(ParticleGrid *particleGrid, Particle *particle, int *outIndices)
{
    int cellCount = 0;
    ParticleCellCoord centerCellCoord = particleGridGetCellCoord(particleGrid, particle);
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

            ParticleCellCoord cellCoord = (ParticleCellCoord){
                centerCellCoord.column + xOffset,
                centerCellCoord.row + yOffset};
            outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, cellCoord);
        }
    }

    return cellCount;
}

void particleGridUpdateCell(
    ParticleGrid *particleGrid, Particle *particle, int particleIndex)
{
    ParticleCell *particleCells = particleGrid->particleCells;
    int newCellIndex = particleGridFindCellIndex(particleGrid, particle);
    if (newCellIndex != particle->cellIndex)
    {
        particleCellRemove(particleCells + particle->cellIndex, particle);
        particleCellAdd(particleCells + newCellIndex, particle);
        particle->cellIndex = newCellIndex;
    }
}