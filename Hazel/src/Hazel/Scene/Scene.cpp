#include "hzpch.h"
#include "Scene.h"

#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Scene/Entity.h"

#include "glm/glm.hpp"

namespace Hazel
{

	Scene::Scene()
	{
		//struct TransformComponent {
		//	glm::mat4 Transform;

		//	TransformComponent() = default;
		//	TransformComponent(const glm::mat4& trans)
		//		:Transform(trans) {
		//	}
		//	TransformComponent(const TransformComponent&) = default;

		//	operator glm::mat4& () { return Transform; }
		//	operator const glm::mat4& () const { return Transform; }
		//};

		//struct MeshComponent
		//{
		//	int ID;

		//	MeshComponent() = default;
		//};

		//// 创建实体
		//entt::entity Entity = m_Registry.create();
		//// 给实体添加组件
		//m_Registry.emplace<TransformComponent>(Entity, glm::mat4(1.0f));
		//// 获取实体的某组件
		//if (m_Registry.try_get<TransformComponent>(Entity)) {

		//}
		//// 获取所有具有某组件的实体
		//auto view = m_Registry.view<TransformComponent>();
		//for (auto entity : view)
		//{
		//	auto& transform = view.get<TransformComponent>(entity);// 在视图中检索transform组件，性能高（视图只包含了特定组件的实体）
		//	//auto transform = m_Registry.get<TransformComponent>(entity);// 在注册表中检索transform组件，性能弱（注册表包含所有实体）
		//}

		//auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		//for (auto entity : group)
		//{
		//	auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
		//}
		//// 当实体的TransformComponent或MeshComponent组件被创建、销毁、或者更新时，组中的实体会被通知
		//group.each([](entt::entity entity, TransformComponent& transform, MeshComponent& mesh) {
		//	// 在这里处理实体的更改
		//	});
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>(glm::mat4{ 1.0f });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unnamed Entity" : name;

		return entity;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Do some rendering

		// 在所有含有 TransformComponent 的实体中搜集含有 sprite 的实体，group 返回一个类似注册表的实体集合
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
		for (auto entity : group) {
			auto& [transform, color] = group.get<TransformComponent, SpriteComponent>(entity);

			Renderer2D::DrawQuad(transform, color);
		}
	}


}