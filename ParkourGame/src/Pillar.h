#pragma once
#include "qlpch.h"

#include <glm/glm.hpp>
class b2Body;

struct Pillar
{
	glm::vec3 TopPosition = { 0.0f,10.0f,0.0f };
	glm::vec2 TopScale = { 15.0f,20.0f };
	b2Body* TopBody = nullptr;
	
	glm::vec3 BottomPosition = { 10.0f,10.0f,0.0f };
	glm::vec2 BottomScale = { 15.0f,20.0f };
	b2Body* BottomBody = nullptr;

	b2Body* SensorBody = nullptr;
	bool Scored = false;
};

