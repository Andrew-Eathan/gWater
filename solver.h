#pragma once
#include "util.h"

//quick fix to disable all C26812 enum-related warnings from flex
#pragma warning(push, 0)
	#include "FleX/include/NvFlex.h"
#pragma warning(pop)

namespace Solver {
	//core
	extern NvFlexLibrary* library;
	extern NvFlexSolver* solver;
	extern NvFlexSolverDesc solverDesc;
	extern NvFlexParams* solverParams;
	
	//config
	extern int particleCount;
	extern int geometryCount;
	extern bool Valid;
	extern bool Running;
	extern float planeDepth;

	//enums
	enum ActionQueue {
		CleanParticles
	};

	struct SphereCollQueue {
		NvFlexCollisionGeometry geo;
		float4 position;
		float4 rotation;
		int flags;
	};

	//buffers
	extern NvFlexBuffer* particleBuffer;
	extern NvFlexBuffer* velocityBuffer;
	extern NvFlexBuffer* phaseBuffer;
	extern NvFlexBuffer* activeBuffer;

	extern NvFlexBuffer* geometryBuffer;
	extern NvFlexBuffer* positionBuffer;
	extern NvFlexBuffer* rotationBuffer;
	extern NvFlexBuffer* flagsBuffer;
	
	//workers and data
	extern float4* publicParticles;
	extern std::mutex* threadMutex;
	extern std::thread* threadWorker;

	//queues
	extern std::vector<QueuedParticle> particleQueue;
	extern std::vector<ActionQueue> actionQueue;
	extern std::vector<SphereCollQueue> sphereCollQueue;

	//methods
	extern void Error(NvFlexErrorSeverity type, const char* msg, const char* file, int line);
	extern void Initialise();
	extern void ThreadMethod();
	extern void Destroy();
	extern void Sleep(int ms);
	extern void SetRunningState(bool running);
	extern void SetPlaneDepth(float pdepth);

	//helpers
	extern NvFlexSolverDesc CreateSolverDesc();
}