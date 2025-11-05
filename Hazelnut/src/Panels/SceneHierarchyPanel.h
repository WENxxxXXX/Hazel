#pragma once

#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel
{

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		// Must be used to Init Context!!! For updating Active Scene(which you are using)
		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
	private:
		// Active scene is the context, Entity (which is being used) is the selectionContext
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};


}