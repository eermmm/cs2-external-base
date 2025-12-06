#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <dwmapi.h>
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"


// ty chat gpt for making this readable

class OverlayWindow
{
public:
    HWND hWnd = nullptr;
    LPCSTR Name = nullptr;

    bool FindDiscordOverlay()
    {
        HWND temp = FindWindowA("Chrome_WidgetWin_1", "Discord Overlay");
        if (!temp)
            return false;

        hWnd = temp;
        return true;
    }
};

class DX9Renderer
{
public:
    IDirect3D9Ex* d3d9 = nullptr;
    IDirect3DDevice9Ex* device = nullptr;
    D3DPRESENT_PARAMETERS params{};
    MARGINS margin{ -1 };
    MSG message{};

    bool Init(HWND hWnd)
    {
        if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9)))
            return false;

        ZeroMemory(&params, sizeof(params));
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.hDeviceWindow = hWnd;
        params.BackBufferFormat = D3DFMT_A8R8G8B8;
        params.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
        params.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
        params.EnableAutoDepthStencil = TRUE;
        params.AutoDepthStencilFormat = D3DFMT_D16;
        params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        if (FAILED(d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, 0, &device)))
        {
            d3d9->Release();
            return false;
        }

        return true;
    }

    void Cleanup()
    {
        if (device) device->Release();
        if (d3d9) d3d9->Release();
    }
};

class ImGuiManager
{
public:
    void Init(HWND hWnd, IDirect3DDevice9Ex* device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        io.FontDefault = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/consola.ttf", 13.0f);

        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(device);
    }
};

class Overlay
{
public:
    OverlayWindow window;
    DX9Renderer   dx9;
    ImGuiManager  imgui;

    bool Initialize()
    {
        if (!window.FindDiscordOverlay())
            return false;

        if (!dx9.Init(window.hWnd))
            return false;

        imgui.Init(window.hWnd, dx9.device);

        dx9.d3d9->Release();

        return true;
    }
};