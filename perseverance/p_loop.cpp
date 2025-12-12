#include <string>
#include <algorithm>
#include "p_loop.h"
#include "imgui/imgui.h"
#include "p_menu.h"

namespace settings
{
    bool menu_key = true;   
    bool box = false;
    bool skeleton = true;
    bool distance = false; 
    bool weapon = true;
    bool name = true;
    bool health = false;
    bool kitty = false;
}

void draw_cornered_box(float x, float y, float w, float h, const ImColor color, float thickness)
{
    float outline = thickness + 2;
    ImColor black = ImColor(0, 0, 0, 255);

    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + (h / 3)), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + (w / 3), y), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y), ImVec2(x + w, y), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + (h / 3)), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h - (h / 3)), ImVec2(x, y + h), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + (w / 3), y + h), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y + h), ImVec2(x + w, y + h), black, outline);
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y + h - (h / 3)), ImVec2(x + w, y + h), black, outline);

	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + (h / 3)), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + (w / 3), y), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y), ImVec2(x + w, y), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + (h / 3)), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h - (h / 3)), ImVec2(x, y + h), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + (w / 3), y + h), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y + h), ImVec2(x + w, y + h), color, thickness);
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y + h - (h / 3)), ImVec2(x + w, y + h), color, thickness);
}

float get_text_scale(float distance)
{
    float scale = 120.0f / distance;   
    return std::clamp(scale, 0.35f, 0.85f); 
}

void esp_loop()
{
    auto c = std::atomic_load_explicit(&perseverance::p_cache, std::memory_order_acquire);
    if (!c) return;

    c->update_misc();

    const LocalPlayer& local = c->get_local();
    if (!local.is_valid())
        return;

    const auto& players = c->get_players();
    for (const auto& player : players)
    {
        if (!player.is_valid() || !player.is_alive() || !player.is_enemy(local.get_team()))
            continue;

        vec3 origin = player.get_pos();
        vec3 head = player.get_bone(player.head);
        vec3 diff = local.get_pos() - player.get_pos();
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        distance = distance / 52.49f;

        head.z += 8.f;

        vec2 origin2d, head2d;
        if (!World2Screen(origin, origin2d, c->viewMatrix.matrix, c->Width, c->Height) ||
            !World2Screen(head, head2d, c->viewMatrix.matrix, c->Width, c->Height))
            continue;

        if (origin2d.x < -50 || origin2d.x > c->Width + 50 ||
            origin2d.y < -50 || origin2d.y > c->Height + 50 ||
            head2d.x < -50 || head2d.x > c->Width + 50 ||
            head2d.y < -50 || head2d.y > c->Height + 50)
            continue;

        float box_height = abs(head2d.y - origin2d.y);
        float box_width = box_height * 0.5f;
        float box_x = head2d.x - box_width / 2;
        float box_y = head2d.y;
        ImVec2 box_min(box_x, box_y);
        ImVec2 box_max(box_x + box_width, box_y + box_height);

        if (settings::box)
            draw_cornered_box(box_x, box_y, box_width, box_height, ImColor(255, 255, 255), 1.f);

        if (settings::health)
        {
            int health = player.get_health();
            ImColor health_color = health > 50 ? ImColor(0, 255, 0) : health > 25 ? ImColor(255, 255, 0) : ImColor(255, 0, 0);
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(box_x - 6, box_y - 1), ImVec2(box_x - 2, box_y + box_height + 1), ImColor(0, 0, 0));
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(box_x - 5, box_y + box_height - (box_height * health / 100.f)), ImVec2(box_x - 3, box_y + box_height), health_color);
        }

        if (settings::distance)
        {
            std::string dist_text = std::to_string((int)distance) + "m";

            float scale = get_text_scale(distance);
            float font_size = ImGui::GetFont()->FontSize * scale;

            ImFont* font = ImGui::GetFont();
            ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, dist_text.c_str());
            ImVec2 text_pos(box_x + box_width / 2 - text_size.x / 2, box_y + box_height + 2);

            float o = 1.0f;
            auto* dl = ImGui::GetBackgroundDrawList();

            dl->AddText(font, font_size, ImVec2(text_pos.x - o, text_pos.y), IM_COL32(0, 0, 0, 255), dist_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x + o, text_pos.y), IM_COL32(0, 0, 0, 255), dist_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y - o), IM_COL32(0, 0, 0, 255), dist_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y + o), IM_COL32(0, 0, 0, 255), dist_text.c_str());

            dl->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), dist_text.c_str());
        }

        if (settings::weapon)
        {
            const char* weapon_name = player.get_weapon();
            if (!weapon_name && strcmp(weapon_name, "Unknown") == 0)
                continue;

            std::string weapon_text = weapon_name;

            float scale = get_text_scale(distance);
            float font_size = ImGui::GetFont()->FontSize * scale;

            ImFont* font = ImGui::GetFont();
            ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, weapon_text.c_str());

            float y_offset = settings::distance ? (font_size + 4) : 2;
            ImVec2 text_pos(box_x + box_width / 2 - text_size.x / 2, box_y + box_height + y_offset);

            float o = 1.0f;
            auto* dl = ImGui::GetBackgroundDrawList();

            dl->AddText(font, font_size, ImVec2(text_pos.x - o, text_pos.y), IM_COL32(0, 0, 0, 255), weapon_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x + o, text_pos.y), IM_COL32(0, 0, 0, 255), weapon_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y - o), IM_COL32(0, 0, 0, 255), weapon_text.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y + o), IM_COL32(0, 0, 0, 255), weapon_text.c_str());

            dl->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), weapon_text.c_str());
        }

        if (settings::name)
        {
            std::string name = player.get_name();

            float scale = get_text_scale(distance);
            ImFont* font = ImGui::GetFont();
            float font_size = font->FontSize * scale;
            ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, name.c_str());
            if (text_size.x > box_width)
            {
                float max_scale = box_width / text_size.x;
                font_size *= max_scale;
                text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, name.c_str());
            }
            ImVec2 text_pos(box_x + box_width - text_size.x, box_y - text_size.y - 2);

            float o = 1.0f;
            auto* dl = ImGui::GetBackgroundDrawList();

            dl->AddText(font, font_size, ImVec2(text_pos.x - o, text_pos.y), IM_COL32(0, 0, 0, 255), name.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x + o, text_pos.y), IM_COL32(0, 0, 0, 255), name.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y - o), IM_COL32(0, 0, 0, 255), name.c_str());
            dl->AddText(font, font_size, ImVec2(text_pos.x, text_pos.y + o), IM_COL32(0, 0, 0, 255), name.c_str());

            dl->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), name.c_str());
        }

        if (settings::skeleton)
        {
            for (const auto& [from, to] : player.skeleton)
            {
                vec3 bone_start_3D = player.get_bone(from);
                vec3 bone_end_3D = player.get_bone(to);
                vec3 head3d = player.get_bone(player.head);

                vec2 screen_start, screen_end, headd;
                if (!World2Screen(bone_start_3D, screen_start, c->viewMatrix.matrix, c->Width, c->Height) ||
                    !World2Screen(bone_end_3D, screen_end, c->viewMatrix.matrix, c->Width, c->Height)     || 
                    !World2Screen(head3d, headd, c->viewMatrix.matrix, c->Width, c->Height))
                    continue;

                //ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screen_start.x, screen_start.y), ImVec2(screen_end.x, screen_end.y), ImColor(0, 0, 0, 255), 3.f);
                ImGui::GetBackgroundDrawList()->AddLine(ImVec2(screen_start.x, screen_start.y), ImVec2(screen_end.x, screen_end.y), ImColor(255, 255, 255, 255), 1.f);

                // head circle
                float radius = 120.0f / distance;        
                radius = std::clamp(radius, 1.0f, 3.0f);
                ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(headd.x, headd.y), radius, ImColor(255, 255, 255, 255), 16, 1.f);
            }
        }

        if (settings::kitty)
            ImGui::GetBackgroundDrawList()->AddImage(static_cast<ImTextureID>(perseverance::cat), box_min, box_max);
    }
}

void main_loop(CheatInstance& ci)
{
    static RECT oldRect = { 0 };
    ZeroMemory(&ci.p_overlay.dx9.message, sizeof(MSG));

    while (ci.p_overlay.dx9.message.message != WM_QUIT && perseverance::initialized == true)
    {
        if (PeekMessage(&ci.p_overlay.dx9.message,
            ci.p_overlay.window.hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&ci.p_overlay.dx9.message);
            DispatchMessage(&ci.p_overlay.dx9.message);
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            settings::menu_key = !settings::menu_key;

        if (GetAsyncKeyState(VK_F8) & 1)
            perseverance::initialized.store(false, std::memory_order_release);

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        input();

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		ImVec2 pos = ImVec2(10, 10);
		ImU32 color = IM_COL32(0, 133, 255, 255);
		draw_list->AddText(pos, color, ("perseverance"));

        if (settings::menu_key) {
            menu();

            SetWindowLong(ci.p_overlay.window.hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
            UpdateWindow(ci.p_overlay.window.hWnd);
            SetFocus(ci.p_overlay.window.hWnd);
        }
        else {
            SetWindowLong(ci.p_overlay.window.hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
            SetFocus(nullptr);
        }
         
        esp_loop();
        ImGui::EndFrame();

        ci.p_overlay.dx9.device->Clear(0, NULL, D3DCLEAR_TARGET,D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
        if (ci.p_overlay.dx9.device->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            ci.p_overlay.dx9.device->EndScene();
        }

        HRESULT result = ci.p_overlay.dx9.device->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST &&
            ci.p_overlay.dx9.device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            ci.p_overlay.dx9.device->Reset(&ci.p_overlay.dx9.params);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }
}