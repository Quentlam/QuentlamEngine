#pragma once

#include "Quentlam/Core/Timestep.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>



struct Particle
{
	bool Active = false;
	glm::vec2 Position;
	float Rotation;

	//Velocity
	glm::vec2 Velocity;
	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;

	float SizeBegin;
	float SizeEnd;

	float LifeTime;
	float LifeRemaining;

};

struct ParticleProps
{
	//Velocity
	glm::vec2 Velocity;
	glm::vec2 Position;
	glm::vec2 VelocityVariation;
	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;

	float SizeVariation;
	float SizeBegin;
	float SizeEnd;
	
	float LifeTime;
};

class ParticleSystem
{
public:

	ParticleSystem();
	~ParticleSystem();

	void Emit(const ParticleProps& particleProps);

	void OnUpdate(Quentlam::Timestep ts);

	void OnRender();

	uint32_t m_PoolIndex = 0;


private:
	std::vector<Particle> m_ParticlePool;

};

