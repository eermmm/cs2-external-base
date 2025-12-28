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

struct SliderAnimation {
    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
    float valueGlow = 0.0f;
    float grabPulse = 0.0f;
};

struct KeybindAnimation {
    float hoverAlpha = 0.0f;
    float activeAlpha = 0.0f;
    float pulseTime = 0.0f;
    float flashAlpha = 0.0f;
};

// Global animation storage
static std::map<std::string, CheckboxAnimation> checkboxAnims;
static std::map<std::string, SectionAnimation> sectionAnims;
static std::map<std::string, float> buttonAnims;
static std::map<std::string, KeybindAnimation> keybindAnims;
static std::map<std::string, SliderAnimation> sliderAnims;

const char* GetKeyName(int key) {
    static char keyName[32];

    if (key == 0) return "None";

    // Special keys
    switch (key) {
    case VK_LBUTTON: return "LMB";
    case VK_RBUTTON: return "RMB";
    case VK_MBUTTON: return "MMB";
    case VK_XBUTTON1: return "Mouse4";
    case VK_XBUTTON2: return "Mouse5";
    case VK_SHIFT: return "Shift";
    case VK_CONTROL: return "Ctrl";
    case VK_MENU: return "Alt";
    case VK_CAPITAL: return "Caps";
    case VK_TAB: return "Tab";
    case VK_SPACE: return "Space";
    case VK_RETURN: return "Enter";
    case VK_BACK: return "Backspace";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "PgUp";
    case VK_NEXT: return "PgDn";
    default:
        if (key >= 0x30 && key <= 0x39) { // 0-9
            snprintf(keyName, sizeof(keyName), "%c", key);
            return keyName;
        }
        if (key >= 0x41 && key <= 0x5A) { // A-Z
            snprintf(keyName, sizeof(keyName), "%c", key);
            return keyName;
        }
        if (key >= VK_F1 && key <= VK_F24) {
            snprintf(keyName, sizeof(keyName), "F%d", key - VK_F1 + 1);
            return keyName;
        }
        snprintf(keyName, sizeof(keyName), "Key %d", key);
        return keyName;
    }
}

bool AnimatedKeybindButton(const char* label, int* key, bool* isListening) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 labelSize = ImGui::CalcTextSize(label);

    float buttonWidth = 120.0f;
    float buttonHeight = 26.0f;
    float spacing = 15.0f;

    ImVec2 buttonPos = ImVec2(pos.x + labelSize.x + spacing, pos.y - 3);
    ImVec2 totalSize = ImVec2(labelSize.x + spacing + buttonWidth, ImMax(labelSize.y, buttonHeight));

    // Button hitbox
    ImGui::SetCursorScreenPos(buttonPos);
    ImGui::InvisibleButton(label, ImVec2(buttonWidth, buttonHeight));
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // Get or create animation state
    std::string key_str = std::string(label);
    auto& anim = keybindAnims[key_str];

    float dt = ImGui::GetIO().DeltaTime;

    // Animate hover
    float targetHover = hovered ? 1.0f : 0.0f;
    anim.hoverAlpha += (targetHover - anim.hoverAlpha) * 15.0f * dt;

    // Animate active/listening state
    float targetActive = *isListening ? 1.0f : 0.0f;
    anim.activeAlpha += (targetActive - anim.activeAlpha) * 12.0f * dt;

    // Pulse animation when listening
    if (*isListening) {
        anim.pulseTime += dt * 4.0f;
    }
    else {
        anim.pulseTime = 0.0f;
    }

    // Flash animation when key changes
    if (anim.flashAlpha > 0.0f) {
        anim.flashAlpha = ImMax(0.0f, anim.flashAlpha - dt * 4.0f);
    }

    // Handle click to start listening
    if (clicked) {
        *isListening = !*isListening;
    }

    // Handle key detection when listening
    if (*isListening) {
        for (int k = 0; k < 256; k++) {
            if (GetAsyncKeyState(k) & 1) {
                if (k != VK_LBUTTON && k != VK_RBUTTON && k != VK_ESCAPE) {
                    *key = k;
                    *isListening = false;
                    anim.flashAlpha = 1.0f;
                    break;
                }
                else if (k == VK_ESCAPE) {
                    *isListening = false;
                    break;
                }
            }
        }
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Draw label
    float labelBrightness = ImLerp(160.0f, 200.0f, anim.hoverAlpha);
    ImU32 labelColor = IM_COL32(labelBrightness, labelBrightness, labelBrightness, 255);
    draw->AddText(pos, labelColor, label);

    // Button background with animated colors
    ImVec2 btnMin = buttonPos;
    ImVec2 btnMax = ImVec2(buttonPos.x + buttonWidth, buttonPos.y + buttonHeight);
    float rounding = 4.0f;

    // Background color transitions
    float bgBrightness = ImLerp(18.0f, 28.0f, anim.hoverAlpha);
    bgBrightness = ImLerp(bgBrightness, 35.0f, anim.activeAlpha);
    ImU32 bgColor = IM_COL32(bgBrightness, bgBrightness, bgBrightness, 255);
    draw->AddRectFilled(btnMin, btnMax, bgColor, rounding);

    // Pulsing glow when listening
    if (*isListening) {
        float pulseFactor = (sin(anim.pulseTime) * 0.5f + 0.5f);
        float glowSize = 3.0f + pulseFactor * 2.0f;
        float glowAlpha = 60.0f + pulseFactor * 80.0f;

        draw->AddRect(
            ImVec2(btnMin.x - glowSize, btnMin.y - glowSize),
            ImVec2(btnMax.x + glowSize, btnMax.y + glowSize),
            IM_COL32(80, 140, 255, glowAlpha),
            rounding + 1.0f,
            0,
            2.0f
        );
    }

    // Flash effect when key changes
    if (anim.flashAlpha > 0.01f) {
        float flashIntensity = anim.flashAlpha * 120.0f;
        draw->AddRect(
            ImVec2(btnMin.x - 1, btnMin.y - 1),
            ImVec2(btnMax.x + 1, btnMax.y + 1),
            IM_COL32(100, 255, 100, flashIntensity),
            rounding,
            0,
            2.0f
        );
    }

    // Border with animations
    float borderBrightness = ImLerp(40.0f, 80.0f, anim.hoverAlpha);
    borderBrightness = ImLerp(borderBrightness, 140.0f, anim.activeAlpha);

    ImU32 borderColor = *isListening
        ? IM_COL32(80, 140, 255, 255)
        : IM_COL32(borderBrightness, borderBrightness, borderBrightness, 255);

    draw->AddRect(btnMin, btnMax, borderColor, rounding, 0, 1.0f);

    // Button text
    const char* displayText = *isListening ? "..." : GetKeyName(*key);
    ImVec2 textSize = ImGui::CalcTextSize(displayText);
    ImVec2 textPos = ImVec2(
        btnMin.x + (buttonWidth - textSize.x) * 0.5f,
        btnMin.y + (buttonHeight - textSize.y) * 0.5f
    );

    float textBrightness = ImLerp(140.0f, 200.0f, anim.hoverAlpha);
    if (*isListening) {
        float pulseFactor = (sin(anim.pulseTime * 1.5f) * 0.5f + 0.5f);
        textBrightness = ImLerp(180.0f, 240.0f, pulseFactor);
    }
    textBrightness += anim.flashAlpha * 60.0f;

    ImU32 textColor = IM_COL32(textBrightness, textBrightness, textBrightness, 255);
    draw->AddText(textPos, textColor, displayText);

    // Restore cursor position for proper layout
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + totalSize.y + 5));

    return clicked;
}


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

bool AnimatedFloatSlider(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f") {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float sliderWidth = 200.0f;
    float sliderHeight = 4.0f;
    float grabRadius = 6.0f;

    // Layout: Label on left, slider on right
    ImVec2 sliderPos = ImVec2(pos.x + labelSize.x + 15.0f, pos.y + labelSize.y * 0.5f - sliderHeight * 0.5f);
    ImVec2 totalSize = ImVec2(labelSize.x + 15.0f + sliderWidth, ImMax(labelSize.y, grabRadius * 2));

    ImGui::InvisibleButton(label, totalSize);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    // Get or create animation state
    std::string key = std::string(label);
    auto& anim = sliderAnims[key];

    float dt = ImGui::GetIO().DeltaTime;

    // Animate hover
    float targetHover = hovered ? 1.0f : 0.0f;
    anim.hoverAlpha += (targetHover - anim.hoverAlpha) * 15.0f * dt;

    // Animate active/dragging
    float targetActive = active ? 1.0f : 0.0f;
    anim.activeAlpha += (targetActive - anim.activeAlpha) * 20.0f * dt;

    // Value change glow
    static float lastValue = *v;
    if (fabsf(*v - lastValue) > 0.01f) {
        anim.valueGlow = 1.0f;
        lastValue = *v;
    }
    anim.valueGlow = ImMax(0.0f, anim.valueGlow - dt * 3.0f);

    // Grab pulse animation
    anim.grabPulse += dt * 4.0f;

    // Handle dragging
    if (active) {
        float mouseX = ImGui::GetIO().MousePos.x;
        float normalizedValue = ImClamp((mouseX - sliderPos.x) / sliderWidth, 0.0f, 1.0f);
        *v = v_min + normalizedValue * (v_max - v_min);
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Draw label
    float labelBrightness = ImLerp(160.0f, 200.0f, anim.hoverAlpha);
    ImU32 labelColor = IM_COL32(labelBrightness, labelBrightness, labelBrightness, 255);
    draw->AddText(pos, labelColor, label);

    // Slider track background
    ImVec2 trackMin = sliderPos;
    ImVec2 trackMax = ImVec2(sliderPos.x + sliderWidth, sliderPos.y + sliderHeight);
    float trackBrightness = ImLerp(30.0f, 40.0f, anim.hoverAlpha);
    draw->AddRectFilled(trackMin, trackMax, IM_COL32(trackBrightness, trackBrightness, trackBrightness, 255), 2.0f);

    // Filled portion with gradient and glow
    float fillRatio = (*v - v_min) / (v_max - v_min);
    ImVec2 fillMax = ImVec2(sliderPos.x + sliderWidth * fillRatio, sliderPos.y + sliderHeight);

    float fillBrightness = ImLerp(80.0f, 140.0f, EaseInOutCubic(fillRatio));
    fillBrightness += anim.activeAlpha * 40.0f + anim.valueGlow * 60.0f;

    // Gradient fill
    ImU32 fillColorLeft = IM_COL32(fillBrightness * 0.7f, fillBrightness * 0.7f, fillBrightness, 255);
    ImU32 fillColorRight = IM_COL32(fillBrightness, fillBrightness, fillBrightness, 255);
    draw->AddRectFilledMultiColor(trackMin, fillMax, fillColorLeft, fillColorRight, fillColorRight, fillColorLeft);

    // Outer glow on filled portion
    if (anim.valueGlow > 0.01f || anim.activeAlpha > 0.01f) {
        float glowAlpha = ImMax(anim.valueGlow, anim.activeAlpha) * 100.0f;
        draw->AddRect(
            ImVec2(trackMin.x - 1, trackMin.y - 1),
            ImVec2(fillMax.x + 1, fillMax.y + 1),
            IM_COL32(120, 140, 255, glowAlpha),
            2.0f,
            0,
            1.5f
        );
    }

    // Grab (slider thumb)
    ImVec2 grabCenter = ImVec2(sliderPos.x + sliderWidth * fillRatio, sliderPos.y + sliderHeight * 0.5f);
    float grabSize = ImLerp(grabRadius, grabRadius * 1.3f, anim.activeAlpha);

    // Pulsing shadow/glow
    float pulseFactor = (sin(anim.grabPulse) * 0.5f + 0.5f);
    float shadowRadius = grabSize + 2.0f + pulseFactor * 2.0f * anim.hoverAlpha;
    float shadowAlpha = ImLerp(40.0f, 100.0f, anim.hoverAlpha);
    draw->AddCircleFilled(grabCenter, shadowRadius, IM_COL32(0, 0, 0, shadowAlpha), 16);

    // Main grab circle
    float grabBrightness = ImLerp(140.0f, 220.0f, anim.activeAlpha);
    grabBrightness += anim.hoverAlpha * 30.0f;
    draw->AddCircleFilled(grabCenter, grabSize, IM_COL32(grabBrightness, grabBrightness, grabBrightness, 255), 16);

    // Grab border
    float borderBrightness = ImLerp(180.0f, 255.0f, anim.hoverAlpha);
    draw->AddCircle(grabCenter, grabSize, IM_COL32(borderBrightness, borderBrightness, borderBrightness, 255), 16, 1.0f);

    // Value text
    char valueText[32];
    snprintf(valueText, sizeof(valueText), format, *v);
    ImVec2 valueTextSize = ImGui::CalcTextSize(valueText);
    ImVec2 valueTextPos = ImVec2(sliderPos.x + sliderWidth + 10.0f, pos.y + (labelSize.y - valueTextSize.y) * 0.5f);

    float valueBrightness = ImLerp(140.0f, 200.0f, anim.valueGlow);
    valueBrightness += anim.hoverAlpha * 40.0f;
    ImU32 valueColor = IM_COL32(valueBrightness, valueBrightness, valueBrightness, 255);
    draw->AddText(valueTextPos, valueColor, valueText);

    return active;
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

void render_aim() {
    if (selectedTab != 1) return;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

    ImGui::SetCursorPos(ImVec2(15, 50));
    ImGui::BeginGroup();

    render_child_section("aimbot", 290, 300, []() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));

        MinimalCheckBox("enable", &settings::aim);
        MinimalCheckBox("show fov", &settings::show_fov);

        AnimatedFloatSlider("fov", &settings::fov, 10.0f, 500.0f, "%.0f");
        AnimatedFloatSlider("smooth", &settings::smoothing, 1.0f, 20.0f);

        ImGui::PopStyleVar();
        });

    ImGui::SetCursorPos(ImVec2(315, 50));

    render_child_section("keybind", 290, 300, []() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));

        static bool isListeningForKey = false;

        AnimatedKeybindButton("aim key", &settings::keybind, &isListeningForKey);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

        ImGui::PopStyleVar();
        });

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

void render_misc() {
    if (selectedTab != 2) return;

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
        float tabSpacing = 25.f; // spacing between tabs
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

        float currentTabX = paddingLeft;

        // Visuals tab
        {
            const char* txt = "visuals";
            ImVec2 textSize = ImGui::CalcTextSize(txt);
            ImVec2 pos = ImVec2(
                outerMin.x + currentTabX,
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

            currentTabX += textSize.x + tabSpacing;
        }

        // Aim tab
        {
            const char* txt = "aim";
            ImVec2 textSize = ImGui::CalcTextSize(txt);
            ImVec2 pos = ImVec2(
                outerMin.x + currentTabX,
                outerMin.y + (barHeight * 0.5f) - (textSize.y * 0.5f)
            );

            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton("aim_btn", textSize);
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

            currentTabX += textSize.x + tabSpacing;
        }

        // Misc tab
        {
            const char* txt = "misc";
            ImVec2 textSize = ImGui::CalcTextSize(txt);
            ImVec2 pos = ImVec2(
                outerMin.x + currentTabX,
                outerMin.y + (barHeight * 0.5f) - (textSize.y * 0.5f)
            );

            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton("misc_btn", textSize);
            bool hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked()) selectedTab = 2;

            ImVec4 color = selectedTab == 2 ? colActive : (hovered ? colHover : colInactive);
            draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), txt);

            // Animated underline
            if (selectedTab == 2) {
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
            render_aim();
        else if (selectedTab == 2)
            render_misc();
        ImGui::PopStyleVar();
    }
    ImGui::End();
}