#include "menu.h"
#include "p_loop.h"
#include "imgui/imgui_internal.h"
#include <functional>

// ty claude ai for the fire menu

static int selectedTab = 0; // global

void input()
{
	ImGuiIO& io = ImGui::GetIO();

	for (int i = 0; i < 5; i++)
		io.MouseDown[i] = false;

	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		io.MouseDown[0] = true;


	for (int key = 0; key < 512; key++)
		io.KeysDown[key] = (GetAsyncKeyState(key) & 0x8000);

	io.KeyCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
	io.KeyShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
	io.KeyAlt = (GetAsyncKeyState(VK_MENU) & 0x8000);

	for (int key = 0x30; key <= 0x5A; key++)
	{
		if (GetAsyncKeyState(key) & 1)
		{
			BYTE keyboardState[256];
			if (!GetKeyboardState(keyboardState))
				return;

			WCHAR unicodeChar[4];
			if (ToUnicode(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), keyboardState, unicodeChar, 4, 0) > 0)
				io.AddInputCharacter(unicodeChar[0]);
		}
	}
}


bool MinimalCheckBox(const char* label, bool* v)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	float boxSize = 10.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 textSize = ImGui::CalcTextSize(label);

	ImVec2 totalSize = ImVec2(boxSize + style.ItemInnerSpacing.x + textSize.x, ImMax(boxSize, textSize.y));
	ImVec2 boxMin = pos;
	ImVec2 boxMax = ImVec2(pos.x + boxSize, pos.y + boxSize);

	ImGui::InvisibleButton(label, totalSize);

	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsItemClicked();

	if (clicked)
		*v = !*v;

	ImDrawList* draw = ImGui::GetWindowDrawList();

	float rounding = 2.0f;

	// Background
	ImU32 bgColor = *v ? IM_COL32(45, 45, 45, 255) : IM_COL32(25, 25, 25, 255);
	draw->AddRectFilled(boxMin, boxMax, bgColor, rounding);

	// Border
	ImU32 borderColor = *v ? IM_COL32(120, 120, 120, 255) : IM_COL32(70, 70, 70, 255);
	if (hovered)
		borderColor = IM_COL32(140, 140, 140, 255);

	draw->AddRect(boxMin, boxMax, borderColor, rounding, 0, 1.0f);

	// Filled indicator when checked
	if (*v)
	{
		ImVec2 innerMin = ImVec2(boxMin.x + 2, boxMin.y + 2);
		ImVec2 innerMax = ImVec2(boxMax.x - 2, boxMax.y - 2);
		draw->AddRectFilled(innerMin, innerMax, IM_COL32(180, 180, 180, 255), rounding * 0.5f);
	}

	// Draw label
	ImVec2 labelPos = ImVec2(boxMax.x + style.ItemInnerSpacing.x, pos.y + (boxSize - textSize.y) * 0.5f);
	ImU32 textColor = *v ? IM_COL32(220, 220, 220, 255) : IM_COL32(160, 160, 160, 255);
	draw->AddText(labelPos, textColor, label);

	return clicked;
}

void render_child_section(const char* title, float width, float height, std::function<void()> content)
{
	ImDrawList* draw = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Draw section header background
	draw->AddRectFilled(
		pos,
		ImVec2(pos.x + width, pos.y + 20),
		IM_COL32(28, 28, 28, 255)
	);

	// Draw header bottom border
	draw->AddLine(
		ImVec2(pos.x, pos.y + 20),
		ImVec2(pos.x + width, pos.y + 20),
		IM_COL32(45, 45, 45, 255),
		1.0f
	);

	// Draw title text
	ImVec2 textSize = ImGui::CalcTextSize(title);
	draw->AddText(
		ImVec2(pos.x + 8, pos.y + (20 - textSize.y) * 0.5f),
		IM_COL32(180, 180, 180, 255),
		title
	);

	// Child window for content
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
	ImGui::BeginChild(title, ImVec2(width, height), true, ImGuiWindowFlags_NoScrollbar);
	{
		ImGui::SetCursorPos(ImVec2(8, 8));
		ImGui::BeginGroup();
		content();
		ImGui::EndGroup();
	}
	ImGui::EndChild();
}

void render_visuals()
{
	if (selectedTab != 0)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

	ImGui::SetCursorPos(ImVec2(15, 50));
	ImGui::BeginGroup();

	// Player ESP section (left)
	render_child_section("player esp", 290, 300, []() {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));
		MinimalCheckBox("box", &settings::box);
		MinimalCheckBox("skeleton", &settings::skeleton);
		MinimalCheckBox("cat", &settings::kitty);
		ImGui::PopStyleVar();
		});

	ImGui::SetCursorPos(ImVec2(315, 50));

	// Visuals section (right)
	render_child_section("player data", 290, 300, []() {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));
		MinimalCheckBox("name", &settings::name);
		MinimalCheckBox("health", &settings::health);
		MinimalCheckBox("weapon", &settings::weapon);
		MinimalCheckBox("distance", &settings::distance);
		ImGui::PopStyleVar();
		});

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(3);
}



void menu()
{
	ImGui::SetNextWindowSize(ImVec2(620, 385), ImGuiCond_FirstUseEver);
	ImGui::Begin("perseverance", 0, ImGuiWindowFlags_NoDecoration);
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImVec2 outerMin = window->OuterRectClipped.Min;
		ImVec2 outerMax = window->OuterRectClipped.Max;
		float barHeight = 28.0f;

		ImDrawList* draw = ImGui::GetForegroundDrawList();

		// Header bar
		draw->AddRectFilled(
			outerMin,
			ImVec2(outerMax.x, outerMin.y + barHeight),
			IM_COL32(20, 20, 20, 255)
		);

		// Bottom border for header
		draw->AddLine(
			ImVec2(outerMin.x, outerMin.y + barHeight),
			ImVec2(outerMax.x, outerMin.y + barHeight),
			IM_COL32(40, 40, 40, 255),
			1.5f
		);

		// Animated gradient title
		static float t = 0.0f;
		t += ImGui::GetIO().DeltaTime * 0.5f;
		float gradient = (sin(t) + 1.0f) * 0.5f;

		ImVec4 c1 = ImLerp(ImVec4(0.15f, 0.50f, 1.0f, 1.0f), ImVec4(1, 1, 1, 1), gradient);
		ImVec4 c2 = ImLerp(ImVec4(1, 1, 1, 1), ImVec4(0.15f, 0.50f, 1.0f, 1.0f), gradient);

		const char* title = "perseverance";
		ImFont* font = ImGui::GetFont();
		ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, title);

		float padding = 18.f;
		float textX = outerMax.x - textSize.x - padding;
		float textY = outerMin.y + (barHeight * 0.5f) - (textSize.y * 0.5f);

		draw->PushClipRect(ImVec2(textX, textY), ImVec2(textX + textSize.x * 0.5f, textY + textSize.y), true);
		draw->AddText(font, font->FontSize, ImVec2(textX, textY),
			ImGui::ColorConvertFloat4ToU32(c1), title);
		draw->PopClipRect();

		draw->PushClipRect(ImVec2(textX + textSize.x * 0.5f, textY), ImVec2(textX + textSize.x, textY + textSize.y), true);
		draw->AddText(font, font->FontSize, ImVec2(textX, textY),
			ImGui::ColorConvertFloat4ToU32(c2), title);
		draw->PopClipRect();

		ImGui::PushFont(ImGui::GetFont());

		float paddingLeft = 18.f;
		float tabSpacing = 65.f;

		ImVec4 colActive = ImVec4(1.f, 1.f, 1.f, 1.f);
		ImVec4 colInactive = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
		ImVec4 colHover = ImVec4(0.8f, 0.8f, 0.8f, 1.f);

		// Visuals tab
		{
			const char* txt = "visuals";
			ImVec2 textSize = ImGui::CalcTextSize(txt);

			ImVec2 pos = ImVec2(
				outerMin.x + paddingLeft,
				outerMin.y + (barHeight * 0.5f) - (textSize.y * 0.5f)
			);

			ImGui::SetCursorScreenPos(pos);
			ImGui::InvisibleButton("visuals_btn", textSize);

			bool hovered = ImGui::IsItemHovered();
			if (ImGui::IsItemClicked())
				selectedTab = 0;

			ImVec4 color = selectedTab == 0 ? colActive : (hovered ? colHover : colInactive);
			draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), txt);

			// Active tab underline
			if (selectedTab == 0)
			{
				draw->AddLine(
					ImVec2(pos.x, pos.y + textSize.y + 3),
					ImVec2(pos.x + textSize.x, pos.y + textSize.y + 3),
					IM_COL32(60, 140, 255, 255),
					2.0f
				);
			}
		}

		// Misc tab
		{
			const char* txt = "misc";
			ImVec2 textSize = ImGui::CalcTextSize(txt);

			ImVec2 pos = ImVec2(
				outerMin.x + paddingLeft + tabSpacing,
				outerMin.y + (barHeight * 0.5f) - (textSize.y * 0.5f)
			);

			ImGui::SetCursorScreenPos(pos);
			ImGui::InvisibleButton("misc_btn", textSize);

			bool hovered = ImGui::IsItemHovered();
			if (ImGui::IsItemClicked())
				selectedTab = 1;

			ImVec4 color = selectedTab == 1 ? colActive : (hovered ? colHover : colInactive);
			draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), txt);

			// Active tab underline
			if (selectedTab == 1)
			{
				draw->AddLine(
					ImVec2(pos.x, pos.y + textSize.y + 3),
					ImVec2(pos.x + textSize.x, pos.y + textSize.y + 3),
					IM_COL32(60, 140, 255, 255),
					2.0f
				);
			}
		}

		ImGui::PopFont();

		if (selectedTab == 0)
			render_visuals();
	}
	ImGui::End();
}