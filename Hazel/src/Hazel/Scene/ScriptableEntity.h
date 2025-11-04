#pragma once

#include "Hazel/Scene/Entity.h"

namespace Hazel {

	class ScriptableEntity
	{
	public:
		template<typename T>
		T& GetComponent()
		{
			return m_ScriptableEntity.GetComponent<T>();
		}
	private:
		Entity m_ScriptableEntity;
		friend class Scene;
		// check the "m_ScriptableEntity.GetComponent();",
		// The member function which from "Entity" will use class "Scene", 
		// so we declare Scene as a friend class.
	};


}