#include "containers.h"
#include <fstream>
#include <raylib.h>

// A window that can be moved around on the main window
struct Pane
{
	Pane(int width, int height, const char* name)
	{
		InitWindow(width, height, name);

	}
	~Pane()
	{
		CloseWindow();
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

	vector<Pane> panes;

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
