#include <stdio.h>
#include "raylib.h"
#include "world.c"

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

int main() {
	float scaleFactor = 2;
	float worldWidth = 600;
	float sidebarWidth = 800;
	float height = 800;
	int windowWidth = (worldWidth + sidebarWidth) * scaleFactor;
	int windowHeight = height * scaleFactor;

	int overlayFontSize = 30;
	int overlayInset = 10;

	World world;
	worldInit(&world, scaleFactor, worldWidth, sidebarWidth, height);

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	ClearBackground(BLACK);

	while (!WindowShouldClose()) {
		int playerAngleDelta = (int)(IsKeyDown(KEY_A)) - (int)(IsKeyDown(KEY_D));
		int playerThrottle = IsKeyDown(KEY_W);
		worldMovePlayer(&world, playerThrottle, playerAngleDelta);

		bool isAttracting = IsKeyDown(KEY_SPACE);
		float updateStartTime = GetTime();
		worldUpdate(&world, isAttracting);
		float updateEndTime = GetTime();
		float updateTimeMs = (updateEndTime - updateStartTime) * 1000;

		// Draw
		BeginDrawing();

		float renderStartTime = GetTime();
		worldRender(&world);
		float renderEndTime = GetTime();
		float renderTimeMs = (renderEndTime - renderStartTime) * 1000;

		int fps = GetFPS();

		char overlayText[100];
		snprintf(overlayText, NELEMS(overlayText) - 1, "FPS %i", fps);
		DrawText(overlayText, overlayInset, overlayInset, overlayFontSize, WHITE);
		snprintf(overlayText, NELEMS(overlayText) - 1, "UPD %2.2f", updateTimeMs);
		DrawText(overlayText, overlayInset, 2 * overlayInset + overlayFontSize, overlayFontSize, WHITE);
		snprintf(overlayText, NELEMS(overlayText) - 1, "REN %2.2f", renderTimeMs);
		DrawText(overlayText, overlayInset, 3 * overlayInset + 2 * overlayFontSize, overlayFontSize, WHITE);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
