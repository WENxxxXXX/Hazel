#include "hzpch.h"
#include "Scene.h"

#include "glm/glm.hpp"

namespace Hazel
{

	static void DoMath(const glm::mat4& transform)
	{
		// Do some math
	}

	Scene::Scene()
	{
		struct TransformComponent {
			glm::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const glm::mat4& trans)
				:Transform(trans) {
			}
			TransformComponent(const TransformComponent&) = default;

			operator glm::mat4& () { return Transform; }
			operator const glm::mat4& () const { return Transform; }
		};

		struct MeshComponent
		{
			int ID;

			MeshComponent() = default;
		};

		// 创建实体
		entt::entity Entity = m_Registry.create();
		// 给实体添加组件
		m_Registry.emplace<TransformComponent>(Entity, glm::mat4(1.0f));
		// 获取实体的某组件
		if (m_Registry.try_get<TransformComponent>(Entity)) {

		}
		// 获取所有具有某组件的实体
		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			auto& transform = view.get<TransformComponent>(entity);// 在视图中检索transform组件，性能高（视图只包含了特定组件的实体）
			//auto transform = m_Registry.get<TransformComponent>(entity);// 在注册表中检索transform组件，性能弱（注册表包含所有实体）
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto entity : group)
		{
			auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
		}
		// 当实体的TransformComponent或MeshComponent组件被创建、销毁、或者更新时，组中的实体会被通知
		group.each([](entt::entity entity, TransformComponent& transform, MeshComponent& mesh) {
			// 在这里处理实体的更改
			});
	}

	Scene::~Scene()
	{
	}
}