#include <stdio.h>
#include "perfOverlay.cpp"
#include "raylib.h"
#include "world.cpp"
#include "worldRender.cpp"
#include "worldUpdate.cpp"

int main()
{
	float scaleFactor = 8;
	int width = 250;
	int height = 250;
	int windowWidth = width * scaleFactor;
	int windowHeight = height * scaleFactor;

	int overlayFontSize = 30;
	int overlayInset = 10;

	World world;
	worldInit(&world, scaleFactor, width, height);

	PerfOverlay perfOverlay;

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	ClearBackground(BLACK);

	while (!WindowShouldClose())
	{
		int playerAngleDelta = (int)(IsKeyDown(KEY_A)) - (int)(IsKeyDown(KEY_D));
		int playerThrottle = IsKeyDown(KEY_W);
		GrowthMode growthMode = IsKeyDown(KEY_SPACE)
									? GrowthMode::Growing
									: IsKeyDown(KEY_C)
										  ? GrowthMode::Shedding
										  : GrowthMode::Maintaining;
		int isPlayerShooting = IsMouseButtonDown(0);

		double updateParticleInteractionsTime, updateSnappedParticlesTime, applySnapPointsTime;
		double worldUpdateStart = GetTime();
		double mouseX = (double)GetMouseX() / scaleFactor;
		double mouseY = (double)GetMouseY() / scaleFactor;
		worldUpdate(
			&world, playerThrottle, playerAngleDelta, growthMode, isPlayerShooting, mouseX, mouseY,
			&updateParticleInteractionsTime, &updateSnappedParticlesTime, &applySnapPointsTime);
		double worldUpdateEnd = GetTime();
		double worldUpdateTime = worldUpdateEnd - worldUpdateStart;

		// Draw
		BeginDrawing();

		double worldRenderStart = GetTime();
		worldRender(&world);
		double worldRenderEnd = GetTime();
		double worldRenderTime = worldRenderEnd - worldRenderStart;

		int fps = GetFPS();

		char overlayText[100];
		snprintf(overlayText, sizeof(overlayText) - 1, "FPS %i", fps);
		DrawText(overlayText, overlayInset, overlayInset, overlayFontSize, WHITE);

		perfOverlay.render(
			overlayInset, 2 * overlayInset + overlayFontSize,
			worldUpdateTime, updateParticleInteractionsTime, updateSnappedParticlesTime,
			applySnapPointsTime, worldRenderTime);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
