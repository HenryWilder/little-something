#include <fstream>
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
	bool OnScreen(Rectangle screenRect) const { return visible && CheckCollisionPointRec(pos, screenRect); }
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
	bool empty = false; // Todo: Don't serialize if empty

	bool OnScreen(Rectangle screenRect) const { return !empty && CheckCollisionCircleRec(base.pos, radius, screenRect); }

	NodeIter_t begin() const { return g_world.begin() + startNode; }
	NodeCIter_t end()  const { return g_world.begin() + endNode; }

	static void UpdateDepletion(ResourcePatch& patch)
	{
		patch.empty = true;
		for (const ResourceNode& node : patch)
		{
			if (node.visible)
			{
				patch.empty = false;
				break;
			}
		}
	}
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

template<class _Pred>
void ForEachVisiblePatch(Rectangle screenRect, _Pred f)
requires(std::is_invocable_v<_Pred>)
{
	for (ResourcePatch& patch : g_patches)
		if (patch.OnScreen(screenRect))
			f();
}
template<class _Pred>
void ForEachVisiblePatch(Rectangle screenRect, _Pred f)
requires(std::is_invocable_v<_Pred, ResourcePatch&>)
{
	for (ResourcePatch& patch : g_patches)
		if (patch.OnScreen(screenRect))
			f(patch);
}
template<class _Pred>
void ForEachVisibleNode(Rectangle screenRect, _Pred f)
requires(std::is_invocable_v<_Pred>)
{
	for (ResourcePatch& patch : g_patches)
	{
		if (!patch.OnScreen(screenRect)) continue;

		for (ResourceNode& node : patch)
		{
			if (node.OnScreen(screenRect))
				f();
		}
	}
}
template<class _Pred>
void ForEachVisibleNode(Rectangle screenRect, _Pred f)
requires(std::is_invocable_v<_Pred, ResourceNode&>)
{
	for (ResourcePatch& patch : g_patches)
	{
		if (!patch.OnScreen(screenRect)) continue;

		for (ResourceNode& node : patch)
		{
			if (node.OnScreen(screenRect))
				f(node);
		}
	}
}
template<class _Pred>
void ForEachVisibleNode(Rectangle screenRect, _Pred f)
requires(std::is_invocable_v<_Pred, ResourcePatch&, ResourceNode&>)
{
	for (ResourcePatch& patch : g_patches)
	{
		if (!patch.OnScreen(screenRect)) continue;

		for (ResourceNode& node : patch)
		{
			if (node.OnScreen(screenRect))
				f(patch, node);
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
				WorldGen::timeToGo = true;
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
					color.a = patch.endNode <= g_world.size() ? 255 : 63;
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

	Shader nodeShader = LoadShader("Nodes.vert", "Nodes.frag");
	SetShader

	Image nanite = GenImageColor(1280, 720, { 0,0,0,0 });
	Texture2D naniteBuffer = LoadTextureFromImage(nanite);
	float lastFixedUpdate = -INFINITY;
	float collectionRange = 7.0f;

	std::vector<Vector2> nodeClearBuffer; // Positions of all nodes the last time nanite was redrawn

	auto RefreshResourceTexture = [&]()
	{
		for (Vector2 pt : nodeClearBuffer)
		{
			int index = ((int)pt.y * nanite.width + (int)pt.x) * 4;
			((unsigned char*)nanite.data)[index + 3] = 0; // Set alpha to 0
		}
		nodeClearBuffer.clear();
		size_t nodeCount = 0;
		ForEachVisibleNode(cullingRect, [&nodeCount]() { ++nodeCount; });
		nodeClearBuffer.reserve(nodeCount);
		auto DrawNodeToNanite = [&nanite, &nodeClearBuffer](const ResourceNode& node)
		{
			Vector2 pt = GetWorldToScreen2D(node.pos, g_playerCamera);
			nodeClearBuffer.push_back(pt);
			ImageDrawPixelV(&nanite, pt, node.GetColor());
		};
		ForEachVisibleNode(cullingRect, DrawNodeToNanite);
		UpdateTexture(naniteBuffer, nanite.data);
	};

	RefreshResourceTexture();

	while (!WindowShouldClose())
	{
		float now = GetTime();

		bool worldDirty = false;

		Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), g_playerCamera);

		// Fixed Update (every g_fixedTimeStep)
		if ((now - lastFixedUpdate) >= g_fixedTimeStep)
		{
			lastFixedUpdate = now;

			ForEachVisiblePatch(cullingRect, ResourcePatch::UpdateDepletion);
		}

		// Update (every frame)
		{
			if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
			{
				g_playerCamera.offset = Vector2Add(g_playerCamera.offset, GetMouseDelta());
				cullingRect.x = -g_playerCamera.offset.x;
				cullingRect.y = -g_playerCamera.offset.y;
				worldDirty = true;
			}

			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				auto CollectNode = [&collectionRange, &mouseWorldPos](ResourceNode& node)
				{
					if (CheckCollisionPointCircle(node.pos, mouseWorldPos, collectionRange))
						node.visible = false;
				};
				ForEachVisibleNode(cullingRect, CollectNode);
				worldDirty = true;
			}

			if (worldDirty)
				RefreshResourceTexture();
		}

		// Frame
		BeginDrawing();
		{
			ClearBackground(LIGHTGRAY);

			DrawTextureRec(naniteBuffer, { 0,0,1280,720 }, { 0,0 }, WHITE);

			BeginMode2D(g_playerCamera);
			BeginBlendMode(BLEND_ADDITIVE);
			DrawRing(mouseWorldPos, collectionRange - 4, collectionRange, 0, 360, 36, GRAY);
			EndBlendMode();
			EndMode2D();

			{
				int visiblePatches = 0;
				int visibleNodes = 0;
				ForEachVisiblePatch(cullingRect, [&visiblePatches](){ ++visiblePatches; });
				ForEachVisibleNode(cullingRect, [&visibleNodes]() { ++visibleNodes; });
				DrawText(TextFormat("Visible patches: %i\nVisible nodes: %i", visiblePatches, visibleNodes), 2, 42, 8, MAGENTA);
			}

			// FPS
			{
				int width = MeasureText(TextFormat("%2i FPS", GetFPS()), 20);
				DrawRectangle(2, 2, width, 20, RAYWHITE);
				DrawFPS(2, 2);
			}
		}
		EndDrawing();
	}

	UnloadTexture(naniteBuffer);
	UnloadImage(nanite);

	CloseWindow();

	return 0;
}
