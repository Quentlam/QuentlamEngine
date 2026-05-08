#pragma once

#include "Quentlam.h"
#include "ParkourComponents.h"

namespace Quentlam {

	class ParkourSystem
	{
	public:
		static void OnRuntimeStart(Scene* scene, bool isEditor = false);
		static void OnUpdate(Scene* scene, Timestep ts);
		static void OnRuntimeStop(Scene* scene);

		// Helper to build the initial scene based on our data-driven config
		static void BuildScene(Scene* scene);

		// Serialize current scene entities to a JSON file
		static void SaveScene(Scene* scene, const std::string& filepath);
		static bool LoadScene(Scene* scene, const std::string& filepath);

	private:
		static void UpdatePlayer(Scene* scene, Timestep ts, bool isMainMenu);
		static void UpdateObstacles(Scene* scene, Timestep ts);
		static void CheckCollisions(Scene* scene);
	};

}
