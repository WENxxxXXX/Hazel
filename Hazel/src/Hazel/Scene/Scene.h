#pragma once

#include "entt.hpp"
#include "Hazel/Core/Timestep.h"

namespace Hazel
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity CreateEntity(const std::string& name = "");
	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth, m_ViewportHeight;			// maybe useful

		friend class Entity;
	};


}