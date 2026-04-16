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
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteTransformComponent>);


		//这里使用get会没办法修改组件的值,因为get返回的是一个临时的右值,所以不能引用
		//for (auto entity : group)
		//{
		//	auto& [transform, sprite] = group.get<TransformComponent, SpriteTransformComponent>(entity);

		//	Renderer2D::DrawQuad(transform, sprite.Color);
		//}

		//2. 使用 each() 自动解包！each循环里的每一个子元素都是引用，所以可以修改
		for (auto [entity, transform, sprite] : group.each())
		{
			// 这里的 transform 就是 TransformComponent&，sprite 就是 SpriteTransformComponent&
			Renderer2D::DrawQuad(transform.Transform, sprite.Color);
		}

	}
}