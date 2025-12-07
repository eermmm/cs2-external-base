#include "main.h"
#include "p_loop.h"
#include "image.hpp"

namespace perseverance
{
	CheatInstance p_cheatinstance;
	std::shared_ptr<Cache> p_cache;
	IDirect3DTexture9* cat;
}

void cache_thread()
{
	while (true)
	{
		auto newc = std::make_shared<Cache>(&perseverance::p_cheatinstance);
		newc->update_all();

		std::atomic_store_explicit(&perseverance::p_cache, newc, std::memory_order_release);

		if (!perseverance::p_cheatinstance.p_mem->FindPid("cs2.exe"))
			break;

		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

int main()
{
	std::cerr << "Press F1 in cs2" << std::endl;
	while (!GetAsyncKeyState(VK_F1));

	if (!perseverance::p_cheatinstance.Initialize())
	{
		std::cerr << "Failed to Initialize" << std::endl;
		return 1;
	}

	HRESULT h = D3DXCreateTextureFromFileInMemory(perseverance::p_cheatinstance.p_overlay.dx9.device, kitty_cat, sizeof(kitty_cat), &perseverance::cat);
	if (h < 0)
		std::cerr << "Failed to Create Image" << std::endl;

	std::cerr << "Initialized" << std::endl;

	std::atomic_store(&perseverance::p_cache, std::make_shared<Cache>(&perseverance::p_cheatinstance));
	std::thread(cache_thread).detach();

	while (true)
	{
		main_loop(perseverance::p_cheatinstance);
	}

	return 0;
}