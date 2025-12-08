#include "p_instance.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

bool CheatInstance::AttachToProcess()
{
	if (p_meminst.IsAttached())
		return false;

	if (p_memoryMethod != UM)
		return false;

	if (!p_meminst.Attach(p_processName))
		return false;

	p_modules.ProcessBase = p_meminst.GetBaseAddress(p_processName);
	p_modules.ClientBase = p_meminst.GetBaseAddress("client.dll");
	p_modules.Engine2Base = p_meminst.GetBaseAddress("engine2.dll");
	std::cerr << "ClientBase 0x" << std::hex << p_modules.ClientBase << std::endl;
	std::cerr << "Engine2Base 0x" << std::hex << p_modules.Engine2Base << std::endl;

	if (!p_modules.ProcessBase || !p_modules.ClientBase || !p_modules.Engine2Base)
		return false;

	return true;
}

bool CheatInstance::Initialize()
{
	if (!AttachToProcess())
		return false;

	std::cerr << "Attached to cs2" << std::endl;

	p_mem = &this->p_meminst;
	if (!p_mem->IsAttached())
		return false;

	std::cerr << "Initialized memory, tab into game" << std::endl;

	bool WindowFocus = false;
	while (!WindowFocus) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (p_mem->FindPid(p_processName) == ForegroundWindowProcessID) {
			WindowFocus = true;
		}
	}

	if (!p_overlay.Initialize())
		return false;

	std::cerr << "Initialized overlay" << std::endl;

	return true;
}

void CheatInstance::Uninitialize()
{
	if (ImGui::GetCurrentContext()) {
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	if (p_overlay.dx9.device) {
		p_overlay.dx9.device->Release();
		p_overlay.dx9.device = nullptr;
	}

	if (p_overlay.dx9.d3d9) {
		p_overlay.dx9.d3d9->Release();
		p_overlay.dx9.d3d9 = nullptr;
	}

	if (p_mem && p_mem->IsAttached())
		CloseHandle(p_mem->GetHandle());

	p_mem = nullptr;
}