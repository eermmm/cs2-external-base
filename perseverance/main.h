#pragma once
#include <Windows.h>
#include <iostream>
#include "p_memory.h"
#include "p_instance.h"
#include "p_cheat.h"
#include <thread>
#include <chrono>
#include <atomic>

namespace perseverance
{
	extern CheatInstance p_cheatinstance;
	extern std::shared_ptr<Cache> p_cache;
	extern IDirect3DTexture9* cat;
	extern bool initialized;
}