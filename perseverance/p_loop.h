#pragma once
#include "p_cheat.h"	
#include "main.h"

namespace settings
{
	extern bool menu_key;
	extern bool box;
	extern bool skeleton;
	extern bool distance;
	extern bool weapon;
	extern bool name;
	extern bool health;
	extern bool kitty;

	extern bool aim;
	extern bool show_fov;
	extern float fov;
	extern int keybind;
	extern float smoothing;
}

extern void main_loop(CheatInstance& ci);