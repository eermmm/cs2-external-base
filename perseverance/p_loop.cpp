#include "p_loop.h"
#include "imgui/imgui.h"

namespace settings
{
    bool box = false;
    bool skeleton = true;
    bool kitty = false;
}

void draw_cornered_box(float x, float y, float w, float h, const ImColor color, float thickness)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + (w / 3), y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y), ImVec2(x + w, y), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + (h / 3)), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h - (h / 3)), ImVec2(x, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + (w / 3), y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w - (w / 3), y + h), ImVec2(x + w, y + h), color, thickness);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(x + w, y + h - (h / 3)), ImVec2(x + w, y + h), color, thickness);
}

void esp_loop()
{
    auto c = std::atomic_load_explicit(&perseverance::p_cache, std::memory_order_acquire);

    if (!c) return;

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

        head.z += 8.f;

        vec2 origin2d, head2d;
        if (!World2Screen(origin, origin2d, c->viewMatrix.matrix, c->Width, c->Height) ||
            !World2Screen(head, head2d, c->viewMatrix.matrix, c->Width, c->Height))
            continue;

        if (origin2d.x < 0 || origin2d.x > c->Width ||
            origin2d.y < 0 || origin2d.y > c->Height)
            continue;

        if (head2d.x < 0 || head2d.x > c->Width ||
            head2d.y < 0 || head2d.y > c->Height)
            continue;

        float box_height = abs(head2d.y - origin2d.y);
        float box_width = box_height * 0.5f;
        float box_x = head2d.x - box_width / 2;
        float box_y = head2d.y;

        if (settings::box)
            draw_cornered_box(box_x, box_y, box_width, box_height, ImColor(255, 255, 255), 0.5f);

        if (settings::skeleton)
        {
            for (const auto& [from, to] : player.skeleton)
            {
                vec3 bone_start_3D = player.get_bone(from);
                vec3 bone_end_3D = player.get_bone(to);

                vec2 screen_start, screen_end;
                if (!World2Screen(bone_start_3D, screen_start, c->viewMatrix.matrix, c->Width, c->Height) ||
                    !World2Screen(bone_end_3D, screen_end, c->viewMatrix.matrix, c->Width, c->Height))
                    continue;

                ImGui::GetBackgroundDrawList()->AddLine(
                    ImVec2(screen_start.x, screen_start.y),
                    ImVec2(screen_end.x, screen_end.y),
                    ImColor(255, 255, 255, 255),
                    1.f
                );
            }
        }

        if (settings::kitty)
            ImGui::GetForegroundDrawList()->AddImage(
                (ImTextureID)perseverance::cat,
                ImVec2(box_x, box_y),                             
                ImVec2(box_x + box_width, box_y + box_height)
            );

    }
}

void main_loop(CheatInstance& ci)
{
    static RECT oldRect = { 0 };
    ZeroMemory(&ci.p_overlay.dx9.message, sizeof(MSG));

    while (ci.p_overlay.dx9.message.message != WM_QUIT)
    {
        if (PeekMessage(&ci.p_overlay.dx9.message,
            ci.p_overlay.window.hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&ci.p_overlay.dx9.message);
            DispatchMessage(&ci.p_overlay.dx9.message);
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

		ImDrawList* draw_list = ImGui::GetForegroundDrawList();
		ImVec2 pos = ImVec2(10, 10);
		ImU32 color = IM_COL32(0, 133, 255, 255);
		draw_list->AddText(pos, color, ("terry davis' third temple"));

        if (GetAsyncKeyState(VK_F1) & 1)
            settings::box = !settings::box;

        if (GetAsyncKeyState(VK_F2) & 1)
            settings::skeleton = !settings::skeleton;

        if (GetAsyncKeyState(VK_F3) & 1)
            settings::kitty = !settings::kitty;

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