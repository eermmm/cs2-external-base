#include "p_cheat.h"
#include "p_instance.h"

vec3 PlayerEntity::get_bone(BONEINDEX bone) const
{
	const auto* c = cheat;
	auto& mem = c->p_mem;

	DWORD64 sceneNode = mem->read<DWORD64>(playerPawn + 0x330);
	if (!sceneNode) return vec3();

	DWORD64 boneArray = mem->read<DWORD64>(sceneNode + 0x190 + 0x80);
	if (!boneArray) return vec3();

	return mem->read<vec3>(boneArray + bone * 32);
}

vec3 PlayerEntity::get_pos() const {
	return cheat->p_mem->read<vec3>(playerPawn + 0x15A0);
}

int PlayerEntity::get_team() const {
	return cheat->p_mem->read<int>(playerPawn + 0x3EB); // unselvoed external in comp game!
}

int PlayerEntity::get_health() const {
	return cheat->p_mem->read<int>(playerPawn + 0x34C);
}

bool PlayerEntity::is_alive() const {
	return get_health() > 0;
}

bool PlayerEntity::is_enemy(int ltid) const {
	return ltid != get_team();
}

void LocalPlayer::update()
{
	const auto* c = cheat;
	auto& mem = c->p_mem;
	auto& mods = c->p_modules;

	playerController = mem->read<DWORD64>(mods.ClientBase + offsets::dwLocalPlayerController);
	playerPawn = mem->read<DWORD64>(mods.ClientBase + offsets::dwLocalPlayerPawn);
}

void Cache::update_players()
{
	const auto* c = cheat;
	auto& mem = c->p_mem;
	auto& mods = c->p_modules;
	this->entityList.clear();

	DWORD64 listEntry = mem->read<DWORD64>(mods.ClientBase + offsets::dwEntityList);
	if (!listEntry) return;

	for (int i = 0; i < 64; ++i) // max players
	{
		DWORD64 controllerEntry = mem->read<DWORD64>(listEntry + (8 * (i & 0x7FFF) >> 9) + 0x10);
		if (!controllerEntry) continue;

		DWORD64 controller =	  mem->read<DWORD64>(controllerEntry + 112 * (i & 0x1FF));
		if (!controller) continue;

		DWORD64 pawnHandle =	  mem->read<DWORD64>(controller + 0x8FC); // m_hPlayerPawn
		if (!pawnHandle) continue;

		int pawnIndex = pawnHandle & 0x7FFF;
		DWORD64 pawnEntry =       mem->read<DWORD64>(listEntry + 8 * (pawnIndex >> 9) + 0x10);
		if (!pawnEntry) continue;

		DWORD64 pawn =			  mem->read<DWORD64>(pawnEntry + 112 * (pawnIndex & 0x1FF));
		if (!pawn) continue;

		entityList.emplace_back(controller, pawn, cheat);
	}
}

void Cache::update_misc()
{
	const auto* c = cheat;
	auto& mem = c->p_mem;
	auto& mods = c->p_modules;
	viewMatrix = mem->read<view_matrix_t>(mods.ClientBase + offsets::dwViewMatrix);
	Width =		 mem->read<int>(mods.Engine2Base + offsets::dwWindowWidth);
	Height =	 mem->read<int>(mods.Engine2Base + offsets::dwWindowHeight);
}

bool World2Screen(const vec3& Pos, vec2& ToPos, const float Matrix[4][4], int Width, int Height)
{
	float View = 0.f;
	float SightX = (float)Width / 2, SightY = (float)Height / 2;

	View = Matrix[3][0] * Pos.x + Matrix[3][1] * Pos.y + Matrix[3][2] * Pos.z + Matrix[3][3];

	if (View <= 0.01)
		return false;

	ToPos.x = SightX + (Matrix[0][0] * Pos.x + Matrix[0][1] * Pos.y + Matrix[0][2] * Pos.z + Matrix[0][3]) / View * SightX;
	ToPos.y = SightY - (Matrix[1][0] * Pos.x + Matrix[1][1] * Pos.y + Matrix[1][2] * Pos.z + Matrix[1][3]) / View * SightY;

	return true;
}