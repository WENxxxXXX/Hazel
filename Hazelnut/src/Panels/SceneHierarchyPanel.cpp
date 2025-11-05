#include "hzpch.h"
#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Component.h"

#include <imgui/imgui.h>

namespace Hazel
{

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.view<entt::entity>().each(
			[&](auto entityID)
			{
				// So we can use member function from class Entity
				Entity entity{ entityID, m_Context.get() };
				DrawEntityNode(entity);
			}
		);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		// Draw first TreeNode
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
			((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		if (opened)
		{
			// Draw nested TreeNode
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			bool opened = ImGui::TreeNodeEx((void*)666999, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();

			ImGui::TreePop();
		}
	}

}