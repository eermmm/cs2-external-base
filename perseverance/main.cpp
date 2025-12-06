#include "main.h"
#include "p_loop.h"

namespace perseverance
{
	CheatInstance p_cheatinstance;
	std::atomic<Cache*> p_cache;
}

void cache_thread()
{
	while (true)
	{
		Cache* newc = new Cache(&perseverance::p_cheatinstance);
		newc->update_all();

		Cache* old = perseverance::p_cache.exchange(newc);
		delete old;

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

	perseverance::p_cache.store(new Cache(&perseverance::p_cheatinstance));
	std::thread(cache_thread).detach();

	while (true)
	{
		main_loop(perseverance::p_cheatinstance);
	}

	return 0;
}