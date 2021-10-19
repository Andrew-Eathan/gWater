/*

	------- GMOD Binary Module Base by AndrewEathan -------

	This example module comes pre-packaged with https://github.com/Facepunch/gmod-module-base/tree/development, and it's fully set up with Debug/Release x32/x64 building.
	It hasn't been updated in 2 years so you can be sure you're not missing out on potential updates, while also avoiding tedious downloading and setup. 
	The Additional Include Directory is set to $(SolutionDir)include, so place any includes there, or modify it yourself in the project settings if you wish.

	By default this builds to a clientside module, but building for serverside is as simple as changing the name of the dll from gmcl_ to gmsv_.
	To build to both, you can create another template project in this solution for the other realm, clientside or serverside.

	After you build, place the .dll in garrysmod/lua/bin.
	NOTE: If you use any other dlls in your binary module, they will need to be placed in the GarrysMod folder!
	Garry's Mod loads those dlls from its root folder (again, GarrysMod)

	To include the resulting module in Garry's Mod, simply write:
		require("binary_module_name")

	Including the prefix/suffix of the .dll's name isn't necessary, Garry's Mod does the work for you there.
	To change the output module name, simply change the solution name.

*/

#pragma once
#include "GarrysMod/Lua/Interface.h"
#include "util.h"
#include "solver.h"
#include <Windows.h>
#include <string>
using namespace GarrysMod::Lua;

#define GLUA_Function(funcName, tblName) GlobalLUA->PushCFunction(funcName); GlobalLUA->SetField(-2, tblName);

ILuaBase* GlobalLUA = nullptr;

LUA_FUNCTION(Initialise) {
	float planeDepth = (float)LUA->GetNumber(-1);
	Solver::Initialise();
	Solver::SetPlaneDepth(planeDepth);
	return 0;
}

LUA_FUNCTION(SetRunningState) {
	Solver::SetRunningState(LUA->GetBool());
	return 0;
}

LUA_FUNCTION(GetParticles) {
	LUA->CreateTable();

	//loop thru all particles & add to table (on stack)
	for (int i = 0; i < Solver::particleCount; i++) {
		LUA->PushNumber((double)i + 1);

		float4 thisPos = Solver::publicParticles[i];

		Vector gmodPos;
		gmodPos.x = thisPos.x;
		gmodPos.y = thisPos.y;
		gmodPos.z = thisPos.z;

		LUA->PushVector(gmodPos);
		LUA->SetTable(-3);
	}

	return 1;
}

LUA_FUNCTION(CreateParticle) {
	Vector pos = LUA->GetVector(-2);
	Vector vel = LUA->GetVector(-1);
	QueuedParticle part;

	part.data = *new Particle{ pos.x, pos.y, pos.z, 0.5f };
	part.vel = *new float3{ vel.x, vel.y, vel.z };

	Solver::particleQueue.push_back(part);

	return 0;
}

LUA_FUNCTION(CreateCube) {
	Vector pos = LUA->GetVector(-3);
	Vector size = LUA->GetVector(-2);
	Vector vel = LUA->GetVector(-1);
	float radius = Solver::solverParams->radius;
	QueuedParticle part;

	for (float z = -size.z * radius; z < size.z * radius; z++)
		for (float y = -size.y * radius; y < size.y * radius; y++)
			for (float x = -size.x * radius; x < size.x * radius; x++) {
				part.data = *new Particle{ x + pos.x, y + pos.y, z + pos.z, 0.5f };
				part.vel = *new float3{ vel.x, vel.y, vel.z };

				Solver::particleQueue.push_back(part);
			}

	return 0;
}

LUA_FUNCTION(CleanParticles) {
	Solver::actionQueue.push_back(Solver::ActionQueue::CleanParticles);
	return 0;
}

LUA_FUNCTION(ParticleRadius) {
	float radius = (float)LUA->GetNumber();
	Solver::threadMutex->lock();
	Solver::solverParams->radius = radius;
	Solver::solverParams->fluidRestDistance = radius * 0.6f;
	Solver::solverParams->collisionDistance = radius * 0.25f;
	NvFlexSetParams(Solver::solver, Solver::solverParams);
	Solver::threadMutex->unlock();
	return 0;
}

LUA_FUNCTION(Destroy) {
	Solver::Destroy();
	return 0;
}

LUA_FUNCTION(AddSphereCollider) {
	Vector pos = LUA->GetVector(-2);
	float radius = (float)LUA->GetNumber(-1);
	Solver::SphereCollQueue coll;
	coll.geo.sphere.radius = radius;

	coll.position = float4(pos.x, pos.y, pos.z, 0.5f);
	coll.rotation = float4();
	
	coll.flags = NvFlexMakeShapeFlags(eNvFlexShapeSphere, false);
	Solver::sphereCollQueue.push_back(coll);
	return 0;
}

// Called when the module is loaded
GMOD_MODULE_OPEN()
{
	AllocConsole();
	FILE* pFile = nullptr;
	freopen_s(&pFile, "CONOUT$", "w", stdout);

	GlobalLUA = LUA;
	LUA_Print("Module opened");
	LUA->PushSpecial(SPECIAL_GLOB);
	LUA->CreateTable();

	GLUA_Function(Initialise, "Initialise");
	GLUA_Function(SetRunningState, "SetRunningState");
	GLUA_Function(GetParticles, "GetParticles");
	GLUA_Function(CreateParticle, "CreateParticle");
	GLUA_Function(CreateCube, "CreateCube");
	GLUA_Function(CleanParticles, "CleanParticles");
	GLUA_Function(AddSphereCollider, "AddSphereCollider");
	GLUA_Function(ParticleRadius, "ParticleRadius");
	GLUA_Function(Destroy, "Destroy");

	LUA->SetField(-2, "gwater");
	LUA->Pop(); //remove _G

	return 0;
}

// Called when the module is unloaded
GMOD_MODULE_CLOSE()
{
	LUA_Print("Module closed");
	FreeConsole();
	Solver::Destroy();
	return 0;
}