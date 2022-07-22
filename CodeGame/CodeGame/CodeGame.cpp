#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <variant>
#include <raylib.h>
#include <raymath.h>

using byte = unsigned char;

Texture2D objectTextures; // Only things that can be nouns
Texture2D wordTextures;   // Can be any word

struct Vector2Int
{
	int16_t x, y;

	Vector2Int() = default;
	constexpr Vector2Int(int x, int y) : x((int16_t)x), y((int16_t)y) {}

	bool operator==(const Vector2Int& other) const
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(const Vector2Int& other) const
	{
		return x != other.x || y != other.y;
	}
};
namespace std
{
	template<> struct hash<Vector2Int>
	{
		size_t operator()(const Vector2Int& x) const
		{
			int32_t value = (x.x << sizeof(decltype(x.y))) | x.y;
			return hash<int32_t>()(value);
		}
	};
}

// Returns true if blocker is between start and end
// (start...end]
bool CheckHit(Vector2Int start, Vector2Int end, Vector2Int blocker)
{
	return
		start.x < blocker.x && blocker.x <= end.x ||
		end.x <= blocker.x && blocker.x < start.x ||
		start.y < blocker.y && blocker.y <= end.y ||
		end.y <= blocker.y && blocker.y < start.y;
}

enum class Word : byte
{
	// Flags
	NOUN_BIT      =  32u,
	VERB_BIT      =  64u,
	ADJECTIVE_BIT = 128u,

	// Nouns
	BABA = NOUN_BIT,
	KEY,
	WALL,
	ROCK,
	TEXT,
	LOVE,

	// Verbs
	IS = VERB_BIT,
	AND,
	HAS,

	// Adjectives
	YOU = ADJECTIVE_BIT,
	MOVE,
	WIN,
	PUSH,
	PULL,
	STOP,
	DEFEAT,
	TELE,
	MELT,
};

// Filters of Word

enum class Noun : byte
{
	BABA = (byte)Word::NOUN_BIT,
	KEY,
	WALL,
	ROCK,
	TEXT,
	LOVE,
};
const char* ToString(Noun value)
{
	constexpr const char* names[] =
	{
		"BABA",
		"KEY",
		"WALL",
		"ROCK",
		"TEXT",
		"LOVE",
	};
	return names[(int)value - (int)Word::NOUN_BIT];
}

enum class Verb : byte
{
	IS = (byte)Word::VERB_BIT,
	AND,
	HAS,
};
const char* ToString(Verb value)
{
	constexpr const char* names[] =
	{
		"IS",
		"AND",
		"HAS",
	};
	return names[(int)value - (int)Word::VERB_BIT];
}

enum class Adjective : byte
{
	YOU = (byte)Word::ADJECTIVE_BIT,
	MOVE,
	WIN,
	PUSH,
	PULL,
	STOP,
	DEFEAT,
	TELE,
	MELT,
};
const char* ToString(Adjective value)
{
	constexpr const char* names[] =
	{
		"YOU",
		"MOVE",
		"WIN",
		"PUSH",
		"PULL",
		"STOP",
		"DEFEAT",
		"TELE",
		"MELT",
	};
	return names[(int)value - (int)Word::ADJECTIVE_BIT];
}

using NounOrAdjective_t = std::variant<Noun, Adjective>;
struct Rule
{
	Noun target;
	Verb action; // Rules are single statements and can't have 'and'
	NounOrAdjective_t value;

	Noun GetConversionResult() const
	{
		_ASSERTE(std::holds_alternative<Noun>(value));
		return std::get<Noun>(value);
	}
	Adjective GetProperty() const
	{
		_ASSERTE(IsProperty());
		return std::get<Adjective>(value);
	}

	bool IsValid() const
	{
		return action == Verb::IS || (std::holds_alternative<Noun>(value) && action == Verb::HAS);
	}

	const std::string ToString() const
	{
		return
			std::string(::ToString(target)) + ' ' +
			std::string(::ToString(action)) + ' ' +
			std::string((std::holds_alternative<Noun>(value)
				? ::ToString(std::get<Noun>(value))
				: ::ToString(std::get<Adjective>(value))));
	}

	// Categorizations

	// Rule converts target to value type; does not include mortum cast
	bool IsConversion() const
	{
		return action == Verb::IS && std::holds_alternative<Noun>(value);
	}
	// Rule applies value as property to target
	bool IsProperty() const
	{
		return action == Verb::IS && std::holds_alternative<Adjective>(value);
	}
	// Rule converts target to type when target is destroyed
	bool IsMortumCast() const
	{
		_ASSERTE(std::holds_alternative<Noun>(value));
		return action == Verb::HAS;
	}

	// "One-Off" rules apply once per instance
	bool IsOneOff() const
	{
		return IsConversion() || IsMortumCast();
	}
	// "Active" rules are applied every step
	bool IsActive() const
	{
		if (!IsProperty()) return false;
		_ASSERTE(std::holds_alternative<Adjective>(value));
		Adjective x = std::get<Adjective>(value);
		return IsProperty() &&
			x == Adjective::MOVE ||
			x == Adjective::YOU;
	}
	// "Passive" rules are applied only through interactions
	bool IsPassive() const
	{
		return !IsActive() && !IsOneOff();
	}
};
using Ruleset_t = std::list<Rule>;
Ruleset_t ruleset; // In-play
std::unordered_map<Noun, Ruleset_t> typeRules;
Ruleset_t* RulesForType(Noun type)
{
	auto it = typeRules.find(type);
	return (it != typeRules.end()) ? &it->second : nullptr;
}

constexpr int gridSize = 32;

// Player movement this Step
Vector2Int input;

class Object;
using Objects_t = std::vector<Object*>;
Objects_t world;

using Grid_t = std::unordered_map<Vector2Int, Objects_t>;
Grid_t grid;

Objects_t* ObjectAtPosition(Vector2Int pos)
{
	auto it = grid.find(pos);
	return (it != grid.end()) ? &it->second : nullptr;
};

using Types_t = std::unordered_map<Noun, Objects_t>;
Types_t types;

class Object
{
	Noun meta;
	Word text; // Only available if meta == text
	byte rotation : 2; // 90-degree clockwise increments
	Vector2Int position;
	Vector2Int velocity; // Delta position this Step

	Object(Noun meta, Word text, int rotation, int x, int y) :
		meta(meta), text(text), rotation(rotation & 3), position{ (short)x,(short)y }, velocity{ 0,0 } {}

public:
	static Object* CreateObject(Noun meta, int rotation, int x, int y)
	{
		world.push_back(new Object(meta, (Word)0, rotation, x, y));
		return world.back();
	}
	static Object* CreateText(Word meta, int rotation, int x, int y)
	{
		world.push_back(new Object(Noun::TEXT, meta, rotation, x, y ));
		return world.back();
	}
	static void DestroyObject(Object*& what)
	{
		auto it = std::find(world.begin(), world.end(), what);
		_ASSERT_EXPR(it != world.end(), L"Cannot remove missing element");
		world.erase(it);
		delete what;
		what = nullptr;
	}
	static void Cleanup()
	{
		for (Object* obj : world)
		{
			delete obj;
		}
		world.clear();
	}

	Vector2Int Position() const { return position; }
	Noun What() const { return meta; }
	Ruleset_t* Rules() const { return RulesForType(meta); }
	bool IsText()      const { return meta == Noun::TEXT; }
	bool IsNoun()      const { return (byte)text & (byte)Word::NOUN_BIT; } // Text noun, not object noun
	bool IsVerb()      const { return (byte)text & (byte)Word::VERB_BIT; }
	bool IsAdjective() const { return (byte)text & (byte)Word::ADJECTIVE_BIT; }
	Word Text()             const { _ASSERTE(IsText()); return text; }
	Noun AsNoun()           const { _ASSERTE(IsNoun()); return (Noun)text; }
	Verb AsVerb()           const { _ASSERTE(IsVerb()); return (Verb)text; }
	Adjective AsAdjective() const { _ASSERTE(IsAdjective()); return (Adjective)text;}

	void Draw(Vector2 offsetFromSpace) const
	{
		// @Todo: This is placeholder code. Please replace with textures.
		Color color;
		if (meta == Noun::TEXT) // Name, not object
		{
			switch (text)
			{
			case Word::BABA:   color = RED; break;
			case Word::KEY:    color = GOLD; break;
			case Word::WALL:   color = GRAY; break;
			case Word::ROCK:   color = ORANGE; break;
			case Word::TEXT:   color = VIOLET; break;
			case Word::LOVE:   color = PINK; break;

			case Word::IS:     color = WHITE; break;
			case Word::AND:    color = WHITE; break;
			case Word::HAS:    color = WHITE; break;

			case Word::YOU:    color = RED; break;
			case Word::WIN:    color = MAGENTA; break;
			case Word::PUSH:   color = ORANGE; break;
			case Word::PULL:   color = ORANGE; break;
			case Word::STOP:   color = DARKGREEN; break;
			case Word::DEFEAT: color = RED; break;
			case Word::TELE:   color = SKYBLUE; break;
			case Word::MELT:   color = SKYBLUE; break;

			default:           color = MAGENTA; break;
			}
		}
		else // Object, not name
		{
			switch (meta)
			{
			case Noun::BABA: color = WHITE; break;
			case Noun::WALL: color = GRAY; break;
			case Noun::ROCK: color = ORANGE; break;
			case Noun::LOVE: color = PINK; break;

			default:         color = MAGENTA; break;
			}
		}

		DrawRectangle(position.x * gridSize, position.y * gridSize, gridSize, gridSize, color);
	}

	// Rules
	// Note: "AND" is a linker, not a rule

	void Is(Noun what)
	{
		meta = what;
	}

	// Called after all rules are applied
	void Update()
	{
		velocity.x = 0;
		velocity.y = 0;

		Ruleset_t* myRules = Rules();

		if (!myRules) return;

		bool destroyedThisStep = false;

		for (const Rule& rule : *myRules)
		{
			if (rule.action != Verb::IS || std::holds_alternative<Noun>(rule.value)) continue;
			Adjective action = std::get<Adjective>(rule.value);

			// "Active" actions which the object performs for itself
			switch (action)
			{
			case Adjective::YOU:
				velocity.x += input.x;
				velocity.y += input.y;
				break;
			case Adjective::MOVE:
				// @Todo: Move in direction of rotation
				break;
			}
		}

		// "Passive" actions which are sampled from adjacent tiles
		constexpr Vector2Int offsets[] =
		{
			{  0, -1 },
			{ +1,  0 },
			{  0, +1 },
			{ -1,  0 },
		};
		for (Vector2Int offset : offsets)
		{
			Vector2Int space =
			{
				position.x + offset.x,
				position.y + offset.y
			};
			Objects_t* objects = ObjectAtPosition(space);
			if (!objects) continue;
			for (Object* object : *objects)
			{
				Ruleset_t* spaceRules = object->Rules();
				for (const Rule& rule : *spaceRules)
				{
					if (!rule.IsProperty()) continue;
					// "Active" actions which the object performs for itself
					switch (rule.GetProperty())
					{
					case Adjective::PUSH:
						// @Todo
						break;
					case Adjective::PULL:
						// @Todo
						break;

					case Adjective::WIN:
						// @Todo: Win condition
						break;
					case Adjective::STOP:
						// Blocking?
						Vector2Int newPos =
						{
							position.x + velocity.x,
							position.y + velocity.y,
						};
						if (CheckHit(position, newPos, space))
						{
							velocity.x = 0;
							velocity.y = 0;
						}
						break;
					case Adjective::DEFEAT:
						// @Todo: Lose condition
						break;
					case Adjective::TELE:
						break;
					case Adjective::MELT:
						break;
					}
				}
			}
		}

		position.x += velocity.x;
		position.y += velocity.y;

		// Has is applied after destruction
		if (destroyedThisStep)
		{
			for (const Rule& rule : *myRules)
			{
				if (rule.IsMortumCast()) continue;
				// @Todo: Create new object
			}
		}
	}
};

void Step()
{
	// Map objects in world
	grid.clear();
	for (Object* obj : world)
	{
		Vector2Int pos = obj->Position();
		grid[pos].push_back(obj);
	}

	// Create rules
	ruleset.clear();
	// Top-leftmost gets highest precedence 
	for (Object* obj : world)
	{
		// Cannot evaluate starting from verbs nor adjectives
		if (!obj->IsNoun())
			continue;

		auto [x, y] = obj->Position();

		do
		{
			Objects_t* rightSpace1 = ObjectAtPosition({ x + 1, y });
			if (!rightSpace1) break;

			Objects_t* rightSpace2 = ObjectAtPosition({ x + 2, y });
			if (!rightSpace2) break;

			Object* right1 = nullptr;
			for (Object* what : *rightSpace1)
			{
				if (what->IsVerb())
				{
					right1 = what;
					break;
				}
			}
			if (!right1) break;

			Object* right2 = nullptr;
			for (Object* what : *rightSpace2)
			{
				if (what->IsNoun() || what->IsAdjective())
				{
					right2 = what;
					break;
				}
			}
			if (!right2) break;

			// @Todo: "AND" linkage
			// @Todo: Validation

			Rule rule;
			rule.target = obj->AsNoun();
			rule.action = right1->AsVerb();
			rule.value = right2->IsNoun() ? NounOrAdjective_t{ right2->AsNoun() } : NounOrAdjective_t{ right2->AsAdjective() };
			ruleset.push_back(rule);
		}
		while (false);

		// @Todo: Vertical rules
	}

	// Map objects by type
	types.clear();
	for (Object* obj : world)
	{
		Noun type = obj->What();
		types[type].push_back(obj);
	}

	// Apply type conversion rules
	for (const Rule& rule : ruleset)
	{
		auto it = types.find(rule.target);
		if (it == types.end()) continue;

		const Objects_t& objects = it->second;
		for (Object* obj : objects)
		{
			if (rule.action == Verb::IS && std::holds_alternative<Noun>(rule.value))
				obj->Is(std::get<Noun>(rule.value));
		}
	}
	// @Todo: The types map will be invalid ?????

	// Map rules by type
	typeRules.clear();
	for (Rule rule : ruleset)
	{
		typeRules[rule.target].push_back(rule);
	}

	// Update objects according to rules
	for (Object* obj : world)
	{
		obj->Update();
	}
}

int main()
{
	InitWindow(1280, 720, "Code Game");
	SetTargetFPS(60);

	Object::CreateText(Word::BABA, 0, 0,0);
	Object::CreateText(Word::IS,   0, 1,0);
	Object::CreateText(Word::YOU,  0, 2,0);

	Object::CreateText(Word::WALL, 0, 0,1);
	Object::CreateText(Word::IS,   0, 1,1);
	Object::CreateText(Word::STOP, 0, 2,1);

	Object::CreateObject(Noun::BABA, 0, 5,5);
	Object::CreateObject(Noun::WALL, 0, 9,5);

	Step(); // Initialize

	while (!WindowShouldClose())
	{
		{
			bool up    = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
			bool down  = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
			bool left  = IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
			bool right = IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);
			bool space = IsKeyPressed(KEY_SPACE);

			if (up || down || left || right || space)
			{
				input.x = (short)right - (short)left;
				input.y = (short)down  - (short)up;

				Step();
			}
		}

		BeginDrawing();

		ClearBackground(BLACK);

		for (Object* obj : world)
		{
			obj->Draw(Vector2Zero());
		}

		int y = 0;
		for (const Rule& rule : ruleset)
		{
			std::string ruleName = rule.ToString();
			DrawText(ruleName.c_str(), 0, y, 20, MAGENTA);
			y += 20;
		}

		EndDrawing();
	}

	Object::Cleanup();

	CloseWindow();
	return 0;
}