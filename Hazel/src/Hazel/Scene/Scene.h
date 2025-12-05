#pragma once

#include "entt.hpp"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"
#include <Hazel/Renderer/Model.h>
#include "Hazel/Renderer/FrameBuffer.h"

class b2World;

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(UUID id, const std::string& name = "");
		void DestroyEntity(Entity& entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera camera, Ref<FrameBuffer> framebuffer);
		void OnScript(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		void DuplicateEntity(Entity& srcEntity);

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