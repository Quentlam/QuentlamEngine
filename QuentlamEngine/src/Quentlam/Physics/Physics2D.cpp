#include "qlpch.h"
#include "Physics2D.h"
#include "Quentlam/Scene/Components.h"
#include "Quentlam/Scene/Entity.h"

// Forward declaration since we can't include ParkourComponents.h from QuentlamEngine core easily
namespace Quentlam {
	struct PlayerControllerComponent;
}

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

namespace Quentlam {

	static b2World* s_PhysicsWorld = nullptr;
	static Scene* s_RuntimeScene = nullptr;

	namespace
	{
		void ResetRuntimeHandles(Scene* scene)
		{
			if (!scene)
				return;

			auto rigidbodyView = scene->GetRegistry().view<Rigidbody2DComponent>();
			for (auto e : rigidbodyView)
				rigidbodyView.get<Rigidbody2DComponent>(e).RuntimeBody = nullptr;

			auto colliderView = scene->GetRegistry().view<BoxCollider2DComponent>();
			for (auto e : colliderView)
				colliderView.get<BoxCollider2DComponent>(e).RuntimeFixture = nullptr;
		}
	}

	bool Physics2D::OnRuntimeStart(Scene* scene)
	{
		if (!scene)
		{
			QL_CORE_ERROR("Physics2D runtime start failed: scene is null.");
			return false;
		}

		if (s_PhysicsWorld && s_RuntimeScene == scene)
			return true;

		if (s_PhysicsWorld)
			OnRuntimeStop(s_RuntimeScene ? s_RuntimeScene : scene);

		ResetRuntimeHandles(scene);

		s_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		s_RuntimeScene = scene;

		auto view = scene->GetRegistry().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = rb2d.Type == Rigidbody2DComponent::BodyType::Static ? b2_staticBody : 
				(rb2d.Type == Rigidbody2DComponent::BodyType::Dynamic ? b2_dynamicBody : b2_kinematicBody);
			
			// Extract position and rotation from transform matrix
			glm::vec3 position = transform.Transform[3];
			bodyDef.position.Set(position.x, position.y);
			
			// Very basic angle extraction for 2D (assuming Z-axis rotation)
			bodyDef.angle = atan2(transform.Transform[0][1], transform.Transform[0][0]);
			bodyDef.fixedRotation = rb2d.FixedRotation;
			bodyDef.gravityScale = rb2d.GravityScale;

			b2Body* body = s_PhysicsWorld->CreateBody(&bodyDef);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				
				// Scale should be extracted from transform too, simplify for now
				glm::vec3 scale(
					glm::length(glm::vec3(transform.Transform[0])),
					glm::length(glm::vec3(transform.Transform[1])),
					glm::length(glm::vec3(transform.Transform[2]))
				);

				boxShape.SetAsBox(bc2d.Size.x * scale.x * 0.5f, bc2d.Size.y * scale.y * 0.5f, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				bc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<TriangleCollider2DComponent>())
			{
				auto& tc2d = entity.GetComponent<TriangleCollider2DComponent>();

				b2PolygonShape triangleShape;
				
				glm::vec3 scale(
					glm::length(glm::vec3(transform.Transform[0])),
					glm::length(glm::vec3(transform.Transform[1])),
					glm::length(glm::vec3(transform.Transform[2]))
				);

				b2Vec2 vertices[3];
				vertices[0].Set(tc2d.Offset.x - tc2d.Size.x * scale.x * 0.5f, tc2d.Offset.y - tc2d.Size.y * scale.y * 0.5f); // Bottom-left
				vertices[1].Set(tc2d.Offset.x + tc2d.Size.x * scale.x * 0.5f, tc2d.Offset.y - tc2d.Size.y * scale.y * 0.5f); // Bottom-right
				vertices[2].Set(tc2d.Offset.x, tc2d.Offset.y + tc2d.Size.y * scale.y * 0.5f); // Top
				triangleShape.Set(vertices, 3);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &triangleShape;
				fixtureDef.density = tc2d.Density;
				fixtureDef.friction = tc2d.Friction;
				fixtureDef.restitution = tc2d.Restitution;
				fixtureDef.restitutionThreshold = tc2d.RestitutionThreshold;
				tc2d.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}
		}

		return true;
	}

	void Physics2D::OnRuntimeStop(Scene* scene)
	{
		Scene* sceneToReset = scene ? scene : s_RuntimeScene;
		ResetRuntimeHandles(sceneToReset);

		delete s_PhysicsWorld;
		s_PhysicsWorld = nullptr;
		s_RuntimeScene = nullptr;
	}

	void Physics2D::OnUpdate(Scene* scene, Timestep ts)
	{
		if (!s_PhysicsWorld || ts == 0.0f)
			return;
			
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		s_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		auto view = scene->GetRegistry().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			if (!body) continue;

			const auto& position = body->GetPosition();
			
			// Update rotation if needed (simplified)
			float angle = body->GetAngle();
			
			// DO NOT overwrite the transform component if the entity is managed by a specific controller 
			// like PlayerControllerComponent which manually handles its visual rotation/scale.
			// Actually we can't cleanly check PlayerControllerComponent here without coupling. 
			// Instead, just don't reset the scale! Keep the scale that was already in the transform.
			glm::vec3 scale(
				glm::length(glm::vec3(transform.Transform[0])),
				glm::length(glm::vec3(transform.Transform[1])),
				glm::length(glm::vec3(transform.Transform[2]))
			);
			
			// We only want to update the translation and possibly rotation
			transform.Transform[3][0] = position.x;
			transform.Transform[3][1] = position.y;
			
			// If it's a dynamic body and NOT fixed rotation, we could update the rotation.
			// For our parkour game, the player's rotation is driven by ParkourSystem, 
			// but ParkourSystem already calls body->SetTransform(..., angle) overriding Physics2D.
			// So we can just leave this as is, but we must reconstruct the matrix properly 
			// using the CURRENT rotation if we don't want to touch it, or using body angle if we do.
			// To avoid breaking the player's custom VisualRotation logic, we won't apply the body's angle 
			// directly to the transform if FixedRotation is false but we are using custom rotation.
			// Actually, ParkourSystem sets the angle on the body, so body->GetAngle() IS the visual rotation!
			transform.Transform[0] = glm::vec4(cos(angle) * scale.x, sin(angle) * scale.x, 0.0f, 0.0f);
			transform.Transform[1] = glm::vec4(-sin(angle) * scale.y, cos(angle) * scale.y, 0.0f, 0.0f);
			transform.Transform[2] = glm::vec4(0.0f, 0.0f, scale.z, 0.0f);
		}
	}

}
