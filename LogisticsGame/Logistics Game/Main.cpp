#include <vector>
#include <random>
#include <thread>
#include <raylib.h>
#include <raymath.h>

Vector2 PointFromAngleAndDistance(float angle, float distance)
{
	return Vector2{
		sin(angle) * distance,
		cos(angle) * distance
	};
}

constexpr float g_fixedTimeStep = 0.1f; // Ten Hz
Camera2D g_playerCamera = { { 0,0 }, { 0,0 }, 0, 1 };
Rectangle g_screenRect = { 0,0,1280,720 };
Vector2 GetWorldPosition(Vector2 pt)
{
	return GetScreenToWorld2D(pt, g_playerCamera);
}

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
	bool visible = true;

	Color GetColor() const { return g_resourceColors[(int)type]; }
	bool OnScreen() const { return visible && CheckCollisionPointRec(pos, g_screenRect); }
	static void Draw(const ResourceNode& node) { DrawRectangleV(node.pos, node.sizev, node.GetColor()); }
};
using Nodes_t     = std::vector<ResourceNode>;
using NodeIter_t  = Nodes_t::iterator;
using NodeCIter_t = Nodes_t::const_iterator;
Nodes_t g_world;

float g_nodeSpread = 6.0f;

struct ResourcePatch
{
	ResourceNode base;
	float radius;
	int startNode, endNode;
	bool empty = false;

	bool OnScreen() const { return !empty && CheckCollisionCircleRec(base.pos, radius, g_screenRect); }

	NodeIter_t begin() const { return g_world.begin() + startNode; }
	NodeCIter_t end()  const { return g_world.begin() + endNode; }
};
using Patches_t    = std::vector<ResourcePatch>;
using PatchIter_t  = Patches_t::iterator;
using PatchCIter_t = Patches_t::const_iterator;
Patches_t g_patches;

namespace WorldGen
{
	std::atomic<bool> timeToGo = false;

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

	std::uniform_int_distribution<int> UniformIntDistrFromCenterAndExtent(int center, int extent)
	{
		return std::uniform_int_distribution(center - extent, center + extent);
	}

	void GenerateWorld()
	{
		std::default_random_engine generator;

		stage.store(WorldGenStage::AllocatingMemory);
		stageProgress.store(0);

		int metalPatches  = UniformIntDistrFromCenterAndExtent(25000, 5000)(generator);
		int woodPatches   = UniformIntDistrFromCenterAndExtent(50000, 5000)(generator);
		int waterPatches  = UniformIntDistrFromCenterAndExtent(15000, 5000)(generator);
		int energyPatches = UniformIntDistrFromCenterAndExtent(10000, 5000)(generator);

		int totalPatches  = metalPatches + woodPatches + waterPatches + energyPatches;

		int totalNodes = 0;
		std::vector<int> patchSizes;
		patchSizes.reserve(totalPatches);
		auto patchSizeDistr = UniformIntDistrFromCenterAndExtent(250, 50);

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
			std::normal_distribution<float> nodeRadiusDistr(0.0f, patchRadius / 6.0f);
			for (int i = 0; i < thisPatchSize; ++i)
			{
				float angle = nodeAngleDistr(generator);
				float length = nodeRadiusDistr(generator);
				Vector2 offset = PointFromAngleAndDistance(angle, length);
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

template<class Callable>
void ForEachVisiblePatch(Callable _Pred)
requires(std::is_invocable_v<Callable, ResourcePatch&>)
{
	for (ResourcePatch& patch : g_patches)
	{
		if (patch.OnScreen())
			_Pred(patch);
	}
}
template<class Callable>
void ForEachVisibleNode(Callable _Pred)
requires(std::is_invocable_v<Callable, ResourceNode&>)
{
	for (ResourcePatch& patch : g_patches)
	{
		if (!patch.OnScreen()) continue;

		for (ResourceNode& node : patch)
		{
			if (node.OnScreen())
				_Pred(node);
		}
	}
}

void UpdateScreenRect()
{
	g_screenRect.x = -g_playerCamera.offset.x;
	g_screenRect.y = -g_playerCamera.offset.y;
	g_screenRect.width = GetScreenWidth();
	g_screenRect.height = GetScreenHeight();
}

void GenerateWorld()
{
	std::thread worldGen(WorldGen::GenerateWorld);

	Vector2 previewPanning = Vector2Zero();

	while (WorldGen::stage != WorldGen::WorldGenStage::Complete)
	{
		if (WindowShouldClose())
		{
			WorldGen::timeToGo = true;
			worldGen.join();
			CloseWindow();
			return;
		}

		UpdateScreenRect();
		Camera2D loadingScreenCam{ {0,0}, {0,0}, 0.0f, 0.075f };

		BeginDrawing();
		WorldGen::WorldGenStage stage = WorldGen::stage.load();
		float stageProgress = WorldGen::stageProgress.load();

		ClearBackground(BLACK);

		loadingScreenCam.offset.x += GetFrameTime() * -40.0f;
		loadingScreenCam.offset.y += GetFrameTime() * -20.0f;

		BeginMode2D(loadingScreenCam);
		for (int i = 0; i < g_patches.size(); ++i)
		{
			const ResourcePatch& patch = g_patches[i];
			Color color = patch.base.GetColor();
			color.a = patch.endNode <= g_world.size() ? 255 : 63;
			DrawPixelV(patch.base.pos, color);
		}
		EndMode2D();

		const char* stageName = WorldGen::g_stageNames[(int)stage];
		DrawText("Generating world", 4, 4, 8, WHITE);
		DrawText(stageName, 4, 20, 8, LIGHTGRAY);
		DrawRectangle(4, 36, 100, 16, DARKGRAY);
		DrawRectangle(4, 36, 100 * stageProgress, 16, BLUE);
		DrawText(TextFormat("%i%%", (int)(stageProgress * 100.0f)), 8, 39, 8, WHITE);
		DrawText(TextFormat("Total patches: %#5i\nTotal nodes: %#8i", g_patches.size(), g_world.size()), 4, 58, 8, LIGHTGRAY);
		EndDrawing();
	}
	worldGen.join();
}

float g_lastFixedUpdate = -INFINITY;
// Every g_fixedTimeStep
void TryFixedUpdate()
{
	// Guard
	{
		float now = GetTime();
		if (g_lastFixedUpdate - now < g_fixedTimeStep)
			return;

		g_lastFixedUpdate = now;
	}

	// Todo
}

// Every frame
void Update()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		g_playerCamera.offset = Vector2Add(g_playerCamera.offset, GetMouseDelta());
	}

	UpdateScreenRect();
}

int g_windowWidth = 1280;
int g_windowHeight = 720;
void DrawFrame()
{
	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode2D(g_playerCamera);
	ForEachVisibleNode(ResourceNode::Draw);
	EndMode2D();

	// FPS counter
	{
		int width = MeasureText(TextFormat("%2i FPS", GetFPS()), 20);
		DrawRectangle(2, 2, width, 20, RAYWHITE);
		DrawFPS(2, 2);
	}
	EndDrawing();
}

void PlayGame()
{
	while (!WindowShouldClose())
	{
		TryFixedUpdate();

		Update();

		DrawFrame();
	}
}

int main()
{
	SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT | ConfigFlags::FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 720, "Logistics Game");
	SetTargetFPS(60);

	GenerateWorld();

	PlayGame();

	CloseWindow();

	return 0;
}
