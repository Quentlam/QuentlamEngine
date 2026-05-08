#include "qlpch.h"
#include "ParkourSystem.h"
#include "Quentlam/Physics/Physics2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

#include <glm/gtc/matrix_transform.hpp>

#include "Quentlam/Core/Input.h"
#include "Quentlam/Core/KeyCodes.h"
// Random generator
#include <random>
#include <sstream>
#include <fstream>

namespace Quentlam {

	static float RandomFloat()
	{
		return (float)rand() / (float)RAND_MAX;
	}

	bool ParkourSystem::LoadScene(Scene* scene, const std::string& filepath)
	{
		// Since we don't have a full JSON SceneSerializer yet, we manually parse the required basic data
		// for our specific ParkourGameTemplate scene.
		std::ifstream stream(filepath);
		if (!stream.is_open()) return false;

		std::stringstream ss;
		ss << stream.rdbuf();
		std::string content = ss.str();

		// Basic string find/substring based JSON parser for our simple format
		auto extractEntityBlock = [&content](const std::string& entityName) -> std::string {
			std::string searchKey = "\"Name\": \"" + entityName + "\"";
			size_t pos = content.find(searchKey);
			if (pos == std::string::npos) return "";
			
			size_t blockStart = content.rfind("{", pos);
			if (blockStart == std::string::npos) return "";
			
			size_t nextName = content.find("\"Name\":", pos + searchKey.length());
			if (nextName == std::string::npos) nextName = content.length();
			
			return content.substr(blockStart, nextName - blockStart);
		};

		auto findFloat = [](const std::string& block, const std::string& key, float defaultVal) -> float {
			size_t pos = block.find("\"" + key + "\":");
			if (pos != std::string::npos)
			{
				size_t start = pos + key.length() + 3;
				// Skip any whitespace or quotes
				while (start < block.length() && (block[start] == ' ' || block[start] == '"'))
					start++;
					
				size_t end = block.find_first_of(", }", start);
				try { return std::stof(block.substr(start, end - start)); } catch (...) { return defaultVal; }
			}
			return defaultVal;
		};

		auto findFloatArray3 = [](const std::string& block, const std::string& key, glm::vec3 defaultVal) -> glm::vec3 {
			size_t keyPos = block.find("\"" + key + "\":");
			if (keyPos == std::string::npos) return defaultVal;

			size_t arrayStart = block.find("[", keyPos);
			if (arrayStart == std::string::npos) return defaultVal;

			size_t arrayEnd = block.find("]", arrayStart);
			if (arrayEnd == std::string::npos) return defaultVal;

			size_t comma1 = block.find(",", arrayStart);
			if (comma1 != std::string::npos && comma1 < arrayEnd)
			{
				size_t comma2 = block.find(",", comma1 + 1);
				if (comma2 != std::string::npos && comma2 < arrayEnd)
				{
					try {
						std::string strX = block.substr(arrayStart + 1, comma1 - arrayStart - 1);
						std::string strY = block.substr(comma1 + 1, comma2 - comma1 - 1);
						std::string strZ = block.substr(comma2 + 1, arrayEnd - comma2 - 1);

						strX.erase(0, strX.find_first_not_of(" \t\r\n"));
						strX.erase(strX.find_last_not_of(" \t\r\n") + 1);
						strY.erase(0, strY.find_first_not_of(" \t\r\n"));
						strY.erase(strY.find_last_not_of(" \t\r\n") + 1);
						strZ.erase(0, strZ.find_first_not_of(" \t\r\n"));
						strZ.erase(strZ.find_last_not_of(" \t\r\n") + 1);

						float x = std::stof(strX);
						float y = std::stof(strY);
						float z = std::stof(strZ);
						return { x, y, z };
					}
					catch (...) {}
				}
			}
			return defaultVal;
		};

		auto findFloatArray2 = [](const std::string& block, const std::string& key, glm::vec2 defaultVal) -> glm::vec2 {
			// Find the specific key block
			size_t keyPos = block.find("\"" + key + "\":");
			if (keyPos == std::string::npos) return defaultVal;

			size_t arrayStart = block.find("[", keyPos);
			if (arrayStart == std::string::npos) return defaultVal;

			size_t arrayEnd = block.find("]", arrayStart);
			if (arrayEnd == std::string::npos) return defaultVal;

			size_t comma = block.find(",", arrayStart);
			if (comma != std::string::npos && comma < arrayEnd)
			{
				try {
					std::string strX = block.substr(arrayStart + 1, comma - arrayStart - 1);
					std::string strY = block.substr(comma + 1, arrayEnd - comma - 1);
					
					// Remove any leading/trailing whitespace manually just in case
					strX.erase(0, strX.find_first_not_of(" \t\r\n"));
					strX.erase(strX.find_last_not_of(" \t\r\n") + 1);
					strY.erase(0, strY.find_first_not_of(" \t\r\n"));
					strY.erase(strY.find_last_not_of(" \t\r\n") + 1);
					
					float x = std::stof(strX);
					float y = std::stof(strY);
					return { x, y };
				} catch (...) {}
			}
			return defaultVal;
		};

		// Clear scene
		scene->GetRegistry().clear();

		// Hardcode recreate the entities since our JSON is not fully dynamic yet
		// 1. Create Background
		Entity bg = scene->CreateEntity("Background");
		std::string bgBlock = extractEntityBlock("Background");
		glm::vec3 bgPos = findFloatArray3(bgBlock, "Position", {0.0f, 0.0f, -0.8f});
		glm::vec3 bgRot = findFloatArray3(bgBlock, "Rotation", {0.0f, 0.0f, 0.0f});
		glm::vec3 bgScale = findFloatArray3(bgBlock, "Scale", {50.0f, 50.0f, 1.0f});
		bg.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), bgPos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(bgRot.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(bgRot.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(bgRot.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), bgScale);
		bg.AddComponent<SpriteTransformComponent>(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

		// 2. Create Player
		Entity player = scene->CreateEntity("PlayerShip");
		auto& ptc = player.GetComponent<TransformComponent>();
		
		auto& pSprite = player.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		pSprite.Texture = Texture2D::Create("assets/texture/Ship.png");
		
		auto& pController = player.AddComponent<PlayerControllerComponent>();
		
		// Parse Player block
		std::string playerBlock = extractEntityBlock("PlayerShip");
		
		// Parse PlayerController
		pController.EnginePower = findFloat(playerBlock, "EnginePower", 1.0f);
		pController.MaxVelocityY = findFloat(playerBlock, "MaxVelocityY", 20.0f);
		pController.RotationSpeedSlow = findFloat(playerBlock, "RotationSpeedSlow", 240.0f);
		pController.RotationSpeedFast = findFloat(playerBlock, "RotationSpeedFast", 600.0f);
		pController.StartPosition = findFloatArray2(playerBlock, "StartPosition", {0.0f, 0.0f});
		pController.StartVelocity = findFloatArray2(playerBlock, "StartVelocity", {5.0f, 0.0f});

		// Parse Transform
		glm::vec3 pPos = findFloatArray3(playerBlock, "Position", {pController.StartPosition.x, pController.StartPosition.y, 0.0f});
		glm::vec3 pRotDeg = findFloatArray3(playerBlock, "Rotation", {0.0f, 0.0f, -90.0f});
		glm::vec3 pScale = findFloatArray3(playerBlock, "Scale", {1.0f, 1.3f, 1.0f});

		ptc.Transform = glm::translate(glm::mat4(1.0f), pPos) *
			            glm::rotate(glm::mat4(1.0f), glm::radians(pRotDeg.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			            glm::rotate(glm::mat4(1.0f), glm::radians(pRotDeg.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			            glm::rotate(glm::mat4(1.0f), glm::radians(pRotDeg.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			            glm::scale(glm::mat4(1.0f), pScale);
		
		player.AddComponent<SimpleParticleSystemComponent>();
		auto& prb = player.AddComponent<Rigidbody2DComponent>();
		prb.Type = Rigidbody2DComponent::BodyType::Dynamic;
		prb.FixedRotation = false;
		prb.GravityScale = 4.0f;

		auto& ptc2d = player.AddComponent<TriangleCollider2DComponent>();
		ptc2d.Size = { 1.0f, 1.0f };
		ptc2d.Offset = { 0.0f, 0.0f };
		ptc2d.Density = 1.0f;
		ptc2d.Friction = 0.3f;
		ptc2d.Restitution = 0.0f;

		// 3. Create Boundaries
		Entity ceiling = scene->CreateEntity("Ceiling");
		std::string ceilBlock = extractEntityBlock("Ceiling");
		glm::vec3 cPos = findFloatArray3(ceilBlock, "Position", {0.0f, 16.5f, 0.6f});
		glm::vec3 cRot = findFloatArray3(ceilBlock, "Rotation", {0.0f, 0.0f, 0.0f});
		glm::vec3 cScale = findFloatArray3(ceilBlock, "Scale", {10000.0f, 2.0f, 1.0f});
		ceiling.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), cPos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(cRot.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(cRot.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(cRot.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), cScale);
		ceiling.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		ceiling.AddComponent<Rigidbody2DComponent>().Type = Rigidbody2DComponent::BodyType::Static;
		ceiling.AddComponent<BoxCollider2DComponent>().Size = { 10000.0f, 2.0f };

		Entity floor = scene->CreateEntity("Floor");
		std::string floorBlock = extractEntityBlock("Floor");
		glm::vec3 fPos = findFloatArray3(floorBlock, "Position", {0.0f, -16.5f, 0.6f});
		glm::vec3 fRot = findFloatArray3(floorBlock, "Rotation", {0.0f, 0.0f, 0.0f});
		glm::vec3 fScale = findFloatArray3(floorBlock, "Scale", {10000.0f, 2.0f, 1.0f});
		floor.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), fPos) *
			glm::rotate(glm::mat4(1.0f), glm::radians(fRot.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(fRot.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(fRot.z), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), fScale);
		floor.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		floor.AddComponent<Rigidbody2DComponent>().Type = Rigidbody2DComponent::BodyType::Static;
		floor.AddComponent<BoxCollider2DComponent>().Size = { 10000.0f, 2.0f };

		// 4. Create Spawner & UI
		Entity spawner = scene->CreateEntity("ObstacleSpawner");
		auto& osc = spawner.AddComponent<ObstacleSpawnerComponent>();
		
		std::string spawnerBlock = extractEntityBlock("ObstacleSpawner");
		if (spawnerBlock.empty()) spawnerBlock = content; // Fallback to whole content if not found

		osc.MinGap = findFloat(spawnerBlock, "MinGap", 5.0f);
		osc.MaxGap = findFloat(spawnerBlock, "MaxGap", 9.0f);
		osc.SpacingX = findFloat(spawnerBlock, "SpacingX", 10.0f);
		osc.CenterVariation = findFloat(spawnerBlock, "CenterVariation", 6.0f);
		osc.MoveSpeedX = findFloat(spawnerBlock, "MoveSpeedX", -5.0f);

		spawner.AddComponent<ScoreSystemComponent>();
		auto& ui = spawner.AddComponent<UIFlowComponent>();
		ui.CurrentState = UIFlowComponent::State::MainMenu;

		// 5. Initial Pillars
		for (int i = 0; i < 20; i++)
		{
			float offset = i * osc.SpacingX + osc.StartXOffset;
			float center = RandomFloat() * (osc.CenterVariation * 2.0f) - osc.CenterVariation;
			float gap = osc.MinGap + RandomFloat() * (osc.MaxGap - osc.MinGap);
			
			float topY = 10.0f + gap * 0.5f + center;
			float botY = -10.0f - gap * 0.5f + center;

			Entity pTop = scene->CreateEntity("PillarTop");
			pTop.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(offset, topY, -0.5f)) * 
				                                                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
				                                                glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 20.0f, 1.0f));
			pTop.AddComponent<TriangleRendererComponent>(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
			auto& trb = pTop.AddComponent<Rigidbody2DComponent>();
			trb.Type = Rigidbody2DComponent::BodyType::Kinematic;
			pTop.AddComponent<TriangleCollider2DComponent>().Size = { 1.0f, 1.0f };

			Entity pBot = scene->CreateEntity("PillarBottom");
			pBot.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(offset, botY, -0.5f)) *
				                                                glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
				                                                glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 20.0f, 1.0f));
			pBot.AddComponent<TriangleRendererComponent>(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
			auto& brb = pBot.AddComponent<Rigidbody2DComponent>();
			brb.Type = Rigidbody2DComponent::BodyType::Kinematic;
			pBot.AddComponent<TriangleCollider2DComponent>().Size = { 1.0f, 1.0f };
		}

		return true;
	}

	void ParkourSystem::OnRuntimeStart(Scene* scene, bool isEditor)
	{
		// Reset score and state
		auto view = scene->GetRegistry().view<ScoreSystemComponent>();
		for (auto entity : view)
		{
			auto& score = view.get<ScoreSystemComponent>(entity);
			score.CurrentScore = 0;
		}

		auto uiView = scene->GetRegistry().view<UIFlowComponent>();
		for (auto entity : uiView)
		{
			auto& ui = uiView.get<UIFlowComponent>(entity);
			// Start in MainMenu state to let player prepare, unless we are in the editor
			ui.CurrentState = isEditor ? UIFlowComponent::State::Playing : UIFlowComponent::State::MainMenu;
		}

		auto playerView = scene->GetRegistry().view<PlayerControllerComponent, Rigidbody2DComponent, TransformComponent>();
		for (auto [entity, player, rb, tc] : playerView.each())
		{
			// Reset player position and velocity
			if (rb.RuntimeBody)
			{
				b2Body* body = (b2Body*)rb.RuntimeBody;
				body->SetTransform(b2Vec2(player.StartPosition.x, player.StartPosition.y), 0.0f);
				
				if (isEditor)
				{
					body->SetLinearVelocity(b2Vec2(player.StartVelocity.x, player.StartVelocity.y));
					body->SetAwake(true);
				}
				else
				{
					body->SetLinearVelocity(b2Vec2(0.0f, 0.0f)); // Start with 0 velocity in MainMenu
					body->SetAwake(false); // Force sleep initially
				}
			}
			
			// Try to preserve editor scale and other rotations
			glm::vec3 scale(
				glm::length(glm::vec3(tc.Transform[0])),
				glm::length(glm::vec3(tc.Transform[1])),
				glm::length(glm::vec3(tc.Transform[2]))
			);
			
			glm::mat4 rotMat(1.0f);
			if (scale.x != 0.0f) rotMat[0] = tc.Transform[0] / scale.x;
			if (scale.y != 0.0f) rotMat[1] = tc.Transform[1] / scale.y;
			if (scale.z != 0.0f) rotMat[2] = tc.Transform[2] / scale.z;

			float rotX = atan2(rotMat[1][2], rotMat[2][2]);
			float rotY = asin(-rotMat[0][2]);
			float rotZ = atan2(rotMat[0][1], rotMat[0][0]);

			// Visual rotation needs to offset by 90 degrees since 0 rotation in code means pointing right, 
			// but the ship texture/model points up naturally (-90 degrees is pointing right).
			player.VisualRotation = rotZ + glm::radians(90.0f);
			
			if (rb.RuntimeBody)
			{
				b2Body* body = (b2Body*)rb.RuntimeBody;
				body->SetTransform(b2Vec2(player.StartPosition.x, player.StartPosition.y), rotZ);
			}

			tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(player.StartPosition.x, player.StartPosition.y, tc.Transform[3].z)) *
						   glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1.0f, 0.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0.0f, 1.0f, 0.0f)) *
				           glm::rotate(glm::mat4(1.0f), rotZ, glm::vec3(0.0f, 0.0f, 1.0f)) *
				           glm::scale(glm::mat4(1.0f), scale);
		}

		// Reset pillars
		std::vector<Entity> topPillars;
		std::vector<Entity> bottomPillars;
		auto pillarView = scene->GetRegistry().view<TagComponent, Rigidbody2DComponent, TransformComponent>();
		for (auto [entity, tag, rb, tc] : pillarView.each())
		{
			if (tag.Tag.find("PillarTop") == 0) topPillars.push_back({ entity, scene });
			else if (tag.Tag.find("PillarBottom") == 0) bottomPillars.push_back({ entity, scene });
		}

		auto sortByX = [](Entity a, Entity b) {
			b2Body* bodyA = (b2Body*)a.GetComponent<Rigidbody2DComponent>().RuntimeBody;
			b2Body* bodyB = (b2Body*)b.GetComponent<Rigidbody2DComponent>().RuntimeBody;
			if (!bodyA || !bodyB) return false;
			return bodyA->GetPosition().x < bodyB->GetPosition().x;
		};
		std::sort(topPillars.begin(), topPillars.end(), sortByX);
		std::sort(bottomPillars.begin(), bottomPillars.end(), sortByX);

		ObstacleSpawnerComponent* spawner = nullptr;
		auto spawnerView = scene->GetRegistry().view<ObstacleSpawnerComponent>();
		if (spawnerView.begin() != spawnerView.end())
			spawner = &spawnerView.get<ObstacleSpawnerComponent>(*spawnerView.begin());

		if (spawner)
		{
			size_t count = std::min(topPillars.size(), bottomPillars.size());
			for (size_t i = 0; i < count; i++)
			{
				Entity topEnt = topPillars[i];
				Entity botEnt = bottomPillars[i];
				
				topEnt.GetComponent<TagComponent>().Tag = "PillarTop";
				botEnt.GetComponent<TagComponent>().Tag = "PillarBottom";

				b2Body* topBody = (b2Body*)topEnt.GetComponent<Rigidbody2DComponent>().RuntimeBody;
				b2Body* botBody = (b2Body*)botEnt.GetComponent<Rigidbody2DComponent>().RuntimeBody;
				
				if (topBody && botBody)
				{
					float offset = i * spawner->SpacingX + spawner->StartXOffset;
					float center = RandomFloat() * (spawner->CenterVariation * 2.0f) - spawner->CenterVariation;
					float gap = spawner->MinGap + RandomFloat() * (spawner->MaxGap - spawner->MinGap);
					
					float topY = 10.0f + gap * 0.5f + center;
					float botY = -10.0f - gap * 0.5f + center;

					topBody->SetTransform(b2Vec2(offset, topY), glm::radians(180.0f));
					botBody->SetTransform(b2Vec2(offset, botY), 0.0f);
					topBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					botBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
				}
			}
			spawner->CurrentXTarget = spawner->StartXOffset + (count > 0 ? (count - 1) * spawner->SpacingX : 0);
		}
	}

	void ParkourSystem::OnUpdate(Scene* scene, Timestep ts)
	{
		if (ts > 0.1f) ts = 0.1f; // Clamp large timesteps (e.g. on first frame or after lag)

		// Rainbow color logic
		static float s_Hue = 0.0f;
		s_Hue += (float)ts * 0.5f; // Adjust speed of color change
		if (s_Hue > 1.0f) s_Hue -= 1.0f;

		auto hsv2rgb = [](float h, float s, float v) -> glm::vec4 {
			int i = (int)(h * 6);
			float f = h * 6 - i;
			float p = v * (1 - s);
			float q = v * (1 - f * s);
			float t = v * (1 - (1 - f) * s);
			float r = 0, g = 0, b = 0;
			switch (i % 6) {
			case 0: r = v, g = t, b = p; break;
			case 1: r = q, g = v, b = p; break;
			case 2: r = p, g = v, b = t; break;
			case 3: r = p, g = q, b = v; break;
			case 4: r = t, g = p, b = v; break;
			case 5: r = v, g = p, b = q; break;
			}
			return { r, g, b, 1.0f };
		};
		glm::vec4 rainbowColor = hsv2rgb(s_Hue, 0.8f, 0.9f);

		// Apply rainbow color to triangles with a "glow" multiplier
		// By multiplying the RGB values > 1.0, post-processing bloom/hdr (if enabled) will make it glow.
		// Even without bloom, a high multiplier makes the color extremely saturated and bright.
		glm::vec4 glowColor = glm::vec4(rainbowColor.x * 2.5f, rainbowColor.y * 2.5f, rainbowColor.z * 2.5f, 1.0f);
		auto triView = scene->GetRegistry().view<TriangleRendererComponent>();
		for (auto e : triView)
		{
			triView.get<TriangleRendererComponent>(e).Color = glowColor;
		}

		// Apply rainbow color to sprites without texture
		auto spriteView = scene->GetRegistry().view<SpriteTransformComponent, TagComponent>();
		for (auto e : spriteView)
		{
			auto& sprite = spriteView.get<SpriteTransformComponent>(e);
			auto& tag = spriteView.get<TagComponent>(e).Tag;
			
			// Apply color if it has no texture and it is NOT the Background
			if (!sprite.Texture && tag != "Background")
			{
				sprite.Color = rainbowColor;
			}
		}

		bool isGameOver = false;
		bool isMainMenu = false;
		bool isPaused = false;
		
		auto uiView = scene->GetRegistry().view<UIFlowComponent>();
		for (auto entity : uiView)
		{
			auto& ui = uiView.get<UIFlowComponent>(entity);
			if (ui.CurrentState == UIFlowComponent::State::GameOver)
				isGameOver = true;
			else if (ui.CurrentState == UIFlowComponent::State::MainMenu)
				isMainMenu = true;
			else if (ui.CurrentState == UIFlowComponent::State::Paused)
				isPaused = true;
		}
		
		if (!isGameOver && !isPaused)
		{
			UpdatePlayer(scene, ts, isMainMenu);
			if (!isMainMenu)
			{
				UpdateObstacles(scene, ts);
				CheckCollisions(scene);
			}
		}
		else if (isPaused)
		{
			// When paused, force the rigidbodies to sleep or set their velocity to zero exactly 
			// and keep their position locked to what it is in the transform component.
			// The issue with the previous implementation was that setting velocity to 0 doesn't 
			// cancel out gravity forces being accumulated in Box2D when the world steps 
			// (or if the world continues to step).
			
			auto playerView = scene->GetRegistry().view<PlayerControllerComponent, Rigidbody2DComponent, TransformComponent>();
			for (auto [e, player, rb, tc] : playerView.each())
			{
				if (rb.RuntimeBody)
				{
					b2Body* body = (b2Body*)rb.RuntimeBody;
					body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					// Explicitly lock the physics body to the current visual transform position
					// so gravity doesn't pull it down.
					body->SetTransform(b2Vec2(tc.Transform[3].x, tc.Transform[3].y), body->GetAngle());
					body->SetAwake(false); // Force sleep
				}
			}
			
			auto obstacleView = scene->GetRegistry().view<TagComponent, Rigidbody2DComponent>();
			for (auto [e, tag, obsRb] : obstacleView.each())
			{
				if (tag.Tag.find("Pillar") == 0 && obsRb.RuntimeBody)
				{
					b2Body* obsBody = (b2Body*)obsRb.RuntimeBody;
					obsBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					obsBody->SetAwake(false); // Force sleep
				}
			}
		}
	}

	void ParkourSystem::OnRuntimeStop(Scene* scene)
	{
	}

	void ParkourSystem::UpdatePlayer(Scene* scene, Timestep ts, bool isMainMenu)
	{
		static float s_HoverTime = 0.0f;
		if (isMainMenu) s_HoverTime += (float)ts;
		
		auto view = scene->GetRegistry().view<PlayerControllerComponent, Rigidbody2DComponent, TransformComponent>();
		for (auto [entity, player, rb, tc] : view.each())
		{
			if (!rb.RuntimeBody) continue;

			b2Body* body = (b2Body*)rb.RuntimeBody;
			b2Vec2 vel = body->GetLinearVelocity();

			bool isEmitting = false;

			if (isMainMenu)
			{
				// In Main Menu, the ship hovers in place.
				vel.x = 0.0f; // No forward movement
				vel.y = 0.0f; // Force vertical velocity to 0 to counter gravity
				
				// Optional: apply hover effect without using velocity
				float hoverOffset = sin(s_HoverTime * 4.0f) * 0.5f;
				body->SetTransform(b2Vec2(player.StartPosition.x, player.StartPosition.y + hoverOffset), body->GetAngle());
				
				// Explicitly bypass physics update for this tick so gravity doesn't override our manual placement
				body->SetAwake(false);
			}
			else
			{
				body->SetAwake(true);
				// Maintain constant forward speed
				vel.x = player.StartVelocity.x;

				// Handle Input
				if (Input::IsMouseButtonPressed(0) || Input::IsKeyPressed(Key::SPACE))
				{
					vel.y = player.EnginePower * 10.0f; // basic jump
					isEmitting = true;
				}
			}

			vel.y = glm::clamp(vel.y, -player.MaxVelocityY, player.MaxVelocityY);
			body->SetLinearVelocity(vel);

			// Update visual rotation based on velocity
			if (glm::length(glm::vec2(vel.x, vel.y)) > 0.01f)
			{
				float targetAngle = atan2(vel.y, vel.x);

				float currentAngle = player.VisualRotation;
				float angleDiff = targetAngle - currentAngle;
				
				while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
				while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

				float absDiff = glm::abs(glm::degrees(angleDiff));
				float rotationSpeed = (absDiff > 1.0f) ? glm::radians(player.RotationSpeedFast) : glm::radians(player.RotationSpeedSlow);
				
				// Make upward rotation (pitch up) significantly faster and snappier
				if (angleDiff > 0.0f)
				{
					rotationSpeed *= 2.0f;
				}

				float step = rotationSpeed * (float)ts;
				if (glm::abs(angleDiff) < step)
					currentAngle = targetAngle;
				else
					currentAngle += glm::sign(angleDiff) * step;

				player.VisualRotation = currentAngle;
			}

			// In Box2D, rotation is just angle in radians.
			// The physical triangle points UP natively, but angle 0 means flying RIGHT.
			// We subtract 90 degrees so the tip points perfectly in the flight direction.
			body->SetTransform(body->GetPosition(), player.VisualRotation - glm::radians(90.0f));
			
			// Always sync physics body position back to TransformComponent, 
			// and apply base scale/rotation so Renderer2D displays correctly.
			glm::vec3 scale(
				glm::length(glm::vec3(tc.Transform[0])),
				glm::length(glm::vec3(tc.Transform[1])),
				glm::length(glm::vec3(tc.Transform[2]))
			);
			glm::mat4 rotMat(1.0f);
			if (scale.x != 0.0f) rotMat[0] = tc.Transform[0] / scale.x;
			if (scale.y != 0.0f) rotMat[1] = tc.Transform[1] / scale.y;
			if (scale.z != 0.0f) rotMat[2] = tc.Transform[2] / scale.z;
			float rotX = atan2(rotMat[1][2], rotMat[2][2]);
			float rotY = asin(-rotMat[0][2]);

			tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(body->GetPosition().x, body->GetPosition().y, tc.Transform[3].z)) *
						   glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1.0f, 0.0f, 0.0f)) *
						   glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0.0f, 1.0f, 0.0f)) *
				           glm::rotate(glm::mat4(1.0f), player.VisualRotation - glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
				           glm::scale(glm::mat4(1.0f), scale);

			// Update Particle System
			Entity e{entity, scene};
			if (e.HasComponent<SimpleParticleSystemComponent>())
			{
				auto& psc = e.GetComponent<SimpleParticleSystemComponent>();
				if (isEmitting)
				{
					glm::vec2 emissionPoint = { 0.0f, -0.6f };
					float rotation = player.VisualRotation - glm::radians(90.0f);
					glm::vec4 rotated = glm::rotate(glm::mat4(1.0f), rotation, {0.0f,0.0f,1.0f}) * glm::vec4(emissionPoint,0.0f,1.0f);
					
					SimpleParticleProps props = player.EngineParticle;
					props.Position = glm::vec2(body->GetPosition().x, body->GetPosition().y) + glm::vec2(rotated.x, rotated.y);
					props.Velocity.y = -vel.y * 0.2f - 0.2f;

					// Emit
					auto& particle = psc.ParticlePool[psc.PoolIndex];
					particle.Active = true;
					particle.Position = props.Position;
					particle.Rotation = RandomFloat() * 2.0f * glm::pi<float>();

					particle.Velocity = props.Velocity;
					particle.Velocity.x += props.VelocityVariation.x * (RandomFloat() - 0.5f);
					particle.Velocity.y += props.VelocityVariation.y * (RandomFloat() - 0.5f);

					particle.ColorBegin = props.ColorBegin;
					particle.ColorEnd = props.ColorEnd;
					particle.SizeBegin = props.SizeBegin + props.SizeVariation * (RandomFloat() - 0.5f);
					particle.SizeEnd = props.SizeEnd;
					particle.LifeTime = props.LifeTime;
					particle.LifeRemaining = props.LifeTime;

					psc.PoolIndex = (psc.PoolIndex + 1) % psc.ParticlePool.size();
				}

				// Update logic
				for (auto& particle : psc.ParticlePool)
				{
					if (!particle.Active) continue;
					if (particle.LifeRemaining <= 0.0f)
					{
						particle.Active = false;
						continue;
					}
					particle.LifeRemaining -= ts;
					particle.Position += particle.Velocity * (float)ts;
					particle.Position += 0.01f * (float)ts;
				}
			}
		}
	}

	void ParkourSystem::UpdateObstacles(Scene* scene, Timestep ts)
	{
		// Find spawner
		ObstacleSpawnerComponent* spawner = nullptr;
		auto spawnerView = scene->GetRegistry().view<ObstacleSpawnerComponent>();
		if (spawnerView.begin() != spawnerView.end())
		{
			spawner = &spawnerView.get<ObstacleSpawnerComponent>(*spawnerView.begin());
		}

		if (!spawner) return;

		// Get player position to determine screen left
		float screenLeft = -20.0f;
		auto playerView = scene->GetRegistry().view<PlayerControllerComponent, TransformComponent>();
		for (auto entity : playerView)
		{
			screenLeft = playerView.get<TransformComponent>(entity).Transform[3].x - 20.0f;
			break;
		}

		// Update pillars (identified by tag "PillarTop" and "PillarBottom")
		// Find all pillars
		std::vector<Entity> topPillars;
		std::vector<Entity> bottomPillars;
		auto pillarView = scene->GetRegistry().view<TagComponent, Rigidbody2DComponent, TransformComponent>();
		for (auto [entity, tag, rb, tc] : pillarView.each())
		{
			if (tag.Tag.find("PillarTop") == 0) topPillars.push_back({ entity, scene });
			else if (tag.Tag.find("PillarBottom") == 0) bottomPillars.push_back({ entity, scene });
		}

		// Sort by X position to pair them up
		auto sortByX = [](Entity a, Entity b) {
			b2Body* bodyA = (b2Body*)a.GetComponent<Rigidbody2DComponent>().RuntimeBody;
			b2Body* bodyB = (b2Body*)b.GetComponent<Rigidbody2DComponent>().RuntimeBody;
			if (!bodyA || !bodyB) return false;
			return bodyA->GetPosition().x < bodyB->GetPosition().x;
		};
		std::sort(topPillars.begin(), topPillars.end(), sortByX);
		std::sort(bottomPillars.begin(), bottomPillars.end(), sortByX);

		float maxX = spawner->CurrentXTarget;
		for (auto topEnt : topPillars)
		{
			b2Body* body = (b2Body*)topEnt.GetComponent<Rigidbody2DComponent>().RuntimeBody;
			if (body && body->GetPosition().x > maxX)
				maxX = body->GetPosition().x;
		}

		size_t count = std::min(topPillars.size(), bottomPillars.size());
		for (size_t i = 0; i < count; i++)
		{
			Entity topEnt = topPillars[i];
			Entity botEnt = bottomPillars[i];
			auto& topTag = topEnt.GetComponent<TagComponent>().Tag;
			auto& topRb = topEnt.GetComponent<Rigidbody2DComponent>();
			auto& topTc = topEnt.GetComponent<TransformComponent>();
			auto& botRb = botEnt.GetComponent<Rigidbody2DComponent>();
			auto& botTc = botEnt.GetComponent<TransformComponent>();

			b2Body* topBody = (b2Body*)topRb.RuntimeBody;
			b2Body* botBody = (b2Body*)botRb.RuntimeBody;
			if (!topBody || !botBody) continue;

			topBody->SetLinearVelocity(b2Vec2(spawner->MoveSpeedX, 0.0f));
			botBody->SetLinearVelocity(b2Vec2(spawner->MoveSpeedX, 0.0f));

			float x = topBody->GetPosition().x;

			// Get player X position for scoring and background following
			float playerX = 0.0f;
			auto playerView = scene->GetRegistry().view<PlayerControllerComponent, Rigidbody2DComponent>();
			for (auto pEnt : playerView)
			{
				auto& prb = playerView.get<Rigidbody2DComponent>(pEnt);
				if (prb.RuntimeBody)
				{
					b2Body* pBody = (b2Body*)prb.RuntimeBody;
					playerX = pBody->GetPosition().x;
				}
				break;
			}

			// Background parallax / fast follow effect
			auto bgView = scene->GetRegistry().view<TagComponent, TransformComponent>();
			for (auto bgEnt : bgView)
			{
				auto& tag = bgView.get<TagComponent>(bgEnt).Tag;
				if (tag == "Background")
				{
					auto& bgTc = bgView.get<TransformComponent>(bgEnt);
					// Background closely follows player X, making it look much faster
					// A multiplier of 0.8f makes it follow at 80% of player speed,
					// meaning it scrolls past the player 20% relative, which looks faster than static.
					bgTc.Transform[3].x = playerX * 0.8f;
				}
			}

			// Score logic: If pillar passes behind the player
			if (x < playerX && topTag == "PillarTop") 
			{
				topTag = "PillarTop_Scored";
				auto uiView = scene->GetRegistry().view<ScoreSystemComponent>();
				for (auto uiEnt : uiView)
					uiView.get<ScoreSystemComponent>(uiEnt).CurrentScore++;
			}

			if (x < screenLeft)
			{
				// Reposition
				topTag = "PillarTop"; // reset tag
				float newX = maxX + spawner->SpacingX;
				
				// Ensure newX is outside visible range (screenLeft + ~45)
				float minX = screenLeft + 45.0f;
				if (newX < minX) newX = minX;

				float center = RandomFloat() * (spawner->CenterVariation * 2.0f) - spawner->CenterVariation;
				float gap = spawner->MinGap + RandomFloat() * (spawner->MaxGap - spawner->MinGap);
				if (gap < spawner->MinGap) gap = spawner->MinGap;

				float newTopY = 10.0f + gap * 0.5f + center;
				float newBotY = -10.0f - gap * 0.5f + center;

				topBody->SetTransform(b2Vec2(newX, newTopY), glm::radians(180.0f));
				botBody->SetTransform(b2Vec2(newX, newBotY), 0.0f);
				
				maxX = newX; // Update maxX for the next recycled pillar in the same frame
				spawner->CurrentXTarget = newX; // Update target

				// Also update visual transform
				glm::vec3 topScale(
					glm::length(glm::vec3(topTc.Transform[0])),
					glm::length(glm::vec3(topTc.Transform[1])),
					glm::length(glm::vec3(topTc.Transform[2]))
				);
				glm::vec3 botScale(
					glm::length(glm::vec3(botTc.Transform[0])),
					glm::length(glm::vec3(botTc.Transform[1])),
					glm::length(glm::vec3(botTc.Transform[2]))
				);

				topTc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(newX, newTopY, topTc.Transform[3].z)) *
					           glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
					           glm::scale(glm::mat4(1.0f), topScale);
				
				botTc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(newX, newBotY, botTc.Transform[3].z)) *
					           glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
					           glm::scale(glm::mat4(1.0f), botScale);
			}
		}
	}

	void ParkourSystem::CheckCollisions(Scene* scene)
	{
		// In a full implementation, we'd hook into Box2D's contact listener.
		// Since we are rebuilding without modifying engine core heavily, we check overlaps or rely on Box2D physics.
		// The engine's Physics2D handles the actual rigid body collision.
		// We can poll positions to see if player fell out of bounds or hit an obstacle.
		
		auto playerView = scene->GetRegistry().view<PlayerControllerComponent, TransformComponent, Rigidbody2DComponent>();
		for (auto entity : playerView)
		{
			auto& tc = playerView.get<TransformComponent>(entity);
			auto& rb = playerView.get<Rigidbody2DComponent>(entity);
			float y = tc.Transform[3].y;
			bool gameOver = false;

			if (y > 15.5f || y < -15.5f)
			{
				gameOver = true;
				QL_CORE_INFO("Player out of bounds. GameOver.");
			}

			// Check actual physical contacts
			if (rb.RuntimeBody)
			{
				b2Body* body = (b2Body*)rb.RuntimeBody;
				for (b2ContactEdge* ce = body->GetContactList(); ce; ce = ce->next)
				{
					if (ce->contact->IsTouching())
					{
						gameOver = true;
						QL_CORE_INFO("Player collided with obstacle. GameOver.");
						break;
					}
				}
			}

			if (gameOver)
			{
				// Zero out player's X velocity to stop moving forward
				if (rb.RuntimeBody)
				{
					b2Body* body = (b2Body*)rb.RuntimeBody;
					b2Vec2 currentVel = body->GetLinearVelocity();
					body->SetLinearVelocity(b2Vec2(0.0f, currentVel.y));
				}

				// Stop all pillars from moving
				auto obstacleView = scene->GetRegistry().view<TagComponent, Rigidbody2DComponent>();
				for (auto [e, tag, obsRb] : obstacleView.each())
				{
					if (tag.Tag.find("Pillar") == 0 && obsRb.RuntimeBody)
					{
						b2Body* obsBody = (b2Body*)obsRb.RuntimeBody;
						obsBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
					}
				}

				// Game over condition met
				auto uiView = scene->GetRegistry().view<UIFlowComponent>();
				for (auto uiEnt : uiView)
				{
					uiView.get<UIFlowComponent>(uiEnt).CurrentState = UIFlowComponent::State::GameOver;
				}
			}
		}
	}

	void ParkourSystem::BuildScene(Scene* scene)
	{
		// 0. Create Background
		Entity bg = scene->CreateEntity("Background");
		bg.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.8f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 1.0f));
		bg.AddComponent<SpriteTransformComponent>(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

		// 1. Create Player
		Entity player = scene->CreateEntity("PlayerShip");
		auto& ptc = player.GetComponent<TransformComponent>();
		// Initialize the player size (1.0, 1.3) and a base rotation of -90 degrees so it faces forward
		ptc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
			            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.3f, 1.0f));
		
		auto& pSprite = player.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		pSprite.Texture = Texture2D::Create("assets/texture/Ship.png");
		
		auto& pController = player.AddComponent<PlayerControllerComponent>();
		player.AddComponent<SimpleParticleSystemComponent>();
		auto& prb = player.AddComponent<Rigidbody2DComponent>();
		prb.Type = Rigidbody2DComponent::BodyType::Dynamic;
		prb.FixedRotation = false;
		prb.GravityScale = 4.0f;

		auto& ptc2d = player.AddComponent<TriangleCollider2DComponent>();
		ptc2d.Size = { 1.0f, 1.0f }; // The scale (1.0, 1.3) will automatically be applied by Physics2D
		ptc2d.Offset = { 0.0f, 0.0f }; // No offset needed, geometric center aligns perfectly
		ptc2d.Density = 1.0f;
		ptc2d.Friction = 0.3f;
		ptc2d.Restitution = 0.0f;

		// 2. Create Boundaries
		Entity ceiling = scene->CreateEntity("Ceiling");
		// The bottom edge of the ceiling needs to be exactly at y=15.5f. If the BoxCollider2D height is 2.0f, the center should be 16.5f.
		ceiling.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 16.5f, 0.6f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10000.0f, 2.0f, 1.0f));
		ceiling.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // We'll change the color dynamically later
		ceiling.AddComponent<Rigidbody2DComponent>().Type = Rigidbody2DComponent::BodyType::Static;
		ceiling.AddComponent<BoxCollider2DComponent>().Size = { 10000.0f, 2.0f };

		Entity floor = scene->CreateEntity("Floor");
		// The top edge of the floor needs to be exactly at y=-15.5f. If the BoxCollider2D height is 2.0f, the center should be -16.5f.
		floor.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -16.5f, 0.6f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10000.0f, 2.0f, 1.0f));
		floor.AddComponent<SpriteTransformComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // We'll change the color dynamically later
		floor.AddComponent<Rigidbody2DComponent>().Type = Rigidbody2DComponent::BodyType::Static;
		floor.AddComponent<BoxCollider2DComponent>().Size = { 10000.0f, 2.0f };

		// 3. Create Spawner & UI
		Entity spawner = scene->CreateEntity("ObstacleSpawner");
		spawner.AddComponent<ObstacleSpawnerComponent>();
		spawner.AddComponent<ScoreSystemComponent>();
		auto& ui = spawner.AddComponent<UIFlowComponent>();
		// Make sure the initial scene is saved as MainMenu state to let player prepare
		ui.CurrentState = UIFlowComponent::State::MainMenu;

		// 4. Initial Pillars
		for (int i = 0; i < 20; i++)
		{
			float offset = i * 10.0f + 20.0f;
			float center = RandomFloat() * 12.0f - 6.0f;
			float gap = 5.0f + RandomFloat() * (9.0f - 5.0f); // Reduced gap for higher difficulty
			
			float topY = 10.0f + gap * 0.5f + center;
			float botY = -10.0f - gap * 0.5f + center;

			Entity pTop = scene->CreateEntity("PillarTop");
			pTop.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(offset, topY, -0.5f)) * 
				                                                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
				                                                glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 20.0f, 1.0f));
			pTop.AddComponent<TriangleRendererComponent>(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
			auto& trb = pTop.AddComponent<Rigidbody2DComponent>();
			trb.Type = Rigidbody2DComponent::BodyType::Kinematic;
			pTop.AddComponent<TriangleCollider2DComponent>().Size = { 1.0f, 1.0f };

			Entity pBot = scene->CreateEntity("PillarBottom");
			pBot.GetComponent<TransformComponent>().Transform = glm::translate(glm::mat4(1.0f), glm::vec3(offset, botY, -0.5f)) *
				                                                glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
				                                                glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 20.0f, 1.0f));
			pBot.AddComponent<TriangleRendererComponent>(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
			auto& brb = pBot.AddComponent<Rigidbody2DComponent>();
			brb.Type = Rigidbody2DComponent::BodyType::Kinematic;
			pBot.AddComponent<TriangleCollider2DComponent>().Size = { 1.0f, 1.0f };
		}
	}

	void ParkourSystem::SaveScene(Scene* scene, const std::string& filepath)
	{
		std::stringstream ss;
		ss << "{\n";
		ss << "  \"Scene\": \"ParkourGameTemplate\",\n";
		ss << "  \"Entities\": [\n";

		auto view = scene->GetRegistry().view<TagComponent>();
		int count = 0;
		for (auto entity : view)
		{
			if (count > 0) ss << ",\n";
			
			auto& tag = view.get<TagComponent>(entity).Tag;
			ss << "    {\n";
			ss << "      \"Name\": \"" << tag << "\",\n";
			ss << "      \"Components\": {\n";
			
			Entity e{ entity, scene };
			bool hasPrevComp = false;

			if (e.HasComponent<TransformComponent>())
			{
				auto& tc = e.GetComponent<TransformComponent>();
				
				glm::vec3 scale(
					glm::length(glm::vec3(tc.Transform[0])),
					glm::length(glm::vec3(tc.Transform[1])),
					glm::length(glm::vec3(tc.Transform[2]))
				);
				
				glm::mat4 rotMat(1.0f);
				if (scale.x != 0.0f) rotMat[0] = tc.Transform[0] / scale.x;
				if (scale.y != 0.0f) rotMat[1] = tc.Transform[1] / scale.y;
				if (scale.z != 0.0f) rotMat[2] = tc.Transform[2] / scale.z;

				float rotX = atan2(rotMat[1][2], rotMat[2][2]);
				float rotY = asin(-rotMat[0][2]);
				float rotZ = atan2(rotMat[0][1], rotMat[0][0]);
				
				glm::vec3 rotDeg = glm::degrees(glm::vec3(rotX, rotY, rotZ));

				ss << "        \"Transform\": { \"Position\": [" << tc.Transform[3].x << ", " << tc.Transform[3].y << ", " << tc.Transform[3].z << "], ";
				ss << "\"Rotation\": [" << rotDeg.x << ", " << rotDeg.y << ", " << rotDeg.z << "], ";
				ss << "\"Scale\": [" << scale.x << ", " << scale.y << ", " << scale.z << "] }";
				hasPrevComp = true;
			}

			if (e.HasComponent<PlayerControllerComponent>())
			{
				if (hasPrevComp) ss << ",\n";
				auto& pc = e.GetComponent<PlayerControllerComponent>();
				ss << "        \"PlayerController\": { \"EnginePower\": " << pc.EnginePower << ", \"MaxVelocityY\": " << pc.MaxVelocityY << ", \"StartPosition\": [" << pc.StartPosition.x << ", " << pc.StartPosition.y << "], \"StartVelocity\": [" << pc.StartVelocity.x << ", " << pc.StartVelocity.y << "], \"RotationSpeedSlow\": " << pc.RotationSpeedSlow << ", \"RotationSpeedFast\": " << pc.RotationSpeedFast << " }";
				hasPrevComp = true;
			}

			if (e.HasComponent<ObstacleSpawnerComponent>())
			{
				if (hasPrevComp) ss << ",\n";
				auto& osc = e.GetComponent<ObstacleSpawnerComponent>();
				ss << "        \"ObstacleSpawner\": { \"MinGap\": " << osc.MinGap << ", \"MaxGap\": " << osc.MaxGap << ", \"SpacingX\": " << osc.SpacingX << ", \"CenterVariation\": " << osc.CenterVariation << ", \"MoveSpeedX\": " << osc.MoveSpeedX << " }";
				hasPrevComp = true;
			}

			ss << "\n      }\n";
			ss << "    }";
			count++;
		}
		ss << "\n  ]\n}\n";

		std::ofstream out(filepath);
		if (out.is_open())
		{
			out << ss.str();
			out.close();
		}
	}

}
