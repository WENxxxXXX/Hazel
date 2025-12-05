#include "hzpch.h"
#include "Scene.h"

#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Renderer/RendererCommand.h"

#include "glm/glm.hpp"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"

namespace Hazel
{
	static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
		case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
		case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}
		HZ_CORE_ASSERT(false, "Unknown body type");
		return b2_staticBody;
	}

	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{
	}

	// CopyComponentForNewEntity
	template<typename Component>
	static void CopyComponentIfExists(Entity dstEntity, Entity srcEntity)
	{
		if (srcEntity.HasComponent<Component>())
		{
			dstEntity.AddOrReplaceComponent<Component>(srcEntity.GetComponent<Component>());
		}

	}

	template<typename Component>
	static void CopyComponentForNewScene(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			HZ_CORE_ASSERT((enttMap.find(uuid) != enttMap.end()), "Unable to find an entity in dst with an ID that matches the src entity ID(Occurs in Scene copy)");
			entt::entity dstEntity = enttMap.at(uuid);			// If src entity ID can be find in dst entity IDs map(dst entiy is which we created by src),then we copy component for this entity

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEntity, component);
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();
		auto& dstRegistry = newScene->m_Registry;
		auto& srcRegistry = other->m_Registry;

		// Init private member
		newScene->m_ViewportHeight = other->m_ViewportHeight;
		newScene->m_ViewportWidth = other->m_ViewportWidth;

		// Init entites & create unordered_map<UUID, entt::entity>
		std::unordered_map<UUID, entt::entity> dstEntityMap;

		auto& view = srcRegistry.view<IDComponent>();
		for (auto e : view)
		{
			UUID uuid = srcRegistry.get<IDComponent>(e).ID;
			const auto& name = srcRegistry.get<TagComponent>(e).Tag;		// const auto& name ?
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);	// Create new entities for newScene (new entities is empty now)

			dstEntityMap[uuid] = (entt::entity)newEntity;
		}

		// Copy data to dstScene from srcScene( except IdComponent & TagComponent)
		CopyComponentForNewScene<TransformComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<CameraComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<SpriteComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<CircleComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<NativeScriptComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<Rigidbody2DComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<BoxCollider2DComponent>(dstRegistry, srcRegistry, dstEntityMap);
		CopyComponentForNewScene<CircleCollider2DComponent>(dstRegistry, srcRegistry, dstEntityMap);

		return newScene;
	}

	void Scene::DuplicateEntity(Entity& srcEntity)
	{
		std::string name = srcEntity.GetComponent<TagComponent>().Tag;
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<TransformComponent>(newEntity, srcEntity);
		CopyComponentIfExists<CameraComponent>(newEntity, srcEntity);
		CopyComponentIfExists<SpriteComponent>(newEntity, srcEntity);
		CopyComponentIfExists<CircleComponent>(newEntity, srcEntity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, srcEntity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, srcEntity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, srcEntity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, srcEntity);

	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>(glm::vec3{ 0.0f });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unnamed Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity& entity)
	{
		m_Registry.destroy(entity);
	}

	// Init the Box2d world & Attach all physical properties to the object beforehand
	void Scene::OnRuntimeStart()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };								// Just for using member function of class Entity
			auto& tc = entity.GetComponent<TransformComponent>();
			auto& rb2c = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2c.Type);
			bodyDef.position.Set(tc.Translation.x, tc.Translation.y);
			bodyDef.angle = tc.Rotation.z;
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);

			body->SetFixedRotation(rb2c.FixedRotation);
			rb2c.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2c = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2c.Size.x * tc.Scale.x, bc2c.Size.y * tc.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2c.Density;
				fixtureDef.friction = bc2c.Friction;
				fixtureDef.restitution = bc2c.Restitution;
				fixtureDef.restitutionThreshold = bc2c.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2c = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2c.Offset.x, cc2c.Offset.y);
				circleShape.m_radius = cc2c.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2c.Density;
				fixtureDef.friction = cc2c.Friction;
				fixtureDef.restitution = cc2c.Restitution;
				fixtureDef.restitutionThreshold = cc2c.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		}
	}

	// Destory the Box2d world
	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		{
			// Allow physics simulation & Update the body's transform every frame for rendering
			const int velocityIterations = 6;
			const int positionIterations = 2;
			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& tc = entity.GetComponent<TransformComponent>();
				auto& rb2c = entity.GetComponent<Rigidbody2DComponent>();

				b2Body* body = (b2Body*)rb2c.RuntimeBody;
				b2Vec2 position = body->GetPosition();
				float angle = body->GetAngle();

				tc.Translation.x = position.x;
				tc.Translation.y = position.y;
				tc.Rotation.z = angle;
			}
		}

		// Render 2D objects
		Camera* mainCamera = nullptr;
		glm::mat4 mainTransform;

		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
			// ����⵽ĳ����������Ϊ����������򴫳�������������ݣ�Ȼ��������
			if (camera.Primary)
			{
				mainCamera = &camera.Camera;
				mainTransform = transform.GetTransform();
				break;
			}
		}

		if (mainCamera) {
			// Do some rendering (��ȡ��ǰ�������ͶӰ���� projection �� λ�ƾ��� transform��
			Renderer2D::BeginScene(*mainCamera, mainTransform);

			
			// Update rectangles
			{
				auto& group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);	// �����к��� TransformComponent ��ʵ�����Ѽ����� sprite ��ʵ�壬group ����һ������ע�����ʵ�弯��
				for (auto entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

					Renderer2D::DrawQuadSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}
			// Update circles
			{
				auto& view = m_Registry.view<TransformComponent, CircleComponent>();
				for (auto entity : view)
				{
					auto [transform, circle] = view.get<TransformComponent, CircleComponent>(entity);
					Renderer2D::DrawCircleSprite(transform.GetTransform(), circle, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera camera, Ref<FrameBuffer> framebuffer)
	{
		Renderer2D::BeginScene(camera);

		// Update rectangles
		{
			auto& group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);	// �����к��� TransformComponent ��ʵ�����Ѽ����� sprite ��ʵ�壬group ����һ������ע�����ʵ�弯��
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

				Renderer2D::DrawQuadSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}
		// Update circles
		{
			auto& view = m_Registry.view<TransformComponent, CircleComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleComponent>(entity);
				Renderer2D::DrawCircleSprite(transform.GetTransform(), circle, (int)entity);
			}
		}

		Renderer2D::EndScene();

		bool hasOIT = false;
		Renderer3D::BeginScene(camera);
		{
			auto& view = m_Registry.view<TransformComponent, MeshComponent, MaterialComponent>();
			for (auto entity : view)
			{
				auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
				if (material.shaderType == ShaderType::WeightedBlendOIT)
				{
					hasOIT = true;
					continue;
				}
				if (mesh.model)
				{
					Renderer3D::DrawIndexed(material, transform.GetTransform(),
						mesh.model->GetVertexArray(), mesh.model->GetIndexBuffer()->GetCount(), (int)entity);
				}
			}

			// Transparent Pass
			RendererCommand::SetDepthTest(true);
			RendererCommand::SetDepthMask(false);
			//RendererCommand::SetColorMaski(0, false, false, false, false);
			//RendererCommand::SetColorMaski(1, true, true, true, true);
			//RendererCommand::SetColorMaski(2, true, true, true, true);
			//RendererCommand::SetColorMaski(3, true, true, true, true);
			RendererCommand::SetBlendFunci(2, RendererAPI::BlendFactor::One, RendererAPI::BlendFactor::One);
			RendererCommand::SetBlendFunci(3, RendererAPI::BlendFactor::Zero, RendererAPI::BlendFactor::OneMinusSrcColor);

			for (auto entity : view)
			{
				auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
				if (material.shaderType != ShaderType::WeightedBlendOIT)
				{
					continue;
				}
				if (mesh.model)
				{
					Renderer3D::DrawIndexed(material, transform.GetTransform(),
						mesh.model->GetVertexArray(), mesh.model->GetIndexBuffer()->GetCount(), (int)entity);
				}
			}

			//RendererCommand::SetColorMaski(0, true, true, true, true);
			//RendererCommand::SetColorMaski(1, false, false, false, false);
			//RendererCommand::SetColorMaski(2, false, false, false, false);
			//RendererCommand::SetColorMaski(3, false, false, false, false);
			if (hasOIT)
			{
				Renderer3D::CompositePass(framebuffer);
			}

			RendererCommand::SetDepthMask(true);
			RendererCommand::SetBlendFunc(RendererAPI::BlendFactor::SrcAlpha, RendererAPI::BlendFactor::OneMinusSrcAlpha);
		}
		//Renderer3D::EndScene();
	}

	void Scene::OnScript(Timestep ts)
	{
		// --------------------------------------------------------------------------------------------------------------------------------------------------------
		// ���С����нű������ʵ�塱����������� transform����Ϊ ScriptableEntity::GetComponent<>() ���� m_ScriptableEntity.GetComponent<T>(); 
		// ����� m_ScriptableEntity ���� Entity ���͵ģ����Ե��õ��� Entity ��GetComponent<T>()�������Ҫ�� m_ScriptableEntity ���г�ʼ����
		// Ϊ��ʹ�����������У��� Scene::OnScript �У�m_ScriptableEntity ����ʼ��Ϊ Entity{ entity, this }��
		// ���ڴ�ʱ�ڻص����� each �У�Entity{ entity, this }�ĵ�һ���������лص������Զ���ȡ�ģ����������������� erntity Ӧ�������ڴ�����ʵ�壬Ҳ���Ǻ��нű������ʵ�塣

		// ����˵�⽫�������а����ű������ʵ�壬����ÿһ��ʵ��� transform �ı����ֵ��ͬ���⵼��ÿһ��ʵ�嶼���ܼ���Ӱ����ƶ������ܴ�ʱֻ��һ��ʵ�屻��ʾ������
		// --------------------------------------------------------------------------------------------------------------------------------------------------------


		// Update scripts�������к��� NativeScriptComponent ��������д�����������ʽ�� lambda ���壩
		m_Registry.view<NativeScriptComponent, CameraComponent>().each
		(
			[=](auto entity, auto& nsc, auto& cc)// nsc => NativeScriptComponent, cc => CameraComponent
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					// �ص������е�entity��һ��uint������id,����ҪΪm_ScriptableEntity���ù��캯��������id��Scene��ָ�롣
					nsc.Instance->m_ScriptableEntity = Entity{ entity, this };

					nsc.Instance->OnCreate();
				}

				//if (cc.Primary)
					nsc.Instance->OnUpdate(ts);
			}
		);


	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto& view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = m_Registry.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.ViewportResize(width, height);
		}
	}

	Entity Scene::GetPrimaryCamera()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& cameraComponent = m_Registry.get<CameraComponent>(entity);
			if (cameraComponent.Primary == true)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleComponent>(Entity entity, CircleComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.ViewportResize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component)
	{
	}
}