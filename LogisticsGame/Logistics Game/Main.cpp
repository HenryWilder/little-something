#include <vector>
#include <deque>
#include <random>
#include <thread>
#include <memory>
#include <raylib.h>
#include <raymath.h>

using byte = unsigned char;
using uniform_int_t   = std::uniform_int_distribution<int>;
using uniform_float_t = std::uniform_real_distribution<float>;
using normal_float_t  = std::normal_distribution<float>;
using random_engine   = std::default_random_engine;

Vector2 TransformToWorld(Vector2 pt, Camera2D camera)
{
	return Vector2Add(Vector2Scale(pt, camera.zoom), camera.offset);
}

Vector2 PointFromAngleAndDistance(float angle, float distance)
{
	return Vector2{
		sin(angle) * distance,
		cos(angle) * distance
	};
}

Color UniformColor(Color colorMin, Color colorMax, std::default_random_engine& gen)
{
	uniform_int_t r(colorMin.r, colorMax.r);
	uniform_int_t g(colorMin.g, colorMax.g);
	uniform_int_t b(colorMin.b, colorMax.b);
	uniform_int_t a(colorMin.a, colorMax.a);
	return { (byte)r(gen), (byte)b(gen), (byte)g(gen), (byte)a(gen) };
}
Vector2 UniformVector2(float radius, random_engine& g)
{
	static uniform_float_t angleDistr(0,2*PI);
	uniform_float_t lenDirstr(0.0f, radius);
	return PointFromAngleAndDistance(angleDistr(g), lenDirstr(g));
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
	Metal,
	Wood,
	Water,
	Energy,
	_TypeCount, // Not an actual type
};
constexpr Color g_resourceColors[] =
{
	LIGHTGRAY, // Metal
	BROWN,     // Wood
	BLUE,      // Water
	GOLD,      // Energy
};
constexpr const char* ResourceTypeNameFromIndex(int typeIndex)
{
	constexpr const char* names[]
	{
		"Metal",
		"Wood",
		"Water",
		"Energy",
	};
	return names[typeIndex];
}
inline constexpr const char* ResourceTypeName(ResourceType type)
{
	return ResourceTypeNameFromIndex((int)type);
}
constexpr int g_resourceTypes = (int)ResourceType::_TypeCount;

struct ResourceNode
{
	static constexpr float sizef = 2;
	static constexpr Vector2 sizev = { sizef,sizef };

	Vector2 pos;
	ResourceType type;
	bool visible = true;
	bool beingInhaled = false;

	Color GetColor() const { return g_resourceColors[(int)type]; }
	bool OnScreen() const { return visible && CheckCollisionPointRec(pos, g_screenRect); }
	static void Draw(const ResourceNode& node) { DrawRectangleV(node.pos, node.sizev, node.GetColor()); }
};
using Nodes_t     = std::vector<ResourceNode>;
using NodeIter_t  = Nodes_t::iterator;
using NodeCIter_t = Nodes_t::const_iterator;
Nodes_t g_world;

float g_nodeSpread = 10.0f;

struct ResourcePatch
{
	ResourceNode base;
	float radius;
	int startNode, endNode;
	bool empty = false;

	bool OnScreen() const { return !empty && CheckCollisionCircleRec(base.pos, radius, g_screenRect); }
	static void UpdateEmpty(ResourcePatch& patch)
	{
		patch.empty = true;
		for (const ResourceNode& node : patch)
		{
			if (node.visible)
			{
				patch.empty = false;
				return;
			}
		}
	}

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

	uniform_int_t UniformIntDistrFromCenterAndExtent(int center, int extent)
	{
		return uniform_int_t(center - extent, center + extent);
	}

	float g_worldExtent = 30000;

	void GenerateWorld()
	{
		random_engine generator;

		stage.store(WorldGenStage::AllocatingMemory);
		stageProgress.store(0);

		int patchCounts[g_resourceTypes]
		{
			UniformIntDistrFromCenterAndExtent(25000, 5000)(generator), // Metal
			UniformIntDistrFromCenterAndExtent(50000, 5000)(generator), // Wood
			UniformIntDistrFromCenterAndExtent(15000, 5000)(generator), // Water
			UniformIntDistrFromCenterAndExtent(10000, 5000)(generator), // Energy
		};
		int patchEndIndices[g_resourceTypes]{};
		patchEndIndices[0] = patchCounts[0];
		for (int i = 1; i < g_resourceTypes; ++i)
		{
			patchEndIndices[i] = patchEndIndices[i - 1] + patchCounts[i];
		}
		int totalPatches = patchEndIndices[g_resourceTypes - 1];

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

		uniform_float_t patchDistr(-g_worldExtent, g_worldExtent);

		int runningStart = 0;
		for (int i = 0; i < totalPatches; ++i)
		{
			Vector2 pt =
			{
				patchDistr(generator),
				patchDistr(generator)
			};
			ResourceType ty;
#if _DEBUG
			bool initializedType = false;
#endif
			for (int j = 0; j < g_resourceTypes; ++j)
			{
				if (i < patchEndIndices[j])
				{
					_ASSERT_EXPR(j >= 0 && j < g_resourceTypes, L"Resource must be a valid ResourceType");
#if _DEBUG
					initializedType = true;
#endif
					ty = (ResourceType)j;
					break;
				}
			}
#if _DEBUG
			_ASSERT_EXPR(initializedType, L"Resource type was not initialized");
#endif
			int end = runningStart + patchSizes[i];
			g_patches.emplace_back(ResourceNode{ pt, ty }, 0.0f, runningStart, end);
			runningStart = end;
			stageProgress.store((float)i / (float)totalPatches);

			if (timeToGo) [[unlikely]] return;
		}

		stageProgress.store(1.0f);
		stage.store(WorldGenStage::GrowingNodes_Metal);
		stageProgress.store(0.0f);

		uniform_float_t nodeAngleDistr(0.0f, 2.0f * PI);

		for (int i = 0; i < totalPatches; ++i)
		{
			ResourcePatch& thisPatch = g_patches[i];
			stage.store((WorldGenStage)((int)WorldGenStage::PlacingPatches + (int)thisPatch.base.type));

			int thisPatchSize = patchSizes[i];
			float patchRadius = sqrtf((float)thisPatchSize / PI) * g_nodeSpread;
			thisPatch.radius = patchRadius;
			normal_float_t nodeRadiusDistr(0.0f, patchRadius / 6.0f);
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

struct ControlOut;
struct ControlIn;

struct ControlOut
{
private:
	bool lastState = false;
public:
	bool state = false;
	std::weak_ptr<ControlIn> dest;

	// Since last time this was called
	bool IsChanged()
	{
		bool changed = lastState != state;
		lastState = state;
		return changed;
	}
};

struct ControlIn
{
	std::weak_ptr<ControlOut> src;

	bool State() const
	{
		if (src.expired())
			return false;

		auto temp = src.lock();
		return temp->state;
	}
	// Since last time this was called
	// Passthrough to src->IsChanged(); always false if src expires
	bool IsChanged() const
	{
		if (src.expired())
			return false;

		auto temp = src.lock();
		return temp->state;
	}
};

struct TrainTrack;
struct TrainStation;
struct TrainEngine;
struct TrainCar;
struct TrainJunction;

struct TrainTrack
{

};

struct TrainStation
{

};

struct TrainCar
{

};

struct TrainEngine
{

};

struct TrainJunction
{

};

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

	float scale = 0.05f;
	Camera2D previewCam{ { WorldGen::g_worldExtent * scale * 0.5f, WorldGen::g_worldExtent * scale * 0.25f }, { 0,0 }, 0.0f, scale };

	while (WorldGen::stage != WorldGen::WorldGenStage::Complete)
	{
		if (WindowShouldClose())
		{
			WorldGen::timeToGo = true;
			worldGen.join();
			CloseWindow();
			exit(0);
		}

		UpdateScreenRect();

		BeginDrawing();
		WorldGen::WorldGenStage stage = WorldGen::stage.load();
		float stageProgress = WorldGen::stageProgress.load();

		ClearBackground(BLACK);

		previewCam.offset.x += GetFrameTime() * -4.0f;
		previewCam.offset.y += GetFrameTime() * -2.0f;

		for (int i = 0; i < g_patches.size(); ++i)
		{
			const ResourcePatch& patch = g_patches[i];
			Color color = patch.base.GetColor();
			color.a = patch.endNode <= g_world.size() ? 255 : 63;
			DrawPixelV(TransformToWorld(patch.base.pos, previewCam), color);
		}
		Vector2 min = TransformToWorld({ 0,0 }, previewCam);
		Vector2 max = TransformToWorld({ g_screenRect.width,g_screenRect.height }, previewCam);
		Rectangle rec{ min.x, min.y, max.x - min.x, max.y - min.y };
		DrawRectangleLinesEx(rec, 2, RED);

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

int g_itemCounts[g_resourceTypes];

struct SuccEffect
{
	static constexpr float lifetime = 0.25f;
	static constexpr float startAlpha = 200;
	static constexpr int maxSimul = 10; // Maximum at a time

	Vector2 position;
	Vector2 velocity;
	float birthDate;
	float radius;
	float growthRate;
	Color color;
}; 
std::deque<SuccEffect> effects;

void SpawnSuccEffect(Vector2 pos, Color color)
{
	if (effects.size() == SuccEffect::maxSimul)
		return;

	static random_engine g;
	pos = Vector2Add(pos, UniformVector2(4.0f, g));
	Vector2 vel = UniformVector2(8.0f, g);
	uniform_float_t startRadiusDistr(0.5f, 1.0f);
	uniform_float_t growthRateDistr(32.0f, 56.0f);
	effects.emplace_front(pos, vel, GetTime(), startRadiusDistr(g), growthRateDistr(g), color);
}
void UpdateSuccEffects()
{
	float now = GetTime();
	while (!effects.empty() && now - effects.back().birthDate >= SuccEffect::lifetime)
	{
		effects.pop_back();
	}

	float dt = GetFrameTime();
	for (SuccEffect& succ : effects)
	{
		float age = now - succ.birthDate;
		float ageMu = age / SuccEffect::lifetime;
		succ.position.x += succ.velocity.x * dt;
		succ.position.y += succ.velocity.y * dt;
		succ.radius += succ.growthRate * dt;
		succ.color.a = (unsigned char)Lerp(SuccEffect::startAlpha, 0, ageMu);
	}
}
void DrawSuccEffects()
{
	for (SuccEffect& succ : effects)
	{
		DrawRing(succ.position, succ.radius - 3, succ.radius, 0, 360, 20, succ.color);
	}
}

std::vector<ResourceNode*> g_inhaling;
Vector2 g_collectionPos = Vector2Zero();
float g_collectionRadius = 8.0f;
float g_lastFixedUpdate = -INFINITY;
// Every g_fixedTimeStep
void TryFixedUpdate()
{
	// Guard
	{
		float now = GetTime();
		if (now - g_lastFixedUpdate < g_fixedTimeStep)
			return;

		g_lastFixedUpdate = now;
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		ForEachVisibleNode([](ResourceNode& node)
			{
				if (!node.beingInhaled && CheckCollisionPointCircle(node.pos, g_collectionPos, g_collectionRadius))
				{
					node.beingInhaled = true;
					g_inhaling.push_back(&node);
				}
			});
	}

	for (ResourceNode* node : g_inhaling)
	{
		if (Vector2Distance(node->pos, g_collectionPos) < 3.0f)
		{
			node->beingInhaled = false;
			node->visible = false;
			++g_itemCounts[(int)node->type];
			SpawnSuccEffect(node->pos, node->GetColor());
		}
	}

	std::erase_if(g_inhaling, [](const ResourceNode* node) { return !node->beingInhaled; });

	ForEachVisiblePatch(ResourcePatch::UpdateEmpty);
}

// Every frame
void Update()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		g_playerCamera.offset = Vector2Add(g_playerCamera.offset, GetMouseDelta());
	}
	for (ResourceNode* node : g_inhaling)
	{
		if (node->visible)
		{
			float distance = Vector2Distance(node->pos, g_collectionPos);
			float speed = (distance + std::max((g_collectionRadius * 5) - distance, 15.0f));
			node->pos = Vector2MoveTowards(node->pos, g_collectionPos, speed * GetFrameTime());
		}
	}

	UpdateScreenRect();
	UpdateSuccEffects();
}

int g_windowWidth = 1280;
int g_windowHeight = 720;
void DrawFrame()
{
	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode2D(g_playerCamera);
	ForEachVisibleNode(ResourceNode::Draw);
	DrawSuccEffects();
	BeginBlendMode(BLEND_ADDITIVE);
	DrawRing(g_collectionPos, g_collectionRadius, g_collectionRadius + 2, 0, 360, 36, GRAY);
	EndBlendMode();
	EndMode2D();

	for (int i = 0; i < g_resourceTypes; ++i)
	{
		DrawText(TextFormat("%s: %i", ResourceTypeNameFromIndex(i), g_itemCounts[i]), 2, 28 + 20 * i, 20, g_resourceColors[i]);
	}

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
		g_collectionPos = GetScreenToWorld2D(GetMousePosition(), g_playerCamera);

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
