#include <vector>
#include <random>
#include <thread>
#include <raylib.h>
#include <raymath.h>

constexpr float g_fixedTimeStep = 0.1f; // Ten Hz
Camera2D g_playerCamera = { { 0,0 }, { 0,0 }, 0, 1 };

enum class ResourceType : int
{
	Metal = 1,
	Wood = 2,
	Water = 3,
};
constexpr Color g_resourceColors[] =
{
	BLACK,     // null
	LIGHTGRAY, // Metal
	BROWN,     // Wood
	BLUE,      // Water
};

struct ResourceNode
{
	static constexpr float sizef = 2;
	static constexpr Vector2 sizev = { sizef,sizef };
	Vector2 pos;
	ResourceType type;
	Color GetColor() const { return g_resourceColors[(int)type]; }
};
std::vector<ResourceNode> g_world;

struct ResourcePatch
{
	ResourceNode base;
	float radius;
	int start, end;
};
std::vector<ResourcePatch> g_patches;

namespace WorldGen
{
	enum class WorldGenStage
	{
		AllocatingMemory,
		PlacingPatches,
		GrowingNodes,
		Complete,
	};
	const char* g_stageNames[] =
	{
		"Allocating Memory",
		"Placing Patches",
		"Growing Nodes",
		"Complete",
	};
	std::atomic<WorldGenStage> stage = WorldGenStage::AllocatingMemory;
	std::atomic<float> stageProgress = 0.0f;

	void GenerateWorld()
	{
		std::default_random_engine generator;

		stage.store(WorldGenStage::AllocatingMemory);
		stageProgress.store(0);

		int metalPatches  = std::uniform_int_distribution<int>(20000, 25000)(generator);
		int woodPatches   = std::uniform_int_distribution<int>(40000, 60000)(generator);
		int waterPatches  = std::uniform_int_distribution<int>(9000, 12000)(generator);

		int totalPatches  = metalPatches + woodPatches + waterPatches;

		int totalNodes = 0;
		std::vector<int> patchSizes;
		patchSizes.reserve(totalPatches);
		std::uniform_int_distribution<int> patchSizeDistr(64, 128);

		for (int i = 0; i < totalPatches; ++i)
		{
			int size = patchSizeDistr(generator);
			patchSizes.push_back(size);
			totalNodes += size;
		}

		stageProgress.store(0.125f);
		g_patches.reserve(totalPatches);

		g_world.reserve(totalNodes);

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::PlacingPatches);
		stageProgress.store(0.0f);

		std::uniform_real_distribution<float> xPatchDistr(-100000, 100000);
		std::uniform_real_distribution<float> yPatchDistr(-100000, 100000);

		int runningStart = 0;
		for (int i = 0; i < totalPatches; ++i)
		{
			Vector2 pt =
			{
				xPatchDistr(generator),
				yPatchDistr(generator)
			};
			ResourceType ty;
			if (i < metalPatches)
				ty = ResourceType::Metal;
			else if (i < metalPatches + woodPatches)
				ty = ResourceType::Wood;
			else
				ty = ResourceType::Water;
			int end = runningStart + patchSizes[i];
			g_patches.emplace_back(ResourceNode{ pt, ty }, 0.0f, runningStart, end);
			runningStart = end;
			stageProgress.store((float)i / (float)totalPatches);
		}

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::GrowingNodes);
		stageProgress.store(0.0f);

		float nodeSpread = 1.0f;
		std::uniform_real_distribution<float> nodeAngleDistr(0.0f, 2 * PI);

		for (int i = 0; i < totalPatches; ++i)
		{
			ResourcePatch& thisPatch = g_patches[i];
			int thisPatchSize = patchSizes[i];
			float patchRadius = sqrtf((float)thisPatchSize / PI) * nodeSpread;
			thisPatch.radius = patchRadius;
			std::normal_distribution<float> nodeRadiusDistr(-patchRadius, patchRadius);
			for (int i = 0; i < thisPatchSize; ++i)
			{
				float angle = nodeAngleDistr(generator);
				float length = nodeRadiusDistr(generator);
				Vector2 offset =
				{
					sin(angle) * length,
					cos(angle) * length
				};

				Vector2 pt = Vector2Add(thisPatch.base.pos, offset);
				g_world.emplace_back(pt, thisPatch.base.type);
			}
			stageProgress.store((float)i / (float)totalPatches);
		}

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::Complete);
	}
}

void DrawPatches(Rectangle cullingRect)
{
	for (int i = 0; i < g_patches.size(); ++i)
	{
		ResourcePatch patch = g_patches[i];
		if (!CheckCollisionCircleRec(patch.base.pos, patch.radius, cullingRect))
			continue;

		for (int j = g_patches[i].start; j < g_patches[i].end; ++j)
		{
			ResourceNode node = g_world[j];
			if (!CheckCollisionPointRec(node.pos, cullingRect))
				continue;

			DrawRectangleV(node.pos, node.sizev, node.GetColor());
		}
	}
}

int main()
{
	InitWindow(1280, 720, "Logistics Game");
	Rectangle cullingRect = { 0,0,1280,720 };
	SetTargetFPS(60);

	// World gen
	{
		std::thread worldGen(WorldGen::GenerateWorld);

		while (WorldGen::stage != WorldGen::WorldGenStage::Complete)
		{
			BeginDrawing();
			{
				ClearBackground(BLACK);

				WorldGen::WorldGenStage stage = WorldGen::stage.load();
				float stageProgress = WorldGen::stageProgress.load();

				const char* stageName = WorldGen::g_stageNames[(int)stage];
				DrawText("Generating world", 4, 4, 8, WHITE);
				DrawRectangle(4, 20, 100, 16, DARKGRAY);
				DrawRectangle(4, 20, 100 * stageProgress, 16, BLUE);
				DrawText(stageName, 4, 20, 8, LIGHTGRAY);
			}
			EndDrawing();
		}
		worldGen.join();
	}

	RenderTexture nanite = LoadRenderTexture(1280, 720);
	float lastFixedUpdate = -INFINITY;

	auto RefreshResourceTexture = [&]()
	{
		BeginTextureMode(nanite);
		{
			ClearBackground({ 0,0,0,0 });
			BeginMode2D(g_playerCamera);
			DrawPatches(cullingRect);
			EndMode2D();
		}
		EndTextureMode();
	};

	RefreshResourceTexture();

	while (!WindowShouldClose())
	{
		float now = GetTime();

		// Fixed Update (every g_fixedTimeStep)
		if ((lastFixedUpdate - now) >= g_fixedTimeStep)
		{
			lastFixedUpdate = now;

			// todo
		}

		// Update (every frame)
		{
			bool worldDirty = false;
			if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			{
				g_playerCamera.offset = Vector2Add(g_playerCamera.offset, GetMouseDelta());
				worldDirty = true;
			}

			cullingRect.x = -g_playerCamera.offset.x;
			cullingRect.y = -g_playerCamera.offset.y;

			if (worldDirty)
				RefreshResourceTexture();
		}

		// Frame
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			DrawTextureRec(nanite.texture, { 0,0,1280,-720 }, { 0,0 }, WHITE);

			int width = MeasureText("999 FPS", 20);
			DrawRectangle(2,2, width, 20, RAYWHITE);
			DrawFPS(2,2);
		}
		EndDrawing();
	}

	UnloadRenderTexture(nanite);

	CloseWindow();

	return 0;
}
