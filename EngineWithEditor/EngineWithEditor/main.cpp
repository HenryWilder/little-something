#include <fstream>
#include "containers.h"
#include "Debug.h"
#include "EditorUI.h"

using namespace EditorUI;

int main()
{
	Rect r;
	r.width = 2;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(1280, 720, "Henry's Editor");
	SetTargetFPS(60);

	{
		Image defaultImg = GenImageColor(1, 1, WHITE);
		UI::uiTexture = LoadTextureFromImage(defaultImg);
		UnloadImage(defaultImg);
	}

	UI::previewShader = LoadShader(0, "preview.frag");
	UI::gripShader = LoadShader(0, "grip.frag");
	UI::gripShaderSizeLoc = GetShaderLocation(UI::gripShader, "size");

	hw::vector<UI::Pane*> panes;
	panes.reserve(8);
	panes.push_back(hw::New<UI::Pane>("Test1", false));
	panes.push_back(hw::New<UI::Pane>("Test2", true));
	panes.back()->Move({50,0});

	UI::Pane* focusedPane = nullptr;
	while (!WindowShouldClose())
	{
		

		// Draw
		BeginDrawing();
		{
			ClearBackground(UI::Theme::color_main);

			
		}
		EndDrawing();
	}

	UnloadShader(UI::gripShader);
	UnloadShader(UI::previewShader);
	UnloadTexture(UI::uiTexture);

	CloseWindow();

	return 0;
}
