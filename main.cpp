#include <stdio.h>
#include <vector>
#include "perfSidebar.cpp"
#include "raylib.h"
#include "world.cpp"
#include "worldRender.cpp"
#include "worldUpdate.cpp"

void printParticleTypes(World *world, std::vector<int> requestedTypes)
{
	printf("t\tst\t");
	for (int j = 0; j < int(requestedTypes.size()); j++)
	{
		printf("%i_f\t%i_r\t", requestedTypes[j], requestedTypes[j]);
	}
	printf("\n");

	for (int i = 0; i < int(requestedTypes.size()); i++)
	{
		printf(
			"%i\t%i\t",
			requestedTypes[i],
			int(world->particleTypes[requestedTypes[i]].steps / world->config.physicsStepsPerFrame));
		for (int j = 0; j < int(requestedTypes.size()); j++)
		{
			printf(
				"%lf\t%lf\t",
				world->particleTypes[requestedTypes[i]].force[requestedTypes[j]],
				world->particleTypes[requestedTypes[i]].radius[requestedTypes[j]]);
		}
		printf("\n");
	}
}

int main()
{
	float scaleFactor = 5;
	int width = 400;
	int height = 400;
	int rawWidth = width * scaleFactor;
	int sidebarRawWidth = 300;
	int windowWidth = rawWidth + sidebarRawWidth;
	int windowHeight = height * scaleFactor;

	int overlayFontSize = 30;
	int overlayInset = 10;
	int sidebarInset = 50;

	World world;
	worldInit(&world, scaleFactor, width, height);

	PerfSidebar perfSidebar;

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

		if (IsKeyPressed(KEY_R))
		{
			worldInit(&world, scaleFactor, width, height);
		}

		if (IsKeyPressed(KEY_P))
		{
			std::vector<int> requestedParticleTypes;
			if (IsKeyDown(KEY_ONE))
			{
				requestedParticleTypes.push_back(1);
			}
			if (IsKeyDown(KEY_TWO))
			{
				requestedParticleTypes.push_back(2);
			}
			if (IsKeyDown(KEY_THREE))
			{
				requestedParticleTypes.push_back(3);
			}
			if (IsKeyDown(KEY_FOUR))
			{
				requestedParticleTypes.push_back(4);
			}
			if (IsKeyDown(KEY_FIVE))
			{
				requestedParticleTypes.push_back(5);
			}
			if (IsKeyDown(KEY_SIX))
			{
				requestedParticleTypes.push_back(6);
			}
			printParticleTypes(&world, requestedParticleTypes);
		}

		if (IsKeyPressed(KEY_B))
		{
			// Breakpoint
			int a = 0;
		}

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

		DrawRectangle(rawWidth, 0, sidebarInset, windowHeight, BLACK);
		perfSidebar.render(
			rawWidth + sidebarInset, sidebarInset,
			worldUpdateTime, updateParticleInteractionsTime, updateSnappedParticlesTime,
			applySnapPointsTime, worldRenderTime);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
