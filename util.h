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


struct float4 { float x, y, z, w; };
struct float3 { float x, y, z; };

struct Particle { float x, y, z, invMass; };
struct QueuedParticle { Particle data; float3 vel; };

extern ILuaBase* GlobalLUA;

void LUA_Print(std::string text);
void LUA_Print(char* text);
void LUA_Print(int num);
std::string FLEX_GetErrorType(NvFlexErrorSeverity type);