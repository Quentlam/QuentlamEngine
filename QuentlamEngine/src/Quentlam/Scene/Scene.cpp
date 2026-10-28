#include "qlpch.h"

#include "Scene.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Entity.h"
#include "Components.h"

#include <glm/glm.hpp>


namespace Quentlam
{
	static void DoMath(const glm::mat4& transform)
	{


	};

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{


	};



	Scene::Scene()
	{



	};


	Scene::~Scene()
	{



	};

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create() , this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	};

	void Scene::OnUpdate(Timestep ts)
	{
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			for (auto [entity, transform, camera] : m_Registry.view<TransformComponent, CameraComponent>().each())
			{
				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = &transform.Transform;
					break;
				}
			}
		}


		if (mainCamera)
		{
			Renderer2D::BeginScene(mainCamera->GetProjection(),*cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto [entity, transform, sprite] : group.each())
			{
				Renderer2D::DrawQuad(transform.Transform, sprite.Color);
			}

			Renderer2D::EndScene();
		}



	}
}