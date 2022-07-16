#include <fstream>
#include "containers.h"
#include "Debug.h"
#include "EditorUI.h"

using namespace EditorUI;
using namespace Engine;

int main()
{
	Rect r;
	r.width = 2;

	rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE | rl::FLAG_VSYNC_HINT | rl::FLAG_MSAA_4X_HINT);
	rl::InitWindow(1280, 720, "Henry's Editor");
	rl::SetTargetFPS(60);

	{
		rl::Image defaultImg = rl::GenImageColor(1, 1, rl::WHITE);
		uiTexture = rl::LoadTextureFromImage(defaultImg);
		rl::UnloadImage(defaultImg);
	}

	previewShader = rl::LoadShader(0, "preview.frag");
	gripShader = rl::LoadShader(0, "grip.frag");
	gripShaderSizeLoc = rl::GetShaderLocation(gripShader, "size");

	hw::vector<Pane*> panes;
	panes.reserve(8);
	panes.push_back(hw::New<Pane>("Test1", false));
	panes.push_back(hw::New<Pane>("Test2", true));
	panes.back()->Move({50,0});

	Pane* focusedPane = nullptr;
	while (!rl::WindowShouldClose())
	{
		

		// Draw
		rl::BeginDrawing();
		{
			rl::ClearBackground(Theme::color_main);

			
		}
		rl::EndDrawing();
	}

	rl::UnloadShader(gripShader);
	rl::UnloadShader(previewShader);
	rl::UnloadTexture(uiTexture);

	rl::CloseWindow();

	return 0;
}
