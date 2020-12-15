#pragma once

#include <math.h>
#include <stdlib.h>
#include "particle.c"
#include "util.c"

typedef struct
{
    Particle **particles;
    int count;
    int capacity;
} ParticleCell;

void particleCellInit(ParticleCell *particleCell, int capacity)
{
    particleCell->particles = malloc(sizeof(Particle *) * capacity);
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

typedef struct
{
    int column;
    int row;
} ParticleCellCoord;

typedef struct
{
    int columnCount;
    int rowCount;
    int cellSize;
    ParticleCell *particleCells;
} ParticleGrid;

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
    particleGrid->particleCells = malloc(sizeof(ParticleCell) * cellCount);
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

// max 9 indices
int particleGridGetNeighborhoodIndices(ParticleGrid *particleGrid, Particle *particle, int *outIndices)
{
    int xOffsets[9] = {0, -1, 0, 1, 1, 1, 0, -1, -1};
    int yOffsets[9] = {0, -1, -1, -1, 0, 1, 1, 1, 0};

    int cellCount = 0;
    ParticleCellCoord centerCellCoord = particleGridGetCellCoord(particleGrid, particle);
    for (int i = 0; i < 9; i++)
    {
        int xOffset = xOffsets[i];
        bool isXWithinBounds = (xOffset == -1 && centerCellCoord.column > 0) ||
            xOffset == 0 ||
            (xOffset == 1 && centerCellCoord.column < particleGrid->columnCount - 1);
        
        int yOffset = yOffsets[i];
        bool isYWithinBounds = (yOffset == -1 && centerCellCoord.row > 0) ||
            yOffset == 0 ||
            (yOffset == 1 && centerCellCoord.row < particleGrid->rowCount - 1);
        
        if (!isXWithinBounds || !isYWithinBounds)
        {
            continue;
        }

        ParticleCellCoord cellCoord = (ParticleCellCoord){
            centerCellCoord.column + xOffset,
            centerCellCoord.row + yOffset};
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, cellCoord);
    }

    return cellCount;
}

void particleGridUpdateCell(
    ParticleGrid *particleGrid, Particle *particle, int particleIndex, int oldCellIndex)
{
    ParticleCell *particleCells = particleGrid->particleCells;
    int newCellIndex = particleGridFindCellIndex(particleGrid, particle);
    if (newCellIndex != oldCellIndex)
    {
        particleCellRemove(particleCells + oldCellIndex, particle);
        particleCellAdd(particleCells + newCellIndex, particle);
        particle->cellIndex = newCellIndex;
    }
}