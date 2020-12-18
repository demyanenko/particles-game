#pragma once

#include "raylib.h"
#include "string.h"
#include "util.cpp"

#define PERF_SAMPLES 180

struct PerfSidebar
{
    int currentSample;
    int xScale = 6;
    int yScale = 2;
    double width = 200;

    PerfSidebar()
    {
        currentSample = 0;
    }

    void render(
        int x, int y, double worldUpdateTime, double updateParticleInteractionsTime,
        double updateSnappedParticlesTime, double applySnapPointsTime, double worldRenderTime)
    {
        int updateParticleInteractionsSample = updateParticleInteractionsTime * 1000 * xScale;
        int updateSnappedParticlesSample = updateSnappedParticlesTime * 1000 * xScale;
        int applySnapPointsSample = applySnapPointsTime * 1000 * xScale;
        int worldUpdateMiscSample =
            (worldUpdateTime - updateParticleInteractionsTime - updateSnappedParticlesTime -
             applySnapPointsTime) *
            1000 * xScale;
        int worldRenderSample = worldRenderTime * 1000 * xScale;

        int nextSample = (currentSample + 1) % PERF_SAMPLES;
        DrawRectangle(x, y + nextSample * yScale, width, 2 * yScale, BLACK);

        float sampleX = x;
        float sampleY = y + currentSample * yScale;

        DrawRectangle(sampleX, sampleY, updateParticleInteractionsSample, yScale, DARKBLUE);
        sampleX += updateParticleInteractionsSample;

        DrawRectangle(sampleX, sampleY, updateSnappedParticlesSample, yScale, DARKBROWN);
        sampleX += updateSnappedParticlesSample;

        DrawRectangle(sampleX, sampleY, applySnapPointsSample, yScale, DARKGRAY);
        sampleX += applySnapPointsSample;

        DrawRectangle(sampleX, sampleY, worldUpdateMiscSample, yScale, DARKGREEN);
        sampleX += worldUpdateMiscSample;

        DrawRectangle(sampleX, sampleY, worldRenderSample, yScale, DARKPURPLE);
        sampleX += worldRenderSample;

        DrawRectangle(sampleX, sampleY, x + width - sampleX, yScale, BLACK);

        float targetX = x + 1000.0 / 60 * xScale;
        DrawRectangle(targetX, y, yScale, PERF_SAMPLES * yScale, RED);

        currentSample = nextSample;
    }
};