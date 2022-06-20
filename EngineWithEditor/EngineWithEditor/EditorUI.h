#pragma once
#include <raylib.h>
#include "Types.h"

void DrawRectangleOutlined(Rectangle rec, Color fillColor, Color linesColor)
{
	DrawRectangleRec(rec, fillColor);
	DrawRectangleLinesEx(rec, 1.0f, linesColor);
}

namespace EditorUI
{
	Texture uiTexture;

	namespace Theme
	{
		Color color_foreground = { 255,255,255,255 };
		Color color_highlight = { 0,127,255,255 };
		Color color_accent = { 80,80,80,255 };
		Color color_main = { 35,35,35,255 };
		Color color_body = { 20,20,20,255 };
		int fontSize = 10;
	}

	Shader gripShader;
	int gripShaderSizeLoc;

	Shader previewShader;
	inline void BeginPreviewMode() noexcept(noexcept(BeginShaderMode)) { BeginShaderMode(previewShader); }
	inline void EndPreviewMode() noexcept(noexcept(EndShaderMode)) { EndShaderMode(); }

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

	// A helper for checking where a potential snap is to be connected
	struct SnapRect
	{
		enum class Region { floating = 0, top, right, bottom, left, center, };

		static constexpr float snapSize = 7;
		static constexpr float centerInset = 50;

		Rectangle regions[5];

		SnapRect() = default;
		SnapRect(Rectangle rect) noexcept;

		inline Region IndexToRegion(size_t index) noexcept { return (Region)(index + 1); }
		inline int IndexFromRegion(Region region) noexcept { return (size_t)region - 1; }
		_Ret_opt_ const Rectangle* RectFromRegion(Region region) noexcept
		{
			if (region == Region::floating)
				return nullptr;
			_ASSERT_EXPR(IndexFromRegion(region) < 5, L"Region should not be modified externally");
			return &regions[IndexFromRegion(region)];
		}
		Region CheckCollision(Vector2 point) noexcept
		{
			for (const Rectangle& rect : regions)
			{
				if (CheckCollisionPointRec(point, rect))
					return IndexToRegion((ptrdiff_t)regions - (ptrdiff_t)&rect);
			}
			return Region::floating;
		}
	};

	enum class PaneInteractFlags
	{
		focused = 0b0001,
		beingDragged = 0b0010,
		resizingX = 0b0100,
		resizingY = 0b1000,
	};
	PaneInteractFlags operator|(PaneInteractFlags a, PaneInteractFlags b)
	{
		return PaneInteractFlags((int)a | (int)b);
	}
	PaneInteractFlags operator&(PaneInteractFlags a, PaneInteractFlags b)
	{
		return PaneInteractFlags((int)a & (int)b);
	}
	bool operator!(PaneInteractFlags what)
	{
		return !(bool)what;
	}
	bool Contains(PaneInteractFlags set, PaneInteractFlags flag)
	{
		return !!((int)set & (int)flag);
	}

	// A window that can be moved around on the main window
	struct Pane
	{
		// Outset width of edges
		static constexpr float edgeSize = 5.0f;
		inline static float minSize = gripFixedSize;

		const char* name;
		Rectangle rect;
		Rectangle gripRect;
		bool gripIsVertical;

		enum class HoverRegion
		{
			notHovering = 0,
			hovering = 1, // Hovering the pane without any distinction
			edge_right,
			edge_bottom,
			corner,
			handle,
		};

		Pane(_In_z_ const char* name, bool gripIsVertical) noexcept
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
		~Pane() noexcept
		{
			// Nothing yet
		}

		void Move(Vector2 delta) noexcept
		{
			rect.x += delta.x;
			rect.y += delta.y;
			gripRect.x += delta.x;
			gripRect.y += delta.y;
		}
		void Resize(Vector2 delta) noexcept
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

		HoverRegion CheckHover()
		{
			Vector2 cursor = GetMousePosition();

			// Check if hovering the main rectangle or the grip
			if (CheckCollisionPointRec(cursor, rect))
			{
				if (CheckCollisionPointRec(cursor, gripRect))
					return HoverRegion::handle;
				return HoverRegion::hovering;

			}

			Rectangle expandedRect = rect;
			expandedRect.width += edgeSize;
			expandedRect.height += edgeSize;

			// Check if hovering an edge
			if (CheckCollisionPointRec(cursor, expandedRect))
			{
				if (cursor.x >= rect.x + rect.width - edgeSize && cursor.y >= rect.y + rect.height)
					return HoverRegion::corner;
				if (cursor.x >= rect.x + rect.width)
					return HoverRegion::edge_right;
				if (cursor.y >= rect.y + rect.height)
					return HoverRegion::edge_bottom;
			}

			// Hovering nothing
			return HoverRegion::notHovering;
		}

		PaneInteractFlags CheckInteraction(HoverRegion hoverState)
		{
			PaneInteractFlags flags = PaneInteractFlags(0);

			// Set states on press
			if (hoverState != HoverRegion::notHovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				switch (hoverState)
				{
				case HoverRegion::edge_right:  flags = PaneInteractFlags::resizingX;								   break;
				case HoverRegion::edge_bottom: flags = PaneInteractFlags::resizingY;								   break;
				case HoverRegion::corner:	   flags = PaneInteractFlags::resizingX | PaneInteractFlags::resizingY;    break;
				case HoverRegion::handle:      flags = PaneInteractFlags::focused | PaneInteractFlags::beingDragged; break;
				case HoverRegion::hovering:    flags = PaneInteractFlags::focused;									   break;
				default: break;
				}
			}

			// Reset states on release
			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
				flags = flags & PaneInteractFlags::focused;

			return flags;
		}

		void Draw() const
		{
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
				float nameWidth = (float)(MeasureText(name, Theme::fontSize) + 4);
				gripDrawRect.x += nameWidth;
				gripDrawRect.width -= nameWidth;
				DrawGrip(gripDrawRect, Theme::color_accent);
			}
			DrawText(name, (int)rect.x + 4, (int)rect.y + 4, Theme::fontSize, Theme::color_foreground);
		}

		void UpdateFocused(PaneInteractFlags flags)
		{
			// Update things that occur due to interaction
			{
				Vector2 mouseDelta = GetMouseDelta();

				{
					Vector2 resizeDelta = mouseDelta;
					if (!(flags & PaneInteractFlags::resizingX)) resizeDelta.x = 0;
					if (!(flags & PaneInteractFlags::resizingY)) resizeDelta.y = 0;
					Resize(resizeDelta);
				}

				if (!!(flags & PaneInteractFlags::beingDragged)) Move(mouseDelta);
			}
		}

		// Assume focused is always true
		void DrawFocused(PaneInteractFlags flags)
		{
			if (!!(flags & PaneInteractFlags::beingDragged)) BeginPreviewMode();
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
				if (!!(flags & PaneInteractFlags::focused)) DrawRectangleRec(gripRect, Theme::color_highlight);
				float nameWidth = (float)(MeasureText(name, Theme::fontSize) + 4);
				gripDrawRect.x += nameWidth;
				gripDrawRect.width -= nameWidth;
				DrawGrip(gripDrawRect, Theme::color_foreground);
			}
			DrawText(name, (int)rect.x + 4, (int)rect.y + 4, Theme::fontSize, Theme::color_foreground);
			if (!!(flags & PaneInteractFlags::beingDragged)) EndPreviewMode();
		}
	};
	void UpdateCursorShapeModeWithoutOverride(Pane::HoverRegion hoverState)
	{
		switch (hoverState)
		{
		case UI::Pane::HoverRegion::edge_right:
			UI::cursorShapeMode = UI::CursorShapeMode::resizeRight;
			break;
		case UI::Pane::HoverRegion::edge_bottom:
			UI::cursorShapeMode = UI::CursorShapeMode::resizeDown;
			break;
		case UI::Pane::HoverRegion::corner:
			UI::cursorShapeMode = UI::CursorShapeMode::resizeDiagonal;
			break;
		case UI::Pane::HoverRegion::handle:
			UI::cursorShapeMode = UI::CursorShapeMode::resizeAll;
			break;
		default: // Don't override
			break;
		}
	}

	SnapRect::SnapRect(Rectangle rect) noexcept :
		regions{
			{
				.x = rect.x,
				.y = rect.y,
				.width = rect.width,
				.height = snapSize
			},
			{
				.x = rect.x + rect.width - snapSize,
				.y = rect.y,
				.width = snapSize,
				.height = rect.height
			},
			{
				.x = rect.x,
				.y = rect.y + rect.height - snapSize,
				.width = rect.width,
				.height = snapSize
			},
			{
				.x = rect.x,
				.y = rect.y,
				.width = snapSize,
				.height = rect.height
			},
			{
				.x = rect.x + centerInset,
				.y = rect.y + centerInset,
				.width = rect.width - centerInset * 2,
				.height = rect.height - centerInset * 2
			}
	}
	{
		float minSizeForCenter = centerInset * 2 + Pane::minSize;
		if (rect.width < minSizeForCenter || rect.height < minSizeForCenter)
			regions[4] = { 0,0,0,0 };
	}
}
