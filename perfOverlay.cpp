#pragma once

#include "raylib.h"
#include "string.h"
#include "util.cpp"

#define PERF_SAMPLES 360

struct PerfOverlay
{
    int updateParticleInteractionsSamples[PERF_SAMPLES];
    int updateSnappedParticlesSamples[PERF_SAMPLES];
    int applySnapPointsSamples[PERF_SAMPLES];
    int worldUpdateMiscSamples[PERF_SAMPLES];
    int worldRenderSamples[PERF_SAMPLES];
    int currentSample;
    int samplesFilled;
    double scale = 6;
    double height = 200;

    PerfOverlay()
    {
        memset(this->updateParticleInteractionsSamples, 0, sizeof(this->updateParticleInteractionsSamples));
        memset(this->updateSnappedParticlesSamples, 0, sizeof(this->updateSnappedParticlesSamples));
        memset(this->applySnapPointsSamples, 0, sizeof(this->applySnapPointsSamples));
        memset(this->worldUpdateMiscSamples, 0, sizeof(this->worldUpdateMiscSamples));
        memset(this->worldRenderSamples, 0, sizeof(this->worldRenderSamples));
        this->currentSample = 0;
        this->samplesFilled = 0;
    }

    void render(
        int x, int y, double worldUpdateTime, double updateParticleInteractionsTime,
        double updateSnappedParticlesTime, double applySnapPointsTime, double worldRenderTime)
    {
        this->updateParticleInteractionsSamples[this->currentSample] = updateParticleInteractionsTime * 1000 * scale;
        this->updateSnappedParticlesSamples[this->currentSample] = updateSnappedParticlesTime * 1000 * scale;
        this->applySnapPointsSamples[this->currentSample] = applySnapPointsTime * 1000 * scale;
        this->worldUpdateMiscSamples[this->currentSample] =
            (worldUpdateTime - updateParticleInteractionsTime - updateSnappedParticlesTime -
             applySnapPointsTime) *
            1000 * scale;
        this->worldRenderSamples[this->currentSample] = worldRenderTime * 1000 * scale;
        this->samplesFilled = max(this->currentSample + 1, this->samplesFilled);

        float currentX = x + currentSample;
        DrawLineEx(
            Vector2({currentX, float(y + this->height)}),
            Vector2({currentX, float(y)}),
            1, BROWN);

        for (int i = 0; i < this->samplesFilled; i++)
        {
            float currentX = x + i;
            float currentY = y + this->height;

            DrawLineEx(
                Vector2({currentX, currentY}),
                Vector2({currentX, currentY - this->updateParticleInteractionsSamples[i]}),
                1, DARKBLUE);
            currentY -= this->updateParticleInteractionsSamples[i];

            DrawLineEx(
                Vector2({currentX, currentY}),
                Vector2({currentX, currentY - this->updateSnappedParticlesSamples[i]}),
                1, DARKBROWN);
            currentY -= this->updateSnappedParticlesSamples[i];

            DrawLineEx(
                Vector2({currentX, currentY}),
                Vector2({currentX, currentY - this->applySnapPointsSamples[i]}),
                1, DARKGRAY);
            currentY -= this->applySnapPointsSamples[i];

            DrawLineEx(
                Vector2({currentX, currentY}),
                Vector2({currentX, currentY - this->worldUpdateMiscSamples[i]}),
                1, DARKGREEN);
            currentY -= this->worldUpdateMiscSamples[i];

            DrawLineEx(
                Vector2({currentX, currentY}),
                Vector2({currentX, currentY - this->worldRenderSamples[i]}),
                1, DARKPURPLE);
        }
        
        float targetY = y + this->height - 1000.0 / 60 * scale;
        DrawLineEx(
            Vector2({float(x), targetY}),
            Vector2({float(x + PERF_SAMPLES), targetY}),
            1, RED);

        this->currentSample++;
    }
};