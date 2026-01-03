#pragma once

#include "Hazel/Core/UUID.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Scene/SceneCamera.h"
#include "Hazel/Scene/ScriptableEntity.h"

namespace Hazel
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const UUID& id)
			:ID(id) {
		};
		IDComponent(const IDComponent&) = default;
	};

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
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation)
			:Translation(translation) {}
		TransformComponent(const TransformComponent&) = default;

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteComponent() = default;
		SpriteComponent(const glm::vec4& color)
			:Color(color) {
		};
		SpriteComponent(const SpriteComponent&) = default;

		operator glm::vec4& () { return Color; }
		operator const glm::vec4& () const { return Color; }
	};

	struct CircleComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleComponent() = default;
		CircleComponent(const glm::vec4& color)
			:Color(color) {
		};
		CircleComponent(const CircleComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = false;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	// ---------------------- Script --------------------------------------
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		// Set functions( Use std::function )
		std::function<ScriptableEntity* ()> InstantiateScript;//ʵ�����ű�
		std::function<void(NativeScriptComponent*)> DeinstantiateScript;

		//std::function<void(ScriptableEntity*)> OnCreateFunction;
		//std::function<void(ScriptableEntity*)> OnDestroyFunction;
		//std::function<void(ScriptableEntity*, Timestep)> OnUpdateFunction;

		// Send functions so we can use them later
		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DeinstantiateScript = [](NativeScriptComponent* nsc) 
				{delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};


	// ----------------------- Physics --------------------------------------
	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime (just for runtime, don't need to serialize)
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };					// ����ƫ����
		glm::vec2 Size = { 0.5f, 0.5f };					// ��ײ���

		// TODO: May move this to Physics Material
		float Density = 1.0f;								// �ܶ�
		float Friction = 0.5f;								// Ħ����
		float Restitution = 0.0f;							// �ָ�
		float RestitutionThreshold = 0.5f;					// �ָ���ֵ

		// Storage for runtime
		void* RuntimeFixture = nullptr;						// ����ʱ�����������ʱ����

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };					// ����ƫ����
		float Radius = 0.5f;								// ��ײ�뾶��С

		// TODO: May move this to Physics Material
		float Density = 1.0f;								// �ܶ�
		float Friction = 0.5f;								// Ħ����
		float Restitution = 0.0f;							// �ָ�
		float RestitutionThreshold = 0.5f;					// �ָ���ֵ

		// Storage for runtime
		void* RuntimeFixture = nullptr;						// ����ʱ�����������ʱ����

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};


	// ----------------------- 3D Render --------------------------------------
	struct MeshComponent
	{
		Ref<Model> model;
		std::string filePath;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	enum class ShaderType
	{
		BlinnPhong,
		WeightedBlendOIT,
		LinkedListOIT_Build,
		PBR
	};
	struct MaterialComponent
	{
		ShaderType shaderType = ShaderType::BlinnPhong;

		glm::vec3 ambient = { 1.0f, 1.0f, 1.0f };
		glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
		glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
		float shininess = 32.0f;
		float alpha = 0.5f;

		// PBR Maps
		Ref<Texture2D> albedoMap;
		Ref<Texture2D> normalMap;
		Ref<Texture2D> metallicMap;
		Ref<Texture2D> roughnessMap;
		Ref<Texture2D> aoMap;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
	};
}