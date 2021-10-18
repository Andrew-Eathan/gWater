#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include "GarrysMod/Lua/Interface.h"
#define stcast static_cast
#define timePoint std::chrono::steady_clock::time_point

using namespace GarrysMod::Lua;

//quick fix to disable all C26812 enum-related warnings from flex
#pragma warning(push, 0)
	#include "FleX/include/NvFlex.h"
#pragma warning(pop)


struct float3 {
	float x, y, z;
	float3(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {};
	float3() : x(0), y(0), z(0) {};
};

struct float4 {
	float x, y, z, w;
	float4(float3 fl, float w1) : x(fl.x), y(fl.y), z(fl.z), w(w1) {};
	float4(float x1, float y1, float z1, float w1) : x(x1), y(y1), z(z1), w(w1) {};
	float4() : x(0), y(0), z(0), w(0) {};
};

struct Particle { float x, y, z, invMass; };
struct QueuedParticle { Particle data; float3 vel; };

extern ILuaBase* GlobalLUA;

void LUA_Print(std::string text);
void LUA_Print(char* text);
void LUA_Print(int num);
std::string FLEX_GetErrorType(NvFlexErrorSeverity type);