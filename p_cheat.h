#pragma once
#include "p_instance.h"
class CheatInstance;
#include <vector>
#include "offsets.h"

struct vec2
{
	float x, y;
	vec2 operator+(const vec2& other) const
	{
		return { x + other.x, y + other.y };
	}
	vec2 operator-(const vec2& other) const
	{
		return { x - other.x, y - other.y };
	}
	vec2 operator-() const {
		return { -x, -y };
	}

};

struct vec3
{
	float x, y, z;
	vec3 operator+(const vec3& other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	vec3 operator-(const vec3& other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}
	vec3 operator*(float scalar) const
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	vec3 operator/(float scalar) const
	{
		return { x / scalar, y / scalar, z / scalar };
	}

	vec3 operator/(const vec3& other) const
	{
		return { x / other.x, y / other.y, z / other.z };
	}

	 double distance(vec3 v) { return double(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0))); }

	 inline vec3 normalize() const
	{
		float len = std::sqrt(x * x + y * y + z * z);
		if (len <= 0.00001f)
			return { 0.0f, 0.0f, 0.0f };
		return { x / len, y / len, z / len };
	}

	inline float dot(const vec3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}
};

struct view_matrix_t {
	float matrix[4][4];

	float* operator[](int i) { return matrix[i]; }
	const float* operator[](int i) const { return matrix[i]; }
};

class PlayerEntity
{
public:
	PlayerEntity(DWORD64 pc, DWORD64 pp, const CheatInstance* cheatPtr) : playerController(pc), playerPawn(pp), cheat(cheatPtr) { }
	~PlayerEntity() = default;

	enum BONEINDEX : DWORD
	{
		pelvis = 0,
		spine_1 = 2,
		spine_3 = 4,
		neck = 5,
		head = 6,

		clavicle_l = 7,
		arm_upper_l = 8,
		arm_lower_l = 9,
		hand_l = 10,

		clavicle_r = 12,
		arm_upper_r = 13,
		arm_lower_r = 14,
		hand_r = 15,

		leg_upper_l = 22,
		leg_lower_l = 23,
		ankle_l = 24,

		leg_upper_r = 25,
		leg_lower_r = 26,
		ankle_r = 27
	};

	const std::vector<std::pair<BONEINDEX, BONEINDEX>> skeleton = {
		{ head, neck },
		{ neck, spine_3 },
		{ spine_3, spine_1 },
		{ spine_1, pelvis },

		// Left arm
		{ spine_3, clavicle_l },
		{ clavicle_l, arm_upper_l },
		{ arm_upper_l, arm_lower_l },
		{ arm_lower_l, hand_l },

		// Right arm
		{ spine_3, clavicle_r },
		{ clavicle_r, arm_upper_r },
		{ arm_upper_r, arm_lower_r },
		{ arm_lower_r, hand_r },

		// Left leg
		{ pelvis, leg_upper_l },
		{ leg_upper_l, leg_lower_l },
		{ leg_lower_l, ankle_l },

		// Right leg
		{ pelvis, leg_upper_r },
		{ leg_upper_r, leg_lower_r },
		{ leg_lower_r, ankle_r }
	};


	vec3 get_bone(BONEINDEX bone) const;
	vec3				get_pos() const;
	int				   get_team() const;
	int				 get_health() const;
	const char*		   get_name() const;
	const char*		 get_weapon() const;
	bool	           is_alive() const;
	bool	   is_enemy(int ltid) const;
	bool is_valid() const { return playerController && playerPawn; }

protected:
	DWORD64				playerController = 0;
	DWORD64					  playerPawn = 0;
	const CheatInstance* cheat = nullptr;
};

class LocalPlayer : public PlayerEntity
{
public:

	LocalPlayer(const CheatInstance* cheatPtr) : PlayerEntity(0, 0, cheatPtr) { }
	~LocalPlayer() = default;

	void update();
};

class Cache
{
public:
	Cache(const CheatInstance* cheatPtr) : local(cheatPtr), cheat(cheatPtr) { }

	view_matrix_t viewMatrix{};
	int			     Width = 0;
	int			    Height = 0;

	void clear() { entityList.clear(); }
	const std::vector<PlayerEntity>& get_players() const { return entityList; }
	const LocalPlayer& get_local() const { return local; }
	void update_misc();
	void update_local() { local.update(); }
	void update_players();

private:
	LocalPlayer					   local;
	std::vector<PlayerEntity> entityList;

protected:
	const CheatInstance* cheat = nullptr;
};

extern bool World2Screen(const vec3& Pos, vec2& ToPos, const float Matrix[4][4], int Width, int Height);