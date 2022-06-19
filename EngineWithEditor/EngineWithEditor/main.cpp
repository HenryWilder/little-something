#include "containers.h"
#include <fstream>
#include <raylib.h>

void DrawRectangleOutlined(Rectangle rec, Color fillColor, Color linesColor)
{
	DrawRectangleRec(rec, fillColor);
	DrawRectangleLinesEx(rec, 1.0f, linesColor);
}

namespace UI
{
	namespace Theme
	{
		Color color_foreground = { 255,255,255,255 };
		Color color_highlight = { 0,127,255,255 };
		Color color_accent = { 80,80,80,255 };
		Color color_main = { 35,35,35,255 };
		Color color_body = { 20,20,20,255 };
		int fontSize = 10;
	}

	Texture uiTexture;
	int gripShaderSizeLoc;
	Shader gripShader;
	Shader previewShader;

	inline void BeginPreviewMode() { BeginShaderMode(previewShader); }
	inline void EndPreviewMode() { EndShaderMode(); }

	// Size of whichever axis of the grip is fixed
	constexpr float gripFixedSize = 18;

	// Mode of the cursor for use in this program (layered on top of Raylibs)
	enum class CursorShapeMode
	{
		none = 0,
		resizeRight,
		resizeDown,
		resizeDiagonal,
		resizeAll,
	};
	CursorShapeMode cursorShapeMode = CursorShapeMode::none;

	void DrawGrip(Rectangle rect, Color color)
	{
		rect.x += 5;
		rect.y += 5;
		rect.width -= 10;
		rect.height -= 10;
		{
			float value[2];
			value[0] = rect.width;
			value[1] = rect.height;
			SetShaderValue(gripShader, gripShaderSizeLoc, value, SHADER_UNIFORM_VEC2);
		}
		BeginShaderMode(gripShader);
		DrawTexturePro(uiTexture, { 0,0,1,1 }, rect, { 0,0 }, 0.0f, color);
		EndShaderMode();
	}

	// A window that can be moved around on the main window
	struct Pane
	{
		// Outset width of edges
		static constexpr float edgeSize = 5.0f;
		float minSize = gripFixedSize;

		const char* name;
		Rectangle rect;
		Rectangle gripRect;
		bool gripIsVertical;
		bool focused = false;
		bool beingDragged = false;
		bool resizingX = false;
		bool resizingY = false;

		enum class HoverRegion
		{
			notHovering = 0,
			hovering = 1, // Hovering the pane without any distinction
			edge_right,
			edge_bottom,
			corner,
			handle,
		};

		Pane(const char* name, bool gripIsVertical)
		{
			this->name = name;
			rect = { 0,0,50,50 };
			gripRect = rect;
			this->gripIsVertical = gripIsVertical;
			if (gripIsVertical)
				gripRect.width = gripFixedSize;
			else
				gripRect.height = gripFixedSize;
		}
		~Pane()
		{
			// Nothing yet
		}

		void Move(Vector2 delta)
		{
			rect.x += delta.x;
			rect.y += delta.y;
			gripRect.x += delta.x;
			gripRect.y += delta.y;
		}
		void Resize(Vector2 delta)
		{
			rect.width += delta.x;
			if (rect.width < minSize) rect.width = minSize;
			rect.height += delta.y;
			if (rect.height < minSize) rect.height = minSize;
			if (gripIsVertical)
			{
				gripRect.height += delta.y;
				if (gripRect.height < minSize) gripRect.height = minSize;
			}
			else
			{
				gripRect.width += delta.x;
				if (gripRect.width < minSize) gripRect.width = minSize;
			}
		}
		void Snap()
		{

		}

		void Update()
		{
			Vector2 cursor = GetMousePosition();
			HoverRegion hoverState;

			// Determine what part is being hovered
			{
				Rectangle expandedRect = rect;
				expandedRect.width += edgeSize;
				expandedRect.height += edgeSize;

				// Static 'hover' resulting from interaction state
				if (beingDragged)
					hoverState = HoverRegion::handle;
				else if (resizingX && resizingY)
					hoverState = HoverRegion::corner;
				else if (resizingX)
					hoverState = HoverRegion::edge_right;
				else if (resizingY)
					hoverState = HoverRegion::edge_bottom;

				// Check if hovering the main rectangle or the grip
				else if (CheckCollisionPointRec(cursor, rect))
				{
					hoverState = HoverRegion::hovering;
					if (CheckCollisionPointRec(cursor, gripRect))
						hoverState = HoverRegion::handle;
				}

				// Check if hovering an edge
				else if (CheckCollisionPointRec(cursor, expandedRect))
				{
					if (cursor.x >= rect.x + rect.width - edgeSize && cursor.y >= rect.y + rect.height)
						hoverState = HoverRegion::corner;
					else if (cursor.x >= rect.x + rect.width)
						hoverState = HoverRegion::edge_right;
					else if (cursor.y >= rect.y + rect.height)
						hoverState = HoverRegion::edge_bottom;
					else
						hoverState = HoverRegion::notHovering;
				}

				// Hovering nothing
				else hoverState = HoverRegion::notHovering;
			}

			// Determine if interaction is happening
			{
				// Set states on press
				if (hoverState != HoverRegion::notHovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
				{
					switch (hoverState)
					{
					case UI::Pane::HoverRegion::edge_right:
						resizingX = true;
						break;
					case UI::Pane::HoverRegion::edge_bottom:
						resizingY = true;
						break;
					case UI::Pane::HoverRegion::corner:
						resizingX = true;
						resizingY = true;
						break;
					case UI::Pane::HoverRegion::handle:
						beingDragged = true;
						focused = true;
						break;
					case UI::Pane::HoverRegion::hovering:
						focused = true;
						break;
					default:
						break;
					}
				}

				// Reset states on release
				if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
				{
					resizingX = false;
					resizingY = false;
					beingDragged = false;
				}

				// Update mouse cursor
				if (cursorShapeMode == CursorShapeMode::none &&
					hoverState != HoverRegion::notHovering)
				{
					switch (hoverState)
					{
					case UI::Pane::HoverRegion::edge_right:
						cursorShapeMode = CursorShapeMode::resizeRight;
						break;
					case UI::Pane::HoverRegion::edge_bottom:
						cursorShapeMode = CursorShapeMode::resizeDown;
						break;
					case UI::Pane::HoverRegion::corner:
						cursorShapeMode = CursorShapeMode::resizeDiagonal;
						break;
					case UI::Pane::HoverRegion::handle:
						cursorShapeMode = CursorShapeMode::resizeAll;
						break;
					default:
						break;
					}
				}
			}

			// Update things that occur due to interaction
			{
				Vector2 mouseDelta = GetMouseDelta();

				{
					Vector2 resizeDelta = mouseDelta;
					if (!resizingX) resizeDelta.x = 0;
					if (!resizingY) resizeDelta.y = 0;
					Resize(resizeDelta);
				}

				if (beingDragged) Move(mouseDelta);
			}
		}
		void Draw() const
		{
			if (beingDragged) BeginPreviewMode();
			DrawRectangleOutlined(rect, Theme::color_main, Theme::color_accent);
			Rectangle gripDrawRect = gripRect;
			if (gripIsVertical)
			{
				float nameHeight = (float)(Theme::fontSize + 4);
				gripDrawRect.y += nameHeight;
				gripDrawRect.height -= nameHeight;
				DrawGrip(gripDrawRect, Theme::color_accent);
			}
			else
			{
				if (focused) DrawRectangleRec(gripRect, Theme::color_highlight);
				float nameWidth = (float)(MeasureText(name, Theme::fontSize) + 4);
				gripDrawRect.x += nameWidth;
				gripDrawRect.width -= nameWidth;
				DrawGrip(gripDrawRect, focused ? Theme::color_foreground : Theme::color_accent);
			}
			DrawText(name, rect.x + 4, rect.y + 4, Theme::fontSize, Theme::color_foreground);
			if (beingDragged) EndPreviewMode();
		}
	};
}

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(1280, 720, "Henry's Editor");
	SetTargetFPS(60);

	Image defaultImg = GenImageColor(1, 1, WHITE);
	UI::uiTexture = LoadTextureFromImage(defaultImg);
	UnloadImage(defaultImg);

	UI::previewShader = LoadShader(0, "preview.frag");

	UI::gripShader = LoadShader(0, "grip.frag");
	UI::gripShaderSizeLoc = GetShaderLocation(UI::gripShader, "size");

	hw::Alloc<UI::Pane*>();
	hw::vector<UI::Pane*> panes;
	panes.reserve(32);
	panes.push_back(hw::New<UI::Pane>("Test1", false));
	panes.push_back(hw::New<UI::Pane>("Test2", true));
	panes.back()->Move({50,0});

	UI::Pane* lastFrameFocusedPane = nullptr;

	while (!WindowShouldClose())
	{
		UI::Pane* focusedPane = nullptr;
		for (UI::Pane* pane : panes)
		{
			pane->Update();
			if (pane->focused && pane != lastFrameFocusedPane)
				focusedPane = pane;
		}
		if (!!focusedPane && lastFrameFocusedPane != focusedPane)
		{
			auto it = std::find(panes.begin(), panes.end(), focusedPane);
			panes.erase(it);
			panes.push_back(focusedPane);
			if (!!lastFrameFocusedPane)
				lastFrameFocusedPane->focused = false;
			focusedPane->focused = true;
		}
		lastFrameFocusedPane = focusedPane;

		switch (UI::cursorShapeMode)
		{
		default:
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			break;
		case UI::CursorShapeMode::resizeRight:
			SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
			break;
		case UI::CursorShapeMode::resizeDown:
			SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
			break;
		case UI::CursorShapeMode::resizeDiagonal:
			SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE);
			break;
		case UI::CursorShapeMode::resizeAll:
			SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL);
			break;
		}
		UI::cursorShapeMode = UI::CursorShapeMode::none;

		BeginDrawing();
		{
			ClearBackground(UI::Theme::color_main);

			for (const UI::Pane* pane : panes)
			{
				pane->Draw();
			}
		}
		EndDrawing();
	}

	UnloadShader(UI::gripShader);
	UnloadShader(UI::previewShader);
	UnloadTexture(UI::uiTexture);

	CloseWindow();

	return 0;
}
