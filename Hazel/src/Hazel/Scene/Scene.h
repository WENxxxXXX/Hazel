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

		Entity CreateEntity(const std::string& name = "");
	private:
		entt::registry m_Registry;

		friend class Entity;
	};


}