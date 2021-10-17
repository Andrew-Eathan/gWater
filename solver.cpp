#include "solver.h"
#include "util.h"
#include "params.h"


NvFlexLibrary* Solver::library = nullptr;
NvFlexSolver* Solver::solver = nullptr;
NvFlexSolverDesc Solver::solverDesc;
NvFlexParams* Solver::solverParams = nullptr;

int Solver::particleCount = 0;
float Solver::planeDepth = 12288.f;
bool Solver::Valid = false;
bool Solver::Running = false;

NvFlexBuffer* Solver::particleBuffer = nullptr;
NvFlexBuffer* Solver::velocityBuffer = nullptr;
NvFlexBuffer* Solver::phaseBuffer = nullptr;
NvFlexBuffer* Solver::activeBuffer = nullptr;

float4* Solver::publicParticles = nullptr;
std::mutex* Solver::threadMutex = nullptr;
std::thread* Solver::threadWorker = nullptr;

//queues
std::vector<QueuedParticle> Solver::particleQueue = std::vector<QueuedParticle>();
std::vector<Solver::ActionQueue> Solver::actionQueue = std::vector<Solver::ActionQueue>();



void Solver::Error(NvFlexErrorSeverity type, const char* msg, const char* file, int line) {
	LUA_Print("[GWATER] FleX error occured:	TYPE " + FLEX_GetErrorType(type) + "	MSG " + msg + "	FILE " + file + "	LINE " + std::to_string(line));
}

void Solver::SetPlaneDepth(float pdepth) {
	planeDepth = pdepth;
}

void Solver::Initialise() {
	library = NvFlexInit(NV_FLEX_VERSION, Solver::Error);
	solverDesc = CreateSolverDesc();
	solverParams = CreateSolverParams(planeDepth);
	solver = NvFlexCreateSolver(library, &solverDesc);

	NvFlexSetParams(solver, solverParams);

	particleBuffer = NvFlexAllocBuffer(library, solverDesc.maxParticles, sizeof(float4), eNvFlexBufferHost);
	velocityBuffer = NvFlexAllocBuffer(library, solverDesc.maxParticles, sizeof(float3), eNvFlexBufferHost);
	phaseBuffer = NvFlexAllocBuffer(library, solverDesc.maxParticles, sizeof(int), eNvFlexBufferHost);
	activeBuffer = NvFlexAllocBuffer(library, solverDesc.maxParticles, sizeof(int), eNvFlexBufferHost);
	publicParticles = stcast<float4*>(malloc(sizeof(float4) * 65536));

	particleQueue = std::vector<QueuedParticle>();

	Valid = true;

	threadMutex = new std::mutex();
	threadWorker = new std::thread(Solver::ThreadMethod);
	threadWorker->detach();
}

void Solver::SetRunningState(bool running) { Running = running; }
void Solver::Sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

timePoint curtime() {
	return std::chrono::high_resolution_clock::now();
}

void Solver::ThreadMethod() {
	float spt = 1.f / 60.f;
	int tps = stcast<int>(spt * 1000);

	if (!Valid)
		//note i didn't actually get this error before lol it's just to be sure
		//andrew from the future: got this once when i forgot to actually set Valid to true lol
		return LUA_Print("Simulation isn't valid when initialised, what the fuck");

	while (Valid) {
		if (!Running) {
			Sleep(tps);
			continue;
		}

		bool nulptr = false;

		if (particleBuffer == nullptr || velocityBuffer == nullptr || phaseBuffer == nullptr || activeBuffer == nullptr) {
			LUA_Print("ERROR: One of the four main buffers is NULLPTR!");
		}

		threadMutex->lock();
			timePoint timeStart = curtime();
			float4* particles = (float4*)(NvFlexMap(particleBuffer, eNvFlexMapWait));
			float3* velocities = (float3*)(NvFlexMap(velocityBuffer, eNvFlexMapWait));
			int* phases = (int*)(NvFlexMap(phaseBuffer, eNvFlexMapWait));
			int* activeIndices = (int*)(NvFlexMap(activeBuffer, eNvFlexMapWait));

			///////// PROCESS QUEUE DATA /////////
			// Action Queue
			for (const auto& action : actionQueue) {
				switch (action) {
				case ActionQueue::CleanParticles:
					memset(particles, 0, particleCount);
					memset(velocities, 0, particleCount);
					memset(phases, 0, particleCount);
					memset(activeIndices, 0, particleCount);
					particleCount = 0;
					break;
				}
			}
			// Particle Creation Queue
			for (const auto& particle : particleQueue) {
				particles[particleCount] = float4{ particle.data.x, particle.data.y, particle.data.z, particle.data.invMass };
				velocities[particleCount] = particle.vel;
				phases[particleCount] = NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);
				activeIndices[particleCount] = particleCount;

				particleCount++;
			}
			//////////////////////////////////////
			// clear queues afterwards
			actionQueue.clear();
			particleQueue.clear();
			//////////////////////////////////////
			

			memcpy(publicParticles, particles, sizeof(float4) * particleCount);

			// unmap buffers
			NvFlexUnmap(particleBuffer);
			NvFlexUnmap(velocityBuffer);
			NvFlexUnmap(phaseBuffer);
			NvFlexUnmap(activeBuffer);

			// write to device (async)	
			NvFlexSetParticles(solver, particleBuffer, NULL);
			NvFlexSetVelocities(solver, velocityBuffer, NULL);
			NvFlexSetPhases(solver, phaseBuffer, NULL);
			NvFlexSetActive(solver, activeBuffer, NULL);
			NvFlexSetActiveCount(solver, particleCount);

			timePoint timeEnd = curtime();
			std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart);

			// tick the solver
			NvFlexUpdateSolver(solver, (spt + diff.count() > 0 ? (float)diff.count() / (float)1000 : 0) * 5, 4, false);

			// read back (async)
			NvFlexGetParticles(solver, particleBuffer, NULL);
			NvFlexGetVelocities(solver, velocityBuffer, NULL);
			NvFlexGetPhases(solver, phaseBuffer, NULL);
		threadMutex->unlock();

		if (diff.count() - tps > 0) Sleep(tps - diff.count());
	}

	Valid = false;
}

void Solver::Destroy() {
	if (library != nullptr) {
		//cleanup
		Valid = false;
		Running = false;
		particleCount = 0;
		
		threadMutex->lock();
			NvFlexDestroySolver(solver);
			delete solverParams;
			delete threadWorker;
		threadMutex->unlock();
	}
}

NvFlexSolverDesc Solver::CreateSolverDesc() {
	NvFlexSolverDesc desc;
	NvFlexSetSolverDescDefaults(&desc);

	desc.maxParticles = 65536;
	desc.maxDiffuseParticles = 0;

	return desc;
}