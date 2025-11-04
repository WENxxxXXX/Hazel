#pragma once

#include "glm/glm.hpp"

#include "Hazel/Scene/SceneCamera.h"
#include "Hazel/Scene/ScriptableEntity.h"

namespace Hazel
{

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const std::string& tag)
			:Tag(tag) {
		};
		TagComponent(const TagComponent&) = default;

	};

	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const glm::mat4& transform)
			:Transform(transform) {
		};
		TransformComponent(const TransformComponent&) = default;

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct SpriteComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteComponent() = default;
		SpriteComponent(const glm::vec4& color)
			:Color(color) {
		};
		SpriteComponent(const SpriteComponent&) = default;

		operator glm::vec4& () { return Color; }
		operator const glm::vec4& () const { return Color; }
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = false;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		// Set functions( Use std::function )
		std::function<ScriptableEntity* ()> InstantiateScript;//ÊµÀý»¯½Å±¾
		std::function<void(NativeScriptComponent*)> DeinstantiateScript;

		//std::function<void(ScriptableEntity*)> OnCreateFunction;
		//std::function<void(ScriptableEntity*)> OnDestroyFunction;
		//std::function<void(ScriptableEntity*, Timestep)> OnUpdateFunction;

		// Send functions so we can use them later
		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return new T(); };
			DeinstantiateScript = [](NativeScriptComponent* nsc) 
				{delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

}