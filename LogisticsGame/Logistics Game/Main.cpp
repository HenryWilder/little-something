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
	Energy = 4,
};
constexpr Color g_resourceColors[] =
{
	BLACK,     // null
	LIGHTGRAY, // Metal
	BROWN,     // Wood
	BLUE,      // Water
	YELLOW,    // Energy
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

float g_nodeSpread = 2.0f;

struct ResourcePatch
{
	ResourceNode base;
	float radius;
	int start, end;
};
std::vector<ResourcePatch> g_patches;

std::atomic<bool> timeToGo = false;

namespace WorldGen
{
	enum class WorldGenStage
	{
		AllocatingMemory,
		PlacingPatches,
		GrowingNodes_Metal,
		GrowingNodes_Wood,
		GrowingNodes_Water,
		GrowingNodes_Energy,
		Complete,
	};
	const char* g_stageNames[] =
	{
		"Allocating Memory",
		"Sewing seeds",
		"Creating metal",
		"Growing wood",
		"Adding water",
		"Generating energy",
		"Complete",
	};
	std::atomic<WorldGenStage> stage = WorldGenStage::AllocatingMemory;
	std::atomic<float> stageProgress = 0.0f;

	void GenerateWorld()
	{
		std::default_random_engine generator;

		stage.store(WorldGenStage::AllocatingMemory);
		stageProgress.store(0);

		int metalPatches  = std::uniform_int_distribution<int>(10000,  50000)(generator);
		int woodPatches   = std::uniform_int_distribution<int>(20000, 100000)(generator);
		int waterPatches  = std::uniform_int_distribution<int>(10000,  20000)(generator);
		int energyPatches = std::uniform_int_distribution<int>( 5000,  10000)(generator);

		int totalPatches  = metalPatches + woodPatches + waterPatches + energyPatches;

		int totalNodes = 0;
		std::vector<int> patchSizes;
		patchSizes.reserve(totalPatches);
		std::uniform_int_distribution<int> patchSizeDistr(200, 300);

		for (int i = 0; i < totalPatches; ++i)
		{
			int size = patchSizeDistr(generator);
			patchSizes.push_back(size);
			totalNodes += size;

			if (timeToGo) [[unlikely]] return;
		}

		stageProgress.store(0.125f);
		g_patches.reserve(totalPatches);

		g_world.reserve(totalNodes);

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::PlacingPatches);
		stageProgress.store(0.0f);

		std::uniform_real_distribution<float> patchDistr(-30000, 30000);

		int runningStart = 0;
		for (int i = 0; i < totalPatches; ++i)
		{
			Vector2 pt =
			{
				patchDistr(generator),
				patchDistr(generator)
			};
			ResourceType ty;
			if (i < metalPatches)
				ty = ResourceType::Metal;
			else if (i < metalPatches + woodPatches)
				ty = ResourceType::Wood;
			else if (i < metalPatches + woodPatches + waterPatches)
				ty = ResourceType::Water;
			else
				ty = ResourceType::Energy;
			int end = runningStart + patchSizes[i];
			g_patches.emplace_back(ResourceNode{ pt, ty }, 0.0f, runningStart, end);
			runningStart = end;
			stageProgress.store((float)i / (float)totalPatches);

			if (timeToGo) [[unlikely]] return;
		}

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::GrowingNodes_Metal);
		stageProgress.store(0.0f);

		std::uniform_real_distribution<float> nodeAngleDistr(0.0f, 2 * PI);

		for (int i = 0; i < totalPatches; ++i)
		{
			ResourcePatch& thisPatch = g_patches[i];
			stage.store((WorldGenStage)((int)WorldGenStage::PlacingPatches + (int)thisPatch.base.type));

			int thisPatchSize = patchSizes[i];
			float patchRadius = sqrtf((float)thisPatchSize / PI) * g_nodeSpread;
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

				if (timeToGo) [[unlikely]] return;
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
	SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT);
	InitWindow(1280, 720, "Logistics Game");
	Rectangle cullingRect = { 0,0,1280,720 };
	SetTargetFPS(60);

	// World gen
	{
		std::thread worldGen(WorldGen::GenerateWorld);

		Vector2 previewPanning = Vector2Zero();

		while (WorldGen::stage != WorldGen::WorldGenStage::Complete)
		{
			if (WindowShouldClose())
			{
				timeToGo = true;
				worldGen.join();
				CloseWindow();
				return 0;
			}
			BeginDrawing();
			{
				WorldGen::WorldGenStage stage = WorldGen::stage.load();
				float stageProgress = WorldGen::stageProgress.load();

				ClearBackground(BLACK);

				previewPanning.x += GetFrameTime() * -40.0f;
				previewPanning.y += GetFrameTime() * -20.0f;

				for (int i = 0; i < g_patches.size(); ++i)
				{
					const ResourcePatch& patch = g_patches[i];
					Color color = patch.base.GetColor();
					color.a = patch.end <= g_world.size() ? 255 : 63;
					DrawPixelV(Vector2Scale(Vector2Add(patch.base.pos, previewPanning), 0.05f), color);
				}

				const char* stageName = WorldGen::g_stageNames[(int)stage];
				DrawText("Generating world", 4, 4, 8, WHITE);
				DrawText(stageName, 4, 20, 8, LIGHTGRAY);
				DrawRectangle(4, 36, 100, 16, DARKGRAY);
				DrawRectangle(4, 36, 100 * stageProgress, 16, BLUE);
				DrawText(TextFormat("%i%%", (int)(stageProgress * 100.0f)), 8, 39, 8, WHITE);
				DrawText(TextFormat("Total patches: %#5i\nTotal nodes: %#8i", g_patches.size(), g_world.size()), 4, 58, 8, LIGHTGRAY);
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
