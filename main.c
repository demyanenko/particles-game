#include <stdio.h>
#include "raylib.h"
#include "world.c"

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

int main() {
	float scaleFactor = 1.75;
	float width = 900;
	float height = 600;
	int windowWidth = width * scaleFactor;
	int windowHeight = height * scaleFactor;

	int overlayFontSize = 30;
	int overlayInset = 10;

	World world;
	worldInit(&world, scaleFactor, width, height);

	InitWindow(windowWidth, windowHeight, "Window");
	SetTargetFPS(60);

	ClearBackground(BLACK);

	while (!WindowShouldClose()) {
		float updateStartTime = GetTime();
		worldUpdate(&world);
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
