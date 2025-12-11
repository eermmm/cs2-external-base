#include "main.h"
#include "p_loop.h"
#include "image.hpp"

namespace perseverance
{
	CheatInstance p_cheatinstance;
	std::shared_ptr<Cache> p_cache;
	IDirect3DTexture9* cat;
	std::atomic_bool initialized{ false };
}

void cache_thread()
{
	while (perseverance::initialized.load())
	{
		if (!perseverance::p_cheatinstance.p_mem->FindPid("cs2.exe"))
			break;

		auto newc = std::make_shared<Cache>(&perseverance::p_cheatinstance);
		newc->update_local();
		newc->update_players();

		std::atomic_store_explicit(&perseverance::p_cache, newc, std::memory_order_release);

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	perseverance::initialized.store(false, std::memory_order_release);
}

int main()
{
	std::cerr << "Press F1 in cs2" << std::endl;	

	if (!RegisterHotKey(nullptr, 1, 0, VK_F1)) {
		std::cerr << "Failed to register hotkey" << std::endl;
		return 1;
	}

	MSG msg = { 0 };
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (msg.message == WM_HOTKEY && msg.wParam == 1)
			break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterHotKey(nullptr, 1);


	if (!perseverance::p_cheatinstance.Initialize())
	{
		std::cerr << "Failed to Initialize" << std::endl;
		return 1;
	}
	perseverance::initialized.store(true, std::memory_order_release);

	HRESULT h = D3DXCreateTextureFromFileInMemory(perseverance::p_cheatinstance.p_overlay.dx9.device, kitty_cat, sizeof(kitty_cat), &perseverance::cat);
	if (h < 0)
		std::cerr << "Failed to Create Image" << std::endl;

	std::cerr << "Initialized" << std::endl;

	std::atomic_store(&perseverance::p_cache, std::make_shared<Cache>(&perseverance::p_cheatinstance));
	std::thread cacheThread = std::thread(cache_thread);

	while (perseverance::initialized.load())
	{
		main_loop(perseverance::p_cheatinstance);
	}

	if (cacheThread.joinable())
		cacheThread.join();

	perseverance::p_cache.reset();
	perseverance::p_cheatinstance.Uninitialize();

	return 0;
}