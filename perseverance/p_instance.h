#pragma once
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include "p_memory.h"
#include "p_overlay.h"
#include "p_cheat.h"

class CheatInstance
{
public:

	CheatInstance() = default;
	explicit CheatInstance(const std::string& processName) : p_processName(processName) {}
	~CheatInstance() = default;

	struct ProcessModules
	{
		DWORD64 ProcessBase = NULL;
		DWORD64 Engine2Base = NULL;
		DWORD64  ClientBase = NULL;
	};

	// km isnt supported js yet
	enum MemoryMethod
	{
		KM = 0,
		UM = 1
	};

	bool AttachToProcess();
	bool	  Initialize();
	
	ProcessModules p_modules{};
	Overlay		   p_overlay{};
	UmMemoryInstance*  p_mem{};

private:
	std::string p_processName = "cs2.exe";
	UmMemoryInstance			p_meminst;
	MemoryMethod	  p_memoryMethod = UM;

};