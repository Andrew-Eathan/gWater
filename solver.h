#pragma once
#include "util.h"

//quick fix to disable all C26812 enum-related warnings from flex
#pragma warning(push, 0)
#include "FleX/include/NvFlex.h"
#include "FleX/include/NvFlexExt.h"
#pragma warning(pop)

namespace Solver {
	//core
	extern NvFlexLibrary* library;
	extern NvFlexSolver* solver;
	extern NvFlexSolverDesc solverDesc;
	extern NvFlexParams* solverParams;
	
	//config
	extern int particleCount;
	extern int maxParticles;
	extern bool Valid;
	extern bool Running;
	extern float planeDepth;

	//enums
	enum class ActionQueue : int {
		CleanParticles
	};

	//buffers
	struct simBuffers {
		NvFlexVector<float4> particleBuffer;
		NvFlexVector<float3> velocityBuffer;
		NvFlexVector<int> phaseBuffer;
		NvFlexVector<int> activeBuffer;

		NvFlexVector<NvFlexCollisionGeometry> geometryBuffer;
		NvFlexVector<float4> positionsBuffer;
		NvFlexVector<float4> rotationsBuffer;
		NvFlexVector<float4> prevPositionsBuffer;
		NvFlexVector<float4> prevRotationsBuffer;
		NvFlexVector<int> flagsBuffer;

		simBuffers(NvFlexLibrary* lib) :
			particleBuffer(lib), velocityBuffer(lib), phaseBuffer(lib), activeBuffer(lib),
			geometryBuffer(lib), positionsBuffer(lib), rotationsBuffer(lib), flagsBuffer(lib),
			prevPositionsBuffer(lib), prevRotationsBuffer(lib)
		{}
	};

	extern simBuffers* flex_buffers;

	//workers and data
	extern float4* publicParticles;
	extern std::mutex* threadMutex;
	extern std::thread* threadWorker;

	//queues
	extern std::vector<QueuedParticle> particleQueue;
	extern std::vector<ActionQueue> actionQueue;

	//methods
	extern void Error(NvFlexErrorSeverity type, const char* msg, const char* file, int line);
	extern void Initialise();
	extern void ThreadMethod();
	extern void Destroy();
	extern void Sleep(int ms);
	extern void SetRunningState(bool running);
	extern void SetPlaneDepth(float pdepth);

	extern simBuffers* AllocBuffers(NvFlexLibrary* lib);
	extern void MapBuffers(simBuffers* buffers);
	extern void UnmapBuffers(simBuffers* buffers);
	extern void DestroyBuffers(simBuffers* buffers);
	extern void ClearBuffers(simBuffers* buffers);

	//helpers
	extern NvFlexSolverDesc CreateSolverDesc();
}