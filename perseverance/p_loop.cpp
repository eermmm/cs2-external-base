#include <string>
#include <algorithm>
#include "p_loop.h"
#include "imgui/imgui.h"
#include "p_menu.h"
#include "esp.h"
#include "aim.h"

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

    bool aim = false;
    bool show_fov = false;
    float fov = 30.f;
    int keybind = 0x01;
    float smoothing = 1.f;
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

        auto c = std::atomic_load_explicit(&perseverance::p_cache, std::memory_order_acquire);
        if (!c) return;
         
        esp_loop(c);
        aim_loop(c);
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