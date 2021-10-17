#include "solver.h"
#include "util.h"
#include "params.h"


NvFlexLibrary* Solver::library = nullptr;
NvFlexSolver* Solver::solver = nullptr;
NvFlexSolverDesc Solver::solverDesc;
NvFlexParams* Solver::solverParams = nullptr;

int Solver::particleCount = 0;
int Solver::maxParticles = 65536;
float Solver::planeDepth = 12288.f;
bool Solver::Valid = false;
bool Solver::Running = false;

float4* Solver::publicParticles = nullptr;
Solver::simBuffers* Solver::flex_buffers = nullptr;
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
	flex_buffers = AllocBuffers(library);

	flex_buffers->particleBuffer.resize(maxParticles);
	flex_buffers->velocityBuffer.resize(maxParticles);
	flex_buffers->phaseBuffer.resize(maxParticles);
	flex_buffers->activeBuffer.resize(maxParticles);

	flex_buffers->geometryBuffer.resize(10);
	flex_buffers->positionsBuffer.resize(10);
	flex_buffers->rotationsBuffer.resize(10);
	flex_buffers->prevPositionsBuffer.resize(10);
	flex_buffers->prevRotationsBuffer.resize(10);
	flex_buffers->flagsBuffer.resize(10);

	publicParticles = stcast<float4*>(malloc(sizeof(float4) * maxParticles));
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

		threadMutex->lock();
			timePoint timeStart = curtime();
			MapBuffers(flex_buffers);

			///////// PROCESS QUEUE DATA /////////
			// Action Queue
			for (const auto& action : actionQueue) {
				switch (action) {
				case ActionQueue::CleanParticles:
					ClearBuffers(flex_buffers);
					MapBuffers(flex_buffers);
					particleCount = 0;
					break;
				}
			}
			// Particle Creation Queue
			for (const auto& particle : particleQueue) {
				flex_buffers->particleBuffer.push_back(float4(particle.data.x, particle.data.y, particle.data.z, particle.data.invMass));
				flex_buffers->velocityBuffer.push_back(particle.vel);
				flex_buffers->phaseBuffer.push_back(NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid));
				flex_buffers->activeBuffer.push_back(particleCount);

				particleCount++;
			}
			//////////////////////////////////////
			// clear queues afterwards
			actionQueue.clear();
			particleQueue.clear();
			//////////////////////////////////////
			
			flex_buffers->particleBuffer.copyto(publicParticles, particleCount);

			// unmap buffers
			UnmapBuffers(flex_buffers);

			// write to device (async)	
			NvFlexSetParticles(solver, flex_buffers->particleBuffer.buffer, NULL);
			NvFlexSetVelocities(solver, flex_buffers->velocityBuffer.buffer, NULL);
			NvFlexSetPhases(solver, flex_buffers->phaseBuffer.buffer, NULL);
			NvFlexSetActive(solver, flex_buffers->activeBuffer.buffer, NULL);
			NvFlexSetActiveCount(solver, particleCount);

			// send shapes to Flex
			NvFlexSetShapes(solver, flex_buffers->geometryBuffer.buffer, flex_buffers->positionsBuffer.buffer, flex_buffers->rotationsBuffer.buffer, NULL, NULL, flex_buffers->flagsBuffer.buffer, 1);

			timePoint timeEnd = curtime();
			std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart);

			// tick the solver
			NvFlexUpdateSolver(solver, (spt + diff.count() > 0 ? (float)diff.count() / (float)1000 : 0) * 5, 4, false);

			// read back (async)
 			NvFlexGetParticles(solver, flex_buffers->particleBuffer.buffer, NULL);
			NvFlexGetVelocities(solver, flex_buffers->velocityBuffer.buffer, NULL);
			NvFlexGetPhases(solver, flex_buffers->phaseBuffer.buffer, NULL);
		threadMutex->unlock();

		if (diff.count() - tps > 0) Sleep(tps - (int)diff.count());
	}

	Valid = false;
}

void Solver::Destroy() {
	if (library != nullptr || Valid) {
		//cleanup
		Valid = false;
		Running = false;
		particleCount = 0;
		
	threadMutex->lock();
		delete solverParams;
		delete threadWorker;
		NvFlexDestroySolver(solver);
		NvFlexShutdown(library);
	threadMutex->unlock();
	}
}

NvFlexSolverDesc Solver::CreateSolverDesc() {
	NvFlexSolverDesc desc;
	NvFlexSetSolverDescDefaults(&desc);

	desc.maxParticles = maxParticles;
	desc.maxDiffuseParticles = 0;

	return desc;
}

Solver::simBuffers* Solver::AllocBuffers(NvFlexLibrary* lib) {
	return new Solver::simBuffers(lib);
}

//yucky but must be done
void Solver::MapBuffers(simBuffers* buffers) {
	buffers->particleBuffer.map();
	buffers->velocityBuffer.map();
	buffers->phaseBuffer.map();
	buffers->activeBuffer.map();

	buffers->flagsBuffer.map();
	buffers->geometryBuffer.map();
	buffers->positionsBuffer.map();
	buffers->rotationsBuffer.map();
	buffers->prevPositionsBuffer.map();
	buffers->prevRotationsBuffer.map();
}

void Solver::UnmapBuffers(simBuffers* buffers) {
	buffers->particleBuffer.unmap();
	buffers->velocityBuffer.unmap();
	buffers->phaseBuffer.unmap();
	buffers->activeBuffer.unmap();

	buffers->flagsBuffer.unmap();
	buffers->geometryBuffer.unmap();
	buffers->positionsBuffer.unmap();
	buffers->rotationsBuffer.unmap();
	buffers->prevPositionsBuffer.unmap();
	buffers->prevRotationsBuffer.unmap();
}

void Solver::DestroyBuffers(simBuffers* buffers) {
	buffers->particleBuffer.destroy();
	buffers->velocityBuffer.destroy();
	buffers->phaseBuffer.destroy();
	buffers->activeBuffer.destroy();

	buffers->flagsBuffer.destroy();
	buffers->geometryBuffer.destroy();
	buffers->positionsBuffer.destroy();
	buffers->rotationsBuffer.destroy();
	buffers->prevPositionsBuffer.destroy();
	buffers->prevRotationsBuffer.destroy();
}

//hacky, but what else can i do without .clear()?
void Solver::ClearBuffers(simBuffers* buffers) {
	buffers->positionsBuffer.init(maxParticles);
	buffers->velocityBuffer.init(maxParticles);
	buffers->phaseBuffer.init(maxParticles);
	buffers->activeBuffer.init(maxParticles);

	buffers->geometryBuffer.init(10);
	buffers->particleBuffer.init(10);
	buffers->rotationsBuffer.init(10);
	buffers->flagsBuffer.init(10);
	buffers->prevPositionsBuffer.init(10);
	buffers->prevRotationsBuffer.init(10);
}