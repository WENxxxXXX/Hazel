#pragma once

#include "entt.hpp"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

class b2World;//前向声明（针对Hazel作用域以外的外部库：Box2D）

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(UUID id, const std::string& name = "");
		void DestroyEntity(Entity& entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera camera);
		void OnScript(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCamera();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		b2World* m_PhysicsWorld = nullptr;

		uint32_t m_ViewportWidth, m_ViewportHeight;// We use it in Scene::OnComponentAdded<CameraComponent>

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};


}