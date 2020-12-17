#pragma once

#include "raylib.h"
#include "string.h"
#include "util.cpp"

#define PERF_SAMPLES 600

struct PerfSidebar
{
    int currentSample;
    double scale = 6;
    double width = 200;

    PerfSidebar()
    {
        this->currentSample = 0;
    }

    void render(
        int x, int y, double worldUpdateTime, double updateParticleInteractionsTime,
        double updateSnappedParticlesTime, double applySnapPointsTime, double worldRenderTime)
    {
        int updateParticleInteractionsSample = updateParticleInteractionsTime * 1000 * scale;
        int updateSnappedParticlesSample = updateSnappedParticlesTime * 1000 * scale;
        int applySnapPointsSample = applySnapPointsTime * 1000 * scale;
        int worldUpdateMiscSample =
            (worldUpdateTime - updateParticleInteractionsTime - updateSnappedParticlesTime -
             applySnapPointsTime) *
            1000 * scale;
        int worldRenderSample = worldRenderTime * 1000 * scale;

        float sampleX = x;
        float sampleY = y + this->currentSample;

        DrawRectangle(sampleX, sampleY, updateParticleInteractionsSample, 1, DARKBLUE);
        sampleX += updateParticleInteractionsSample;

        DrawRectangle(sampleX, sampleY, updateSnappedParticlesSample, 1, DARKBROWN);
        sampleX += updateSnappedParticlesSample;

        DrawRectangle(sampleX, sampleY, applySnapPointsSample, 1, DARKGRAY);
        sampleX += applySnapPointsSample;

        DrawRectangle(sampleX, sampleY, worldUpdateMiscSample, 1, DARKGREEN);
        sampleX += worldUpdateMiscSample;

        DrawRectangle(sampleX, sampleY, worldRenderSample, 1, DARKPURPLE);
        sampleX += worldRenderSample;

        DrawRectangle(sampleX, sampleY, x + this->width - sampleX, 1, BLACK);

        float targetX = x + 1000.0 / 60 * scale;
        DrawRectangle(targetX, y, 1, PERF_SAMPLES, RED);

        this->currentSample = (this->currentSample + 1) % PERF_SAMPLES;
    }
};