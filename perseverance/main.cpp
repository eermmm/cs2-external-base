#include "main.h"
#include "p_loop.h"

namespace perseverance
{
	CheatInstance p_cheatinstance;
	std::shared_ptr<Cache> p_cache;
}

void cache_thread()
{
	while (true)
	{
		auto newc = std::make_shared<Cache>(&perseverance::p_cheatinstance);
		newc->update_all();

		std::atomic_store_explicit(&perseverance::p_cache, newc, std::memory_order_release);


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

	std::cerr << "Initialized" << std::endl;

	std::atomic_store(&perseverance::p_cache, std::make_shared<Cache>(&perseverance::p_cheatinstance));
	std::thread(cache_thread).detach();

	while (true)
	{
		main_loop(perseverance::p_cheatinstance);
	}

	return 0;
}