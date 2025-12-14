#include "p_menu.h"
#include "p_loop.h"
#include "imgui/imgui_internal.h"
#include <functional>
#include <map>

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


// Animation helper functions
float SmoothStep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float EaseOutCubic(float t) {
    return 1.0f - pow(1.0f - t, 3.0f);
}

float EaseInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// Animation states
struct CheckboxAnimation {
    float checkAlpha = 0.0f;
    float hoverAlpha = 0.0f;
    float borderPulse = 0.0f;
};

struct SectionAnimation {
    float slideIn = 0.0f;
    float fadeIn = 0.0f;
};

// Global animation storage
static std::map<std::string, CheckboxAnimation> checkboxAnims;
static std::map<std::string, SectionAnimation> sectionAnims;
static std::map<std::string, float> buttonAnims;

bool MinimalCheckBox(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

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

    if (clicked) *v = !*v;

    // Get or create animation state
    std::string key = std::string(label);
    auto& anim = checkboxAnims[key];

    float dt = ImGui::GetIO().DeltaTime;

    // Animate check state
    float targetCheck = *v ? 1.0f : 0.0f;
    anim.checkAlpha += (targetCheck - anim.checkAlpha) * 12.0f * dt;

    // Animate hover state
    float targetHover = hovered ? 1.0f : 0.0f;
    anim.hoverAlpha += (targetHover - anim.hoverAlpha) * 15.0f * dt;

    // Pulse animation when checked
    if (*v) {
        anim.borderPulse += dt * 3.0f;
    }
    else {
        anim.borderPulse = 0.0f;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    float rounding = 2.0f;

    // Animated background color
    ImVec4 bgColorStart = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.0f);
    ImVec4 bgColorEnd = ImVec4(45 / 255.0f, 45 / 255.0f, 45 / 255.0f, 1.0f);
    ImVec4 bgColor = ImLerp(bgColorStart, bgColorEnd, EaseInOutCubic(anim.checkAlpha));
    draw->AddRectFilled(boxMin, boxMax, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);

    // Animated border with hover and pulse effect
    float borderBrightness = ImLerp(70.0f, 120.0f, anim.checkAlpha);
    float pulseFactor = (sin(anim.borderPulse) * 0.5f + 0.5f) * 20.0f;
    borderBrightness += anim.hoverAlpha * 70.0f + (*v ? pulseFactor : 0.0f);
    ImU32 borderColor = IM_COL32(borderBrightness, borderBrightness, borderBrightness, 255);
    draw->AddRect(boxMin, boxMax, borderColor, rounding, 0, 1.0f);

    // Animated filled indicator
    if (anim.checkAlpha > 0.01f) {
        ImVec2 innerMin = ImVec2(boxMin.x + 2, boxMin.y + 2);
        ImVec2 innerMax = ImVec2(boxMax.x - 2, boxMax.y - 2);

        float innerBrightness = ImLerp(100.0f, 180.0f, EaseOutCubic(anim.checkAlpha));
        ImU32 innerColor = IM_COL32(innerBrightness, innerBrightness, innerBrightness, 255 * anim.checkAlpha);
        draw->AddRectFilled(innerMin, innerMax, innerColor, rounding * 0.5f);
    }

    // Animated label with color transition
    ImVec2 labelPos = ImVec2(boxMax.x + style.ItemInnerSpacing.x, pos.y + (boxSize - textSize.y) * 0.5f);
    float textBrightness = ImLerp(160.0f, 220.0f, anim.checkAlpha);
    ImU32 textColor = IM_COL32(textBrightness, textBrightness, textBrightness, 255);
    draw->AddText(labelPos, textColor, label);

    return clicked;
}

void render_child_section(const char* title, float width, float height, std::function<void()> content) {
    std::string key = std::string(title);
    auto& anim = sectionAnims[key];

    float dt = ImGui::GetIO().DeltaTime;

    // Slide in from left and fade in
    if (anim.slideIn < 1.0f) {
        anim.slideIn = ImMin(anim.slideIn + dt * 4.0f, 1.0f);
    }
    if (anim.fadeIn < 1.0f) {
        anim.fadeIn = ImMin(anim.fadeIn + dt * 5.0f, 1.0f);
    }

    float slideProgress = EaseOutCubic(anim.slideIn);
    float fadeProgress = anim.fadeIn;

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 originalPos = ImGui::GetCursorScreenPos();

    // Offset for slide animation
    float slideOffset = (1.0f - slideProgress) * 30.0f;
    ImVec2 pos = ImVec2(originalPos.x - slideOffset, originalPos.y);

    // Draw section header background with fade
    ImU32 headerBg = IM_COL32(28, 28, 28, 255 * fadeProgress);
    draw->AddRectFilled(
        pos,
        ImVec2(pos.x + width, pos.y + 20),
        headerBg
    );

    // Draw header bottom border with glow effect
    static float glowTime = 0.0f;
    glowTime += dt;
    float glowIntensity = (sin(glowTime * 2.0f) * 0.5f + 0.5f) * 0.3f + 0.7f;
    ImU32 borderColor = IM_COL32(45 * glowIntensity, 45 * glowIntensity, 45 * glowIntensity, 255 * fadeProgress);
    draw->AddLine(
        ImVec2(pos.x, pos.y + 20),
        ImVec2(pos.x + width, pos.y + 20),
        borderColor,
        1.0f
    );

    // Draw title text with fade
    ImVec2 textSize = ImGui::CalcTextSize(title);
    ImU32 textColor = IM_COL32(180, 180, 180, 255 * fadeProgress);
    draw->AddText(
        ImVec2(pos.x + 8, pos.y + (20 - textSize.y) * 0.5f),
        textColor,
        title
    );

    // Adjust cursor for slide offset
    ImGui::SetCursorScreenPos(ImVec2(originalPos.x - slideOffset, originalPos.y));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);

    // Child window with fade
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fadeProgress);
    ImGui::BeginChild(title, ImVec2(width, height), true, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetCursorPos(ImVec2(8, 8));
        ImGui::BeginGroup();
        content();
        ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void render_visuals() {
    if (selectedTab != 0) return;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

    ImGui::SetCursorPos(ImVec2(15, 50));
    ImGui::BeginGroup();

    render_child_section("player esp", 290, 300, []() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));
        MinimalCheckBox("box", &settings::box);
        MinimalCheckBox("skeleton", &settings::skeleton);
        MinimalCheckBox("cat", &settings::kitty);
        ImGui::PopStyleVar();
        });

    ImGui::SetCursorPos(ImVec2(315, 50));

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

void render_misc() {
    if (selectedTab != 1) return;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

    ImGui::SetCursorPos(ImVec2(15, 50));
    ImGui::BeginGroup();

    render_child_section("misc", 290, 300, []() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));

        // Animated button
        std::string btnKey = "exit_btn";
        auto& btnAnim = buttonAnims[btnKey];

        bool hovered = false;
        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImVec2 btnSize = ImVec2(40, 20);

        if (ImGui::Button("Exit", btnSize)) {
            perseverance::initialized.store(false, std::memory_order_release);
        }
        hovered = ImGui::IsItemHovered();

        float dt = ImGui::GetIO().DeltaTime;
        float target = hovered ? 1.0f : 0.0f;
        btnAnim += (target - btnAnim) * 12.0f * dt;

        // Draw glow effect on hover
        if (btnAnim > 0.01f) {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            float glowAlpha = btnAnim * 80.0f;
            draw->AddRect(
                ImVec2(btnPos.x - 1, btnPos.y - 1),
                ImVec2(btnPos.x + btnSize.x + 1, btnPos.y + btnSize.y + 1),
                IM_COL32(255, 100, 100, glowAlpha),
                3.0f,
                0,
                2.0f
            );
        }

        ImGui::PopStyleVar();
        });

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(4);
}

static int prevTab = -1;
static float tabTransition = 0.0f;

void menu() {
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
        draw->AddText(font, font->FontSize, ImVec2(textX, textY), ImGui::ColorConvertFloat4ToU32(c1), title);
        draw->PopClipRect();

        draw->PushClipRect(ImVec2(textX + textSize.x * 0.5f, textY), ImVec2(textX + textSize.x, textY + textSize.y), true);
        draw->AddText(font, font->FontSize, ImVec2(textX, textY), ImGui::ColorConvertFloat4ToU32(c2), title);
        draw->PopClipRect();

        ImGui::PushFont(ImGui::GetFont());

        float paddingLeft = 18.f;
        float tabSpacing = 65.f;
        ImVec4 colActive = ImVec4(1.f, 1.f, 1.f, 1.f);
        ImVec4 colInactive = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
        ImVec4 colHover = ImVec4(0.8f, 0.8f, 0.8f, 1.f);

        // Tab change detection and animation
        if (prevTab != selectedTab) {
            tabTransition = 0.0f;
            prevTab = selectedTab;

            // Reset section animations when switching tabs
            sectionAnims.clear();
        }

        float dt = ImGui::GetIO().DeltaTime;
        if (tabTransition < 1.0f) {
            tabTransition = ImMin(tabTransition + dt * 6.0f, 1.0f);
        }

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
            if (ImGui::IsItemClicked()) selectedTab = 0;

            ImVec4 color = selectedTab == 0 ? colActive : (hovered ? colHover : colInactive);
            draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), txt);

            // Animated underline
            if (selectedTab == 0) {
                float underlineProgress = EaseOutCubic(tabTransition);
                float underlineWidth = textSize.x * underlineProgress;
                draw->AddLine(
                    ImVec2(pos.x, pos.y + textSize.y + 3),
                    ImVec2(pos.x + underlineWidth, pos.y + textSize.y + 3),
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
            if (ImGui::IsItemClicked()) selectedTab = 1;

            ImVec4 color = selectedTab == 1 ? colActive : (hovered ? colHover : colInactive);
            draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), txt);

            // Animated underline
            if (selectedTab == 1) {
                float underlineProgress = EaseOutCubic(tabTransition);
                float underlineWidth = textSize.x * underlineProgress;
                draw->AddLine(
                    ImVec2(pos.x, pos.y + textSize.y + 3),
                    ImVec2(pos.x + underlineWidth, pos.y + textSize.y + 3),
                    IM_COL32(60, 140, 255, 255),
                    2.0f
                );
            }
        }

        ImGui::PopFont();

        // Content fade transition
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tabTransition);
        if (selectedTab == 0)
            render_visuals();
        else if (selectedTab == 1)
            render_misc();
        ImGui::PopStyleVar();
    }
    ImGui::End();
}