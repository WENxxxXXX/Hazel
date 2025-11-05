#pragma once

#include "entt.hpp"

#include "Hazel/Scene/Scene.h"

namespace Hazel
{

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);
		Entity(const Entity& other) = default;

		~Entity() = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			HZ_CORE_ASSERT(!HasComponent<T>(), "This Entity already has component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "This Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "This Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template <typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		// Where have i used it
		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other) const 
		{ 
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; 
		}
		bool operator!=(const Entity& other) const { return !(*this == other); }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};



}