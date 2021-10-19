#include "params.h"
#include "FleX/include/NvFlex.h"
#include <cfloat>

NvFlexParams* Solver::CreateSolverParams(float planeDepth) {
	NvFlexParams* params = new NvFlexParams();
	params->gravity[0] = 0.0f;
	params->gravity[1] = 0.0f;
	params->gravity[2] = -9.8f;

	params->wind[0] = 0.0f;
	params->wind[1] = 0.0f;
	params->wind[2] = 0.0f;

	params->radius = 10.f;
	params->viscosity = 0.002f;
	params->dynamicFriction = 0.05f;
	params->staticFriction = 0.05f;
	params->particleFriction = 0.02f; // scale friction between particles by default
	params->freeSurfaceDrag = 0.07f;
	params->drag = 0.0f;
	params->lift = 1.0f;
	params->numIterations = 4;
	params->fluidRestDistance = 6.f;
	params->solidRestDistance = 11.f;

	params->anisotropyScale = 1.0f;
	params->anisotropyMin = 0.1f;
	params->anisotropyMax = 2.0f;
	params->smoothing = 1.0f;

	params->dissipation = 0.0f;
	params->damping = 0.0f;
	params->particleCollisionMargin = 0.2f;
	params->shapeCollisionMargin = 1.0f;
	params->collisionDistance = params->radius * 0.25f;
	params->sleepThreshold = 0.0f;
	params->shockPropagation = 0.0f;
	params->restitution = 0.0f;

	params->maxSpeed = FLT_MAX;
	params->maxAcceleration = 100.0f;	// approximately 10x gravity
	params->relaxationMode = eNvFlexRelaxationLocal;
	params->relaxationFactor = 1.0f;
	params->solidPressure = 1.0f;
	params->adhesion = 0.003f;
	params->cohesion = 0.015f;
	params->surfaceTension = 0.0f;
	params->vorticityConfinement = 0.0f;
	params->buoyancy = 1.0f;
	params->diffuseThreshold = 100.0f;
	params->diffuseBuoyancy = 1.0f;
	params->diffuseDrag = 0.8f;
	params->diffuseBallistic = 16;
	params->diffuseLifetime = 2.0f;

	// planes created after particles
	params->planes[0][0] = 0.f;
	params->planes[0][1] = 0.f;
	params->planes[0][2] = 1.f;
	params->planes[0][3] = planeDepth;

	params->numPlanes = 1;

	return params;
}