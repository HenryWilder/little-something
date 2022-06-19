#include "containers.h"
#include <fstream>
#include <raylib.h>

struct GUIElement
{

};

// A window that can be moved around on the main window
struct Pane
{
	hw::vector<GUIElement> elements;
	Pane()
	{
		elements.reserve(256);
	}
	~Pane()
	{

	}
	void Update()
	{

	}
	void Draw()
	{

	}
};

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(1280, 720, "Henry's Editor");
	SetTargetFPS(60);

	hw::vector<Pane> test;
	test.reserve(32);
	test.push_back(Pane());

	while (!WindowShouldClose())
	{


		BeginDrawing();
		{
			ClearBackground(BLACK);


		}
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
