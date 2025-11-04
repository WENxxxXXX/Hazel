#pragma once

#include "Hazel/Scene/Entity.h"

namespace Hazel {

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() = default;

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
	protected:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(Timestep ts) = 0;
	};

	// -----------------------------------------------------------------------------------
	class ScriptCameraController : public ScriptableEntity
	{
	public:
		void OnCreate() override;
		void OnDestroy() override;
		void OnUpdate(Timestep ts) override;
	};
}