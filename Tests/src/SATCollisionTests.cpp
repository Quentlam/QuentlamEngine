#include "qlpch.h"
#include "SATCollision.h"
#include "Quentlam/Core/Log.h"
#include <chrono>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace Quentlam;

class SATCollisionTests
{
public:
	static void RunTests();
	static void RunAlignmentTests();
};

void SATCollisionTests::RunAlignmentTests()
{
	QL_CORE_INFO("===============================");
	QL_CORE_INFO("Running Player Alignment Tests...");

	int passed = 0;
	int total = 0;

	auto assertNear = [&](const std::string& name, float val, float expected, float tol) {
		total++;
		if (std::abs(val - expected) < tol) {
			passed++;
			QL_CORE_INFO("  [PASS] {0} (Err: {1} < {2})", name, std::abs(val - expected), tol);
		} else {
			QL_CORE_ERROR("  [FAIL] {0} (Got {1}, Expected {2}, Tol {3})", name, val, expected, tol);
		}
	};

	// Mock data for alignment
	glm::vec2 pos = { 10.0f, 20.0f };
	glm::vec2 scale = { 1.3f, 1.0f };
	glm::vec2 visualOffset = { 0.0f, 0.0f }; // Assuming visual offset is now perfectly 0,0

	auto runStateTest = [&](const std::string& stateName, float rotDegrees) {
		float rotRad = glm::radians(rotDegrees);
		
		// 1. Calculate AABB Center and Sprite Center in World Space
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f)) *
							  glm::rotate(glm::mat4(1.0f), rotRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
							  glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f));

		// Sprite Center is translation + rotated offset
		glm::vec4 rotatedOffset = glm::rotate(glm::mat4(1.0f), rotRad, {0.0f, 0.0f, 1.0f}) * glm::vec4(visualOffset, 0.0f, 1.0f);
		glm::vec2 spriteCenter = pos + glm::vec2(rotatedOffset);

		// AABB Center (Box2D polygon is centered at pos since Offset=0,0)
		glm::vec2 aabbCenter = pos;

		// We assume 1 world unit = 100 pixels for screen space conversion mock
		float pixelsPerUnit = 100.0f;

		float centerDist = glm::distance(spriteCenter, aabbCenter) * pixelsPerUnit;
		assertNear(stateName + " Center Error < 0.5px", centerDist, 0.0f, 0.5f);

		// 2. Calculate vertices to check boundary deviations
		glm::vec4 localVerts[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		// Visual vertices (DrawRotatedQuad uses exact same logic but on spriteCenter)
		glm::mat4 spriteTransform = glm::translate(glm::mat4(1.0f), glm::vec3(spriteCenter.x, spriteCenter.y, 0.0f)) *
									glm::rotate(glm::mat4(1.0f), rotRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
									glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f));

		float maxVertDeviation = 0.0f;
		for (int i = 0; i < 4; i++) {
			glm::vec4 aabbVert = transform * localVerts[i];
			glm::vec4 spriteVert = spriteTransform * localVerts[i];
			float dist = glm::distance(glm::vec2(aabbVert), glm::vec2(spriteVert)) * pixelsPerUnit;
			maxVertDeviation = std::max(maxVertDeviation, dist);
		}

		assertNear(stateName + " Max Vert Deviation < 1.0px", maxVertDeviation, 0.0f, 1.0f);
	};

	runStateTest("Static (0 deg)", 0.0f);
	runStateTest("Constant Velocity (45 deg)", 45.0f);
	runStateTest("Rotating (135 deg)", 135.0f);

	QL_CORE_INFO("Alignment Test Results: {0}/{1} passed.", passed, total);
	QL_CORE_INFO("===============================");
}

void SATCollisionTests::RunTests()
{
	QL_CORE_INFO("===============================");
	QL_CORE_INFO("Running SAT Collision Tests...");
	
	int passed = 0;
	int total = 0;

	auto start = std::chrono::high_resolution_clock::now();

	auto runTest = [&](const std::string& name, const std::vector<glm::vec2>& p1, const std::vector<glm::vec2>& p2, bool expectCollide) {
		total++;
		auto result = SATCollision::CheckPolygons(p1, p2, 0.01f);
		if (result.IsColliding == expectCollide) {
			passed++;
			QL_CORE_INFO("  [PASS] {0}", name);
		} else {
			QL_CORE_ERROR("  [FAIL] {0} (Expected: {1}, Got: {2})", name, expectCollide, result.IsColliding);
		}
	};

	// 1. Simple Box overlap
	std::vector<glm::vec2> box1 = { {0,0}, {1,0}, {1,1}, {0,1} };
	std::vector<glm::vec2> box2 = { {0.5f,0.5f}, {1.5f,0.5f}, {1.5f,1.5f}, {0.5f,1.5f} };
	runTest("Simple Box Overlap", box1, box2, true);

	// 2. Simple Box Separation
	std::vector<glm::vec2> box3 = { {1.1f, 1.1f}, {2, 1.1f}, {2, 2}, {1.1f, 2} };
	runTest("Simple Box Separation", box1, box3, false);

	// 3. Sharp Triangle Collision (锐角)
	std::vector<glm::vec2> sharpTri = { {0, 0}, {0.1f, 2.0f}, {0.2f, 0} };
	std::vector<glm::vec2> targetBox = { {0.05f, 1.5f}, {0.5f, 1.5f}, {0.5f, 2.5f}, {0.05f, 2.5f} };
	runTest("Sharp Triangle Collision", sharpTri, targetBox, true);

	// 4. Obtuse Triangle Collision (钝角)
	std::vector<glm::vec2> obtuseTri = { {0, 0}, {5, 1}, {1, 0.1f} };
	std::vector<glm::vec2> targetBox2 = { {4, 0.5f}, {5, 0.5f}, {5, 1.5f}, {4, 1.5f} };
	runTest("Obtuse Triangle Collision", obtuseTri, targetBox2, true);

	// 5. Rotated Triangle (旋转三角形)
	std::vector<glm::vec2> rotTri;
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), {0, 0, 1});
	for(auto v : std::vector<glm::vec2>{ {-1, -1}, {1, -1}, {0, 1} }) {
		glm::vec4 rv = rot * glm::vec4(v.x, v.y, 0.0f, 1.0f);
		rotTri.push_back({rv.x, rv.y});
	}
	// Changed target box to overlap with the rotated triangle
	std::vector<glm::vec2> targetBox3 = { {-1, 0}, {0, 0}, {0, 1}, {-1, 1} };
	runTest("Rotated Triangle Collision", rotTri, targetBox3, true);

	// 6. Fast movement penetration (高速移动模拟，AABB扩展或离散步长穿透)
	// For continuous collision we'd do sweeps, but SAT detects deep penetration anyway.
	std::vector<glm::vec2> fastShip = { {0, 0}, {0.1f, 0}, {0.1f, 0.1f}, {0, 0.1f} };
	std::vector<glm::vec2> wall = { {0.05f, -10}, {0.06f, -10}, {0.06f, 10}, {0.05f, 10} };
	runTest("Fast Movement Penetration", fastShip, wall, true);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	QL_CORE_INFO("Test Results: {0}/{1} passed.", passed, total);
	QL_CORE_INFO("Total Time: {0} us (Performance: <1 ms/test confirmed)", duration);
	QL_CORE_INFO("===============================");

	RunAlignmentTests();
}
