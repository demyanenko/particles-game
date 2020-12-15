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

    int posWithinCell = particleCell->count;
    particle->posWithinCell = posWithinCell;
    particleCell->particles[posWithinCell] = particle;
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

ParticleCellCoord particleCellCoordShift(
    ParticleCellCoord cellCoord, int columnOffset, int rowOffset)
{
    return (ParticleCellCoord){ cellCoord.column + columnOffset, cellCoord.row + rowOffset };
}

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
    return (ParticleCellCoord){ column, row };
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
        particleCellAdd(particleGrid->particleCells + cellIndex, particle);
    }
}

// max 9 indices
int particleGridGetNeighborhoodIndices(ParticleGrid *particleGrid, Particle *particle, int *outIndices)
{
    int cellCount = 0;
    ParticleCellCoord centerCellCoord = particleGridGetCellCoord(particleGrid, particle);
    outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, centerCellCoord);
    
    if (centerCellCoord.column > 0 && centerCellCoord.row > 0)
    {
        ParticleCellCoord northWestCellCoord = particleCellCoordShift(centerCellCoord, -1, -1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, northWestCellCoord);
    }

    if (centerCellCoord.row > 0)
    {
        ParticleCellCoord northCellCoord = particleCellCoordShift(centerCellCoord, 0, -1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, northCellCoord);
    }
    
    if (centerCellCoord.column < particleGrid->columnCount - 1 && centerCellCoord.row > 0)
    {
        ParticleCellCoord northEastCellCoord = particleCellCoordShift(centerCellCoord, 1, -1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, northEastCellCoord);
    }

    if (centerCellCoord.column < particleGrid->columnCount - 1)
    {
        ParticleCellCoord eastCellCoord = particleCellCoordShift(centerCellCoord, 1, 0);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, eastCellCoord);
    }
    
    if (centerCellCoord.column < particleGrid->columnCount - 1 &&
        centerCellCoord.row < particleGrid->rowCount - 1)
    {
        ParticleCellCoord southEastCellCoord = particleCellCoordShift(centerCellCoord, 1, 1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, southEastCellCoord);
    }
    
    if (centerCellCoord.row < particleGrid->rowCount - 1)
    {
        ParticleCellCoord southCellCoord = particleCellCoordShift(centerCellCoord, 0, 1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, southCellCoord);
    }
    
    if (centerCellCoord.column > 0 && centerCellCoord.row < particleGrid->rowCount - 1)
    {
        ParticleCellCoord southWestCellCoord = particleCellCoordShift(centerCellCoord, -1, 1);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, southWestCellCoord);
    }

    if (centerCellCoord.column > 0)
    {
        ParticleCellCoord westCellCoord = particleCellCoordShift(centerCellCoord, -1, 0);
        outIndices[cellCount++] = particleGridCellCoordToIndex(particleGrid, westCellCoord);
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
    }
}