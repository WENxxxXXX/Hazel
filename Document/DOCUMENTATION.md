# Hazel 游戏引擎技术文档

## 1. 项目简介

Hazel 是一个基于 C++ 开发的现代游戏引擎，旨在提供高性能的 2D/3D 渲染能力和灵活的实体组件系统 (ECS)。本项目参考了 The Cherno 的 Hazel 引擎开发教程，并在此基础上进行了扩展和实现。

## 2. 核心架构

引擎采用了分层架构设计，核心模块位于 `Hazel` 静态库中，编辑器 `Hazelnut` 和沙盒 `Sandbox` 作为客户端应用链接该核心库。

### 2.1 核心循环 (Application Loop)
`Application` 类是引擎的入口点，负责管理主循环：
*   **初始化**: 窗口创建、渲染器初始化、ImGui 上下文设置。
*   **运行循环**: 计算时间步 (Timestep)，更新层栈 (LayerStack)，渲染 ImGui，交换缓冲区。
*   **事件处理**: 接收并分发窗口、输入等事件。

### 2.2 层系统 (Layer System)
引擎使用 `LayerStack` 管理渲染层和逻辑层。
*   **Layer**: 基础类，定义了 `OnAttach`, `OnDetach`, `OnUpdate`, `OnEvent`, `OnImGuiRender` 等生命周期函数。
*   **Overlay**: 覆盖层，通常用于调试 UI 或最上层的渲染，总是最后更新和渲染。
*   **执行顺序**: 普通 Layer 按插入顺序更新，Overlay 最后更新；事件反向传播 (从 Overlay 到底层 Layer)，一旦被处理 (Handled) 即停止传播。

## 3. 核心子系统详解

### 3.1 事件系统 (Event System)
位于 `Hazel/Events`，采用阻塞式事件分发机制。

*   **事件类型 (`EventType`)**: 包含 `WindowClose`, `WindowResize`, `KeyPressed`, `MouseButtonPressed`, `AppTick` 等。
*   **事件分类 (`EventCategory`)**: 使用位掩码标记事件属性 (如 `EventCategoryInput`, `EventCategoryKeyboard`)，便于快速过滤。
*   **分发器 (`EventDispatcher`)**: 
    ```cpp
    // 示例：在 Application::OnEvent 中分发事件
    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        // 如果事件类型是 WindowCloseEvent，则调用 OnWindowClose 函数
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

        // 反向遍历层栈，将事件传递给每一层
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
        {
            (*--it)->OnEvent(e);
            if (e.Handled) // 如果事件被处理，停止传播
                break;
        }
    }
    ```

### 3.2 渲染系统 (Render System)
位于 `Hazel/Renderer`，设计为 API 无关 (API Agnostic)，目前主要实现 OpenGL 后端。

*   **Renderer API**: 定义了基础绘图命令 (`SetClearColor`, `Clear`, `DrawIndexed`)。
*   **Renderer2D**: 
    *   **批处理 (Batching)**: 自动合并多次绘制请求 (DrawQuad) 到单个顶点缓冲区 (VertexBuffer)，减少 Draw Call。
    *   **图元支持**: 支持带颜色/纹理的四边形 (Quad)、旋转四边形、圆形 (Circle)。
    *   **统计 (Stats)**: 实时统计 Draw Calls, Quad Count, Vertex Count 等。
    ```cpp
    // 示例：Renderer2D 使用流程
    Renderer2D::BeginScene(camera); // 开启场景，设置视图投影矩阵
    Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {0.8f, 0.2f, 0.3f, 1.0f}); // 绘制红色方块
    Renderer2D::DrawQuad({1.0f, 0.0f}, {1.0f, 1.0f}, texture); // 绘制带纹理方块
    Renderer2D::EndScene(); // 结束场景，提交批处理数据进行渲染
    ```
*   **Renderer3D**:
    *   支持模型加载 (`Assimp`)。
    *   材质系统 (`MaterialComponent`): 支持 Blinn-Phong, PBR, OIT (加权混合顺序无关透明度) 等着色模式。
*   **帧缓冲 (Framebuffer)**: 支持多附件 (Color, Depth, Red-Integer for ID picking)，用于实现视口渲染和鼠标拾取实体。

### 3.3 实体组件系统 (ECS) 与 EnTT

#### 3.3.1 什么是 ECS？
ECS (Entity Component System) 是一种遵循“组合优于继承”原则的软件架构模式，广泛应用于游戏开发。
*   **Entity (实体)**: 仅仅是一个唯一的 ID (通常是整数)。它本身不包含数据或行为，只是组件的容器。
*   **Component (组件)**: 纯数据结构 (Struct)，不包含逻辑。例如 `TransformComponent` 只包含位置、旋转、缩放数据。
*   **System (系统)**: 包含逻辑，处理拥有特定组件的实体。例如 `PhysicsSystem` 会遍历所有拥有 `Rigidbody` 和 `Transform` 组件的实体，并更新它们的位置。

#### 3.3.2 引入 ECS 的意义
1.  **性能 (Performance)**: 组件数据在内存中紧凑排列，极大提高了 CPU 缓存命中率 (Cache Locality)。
2.  **解耦 (Decoupling)**: 逻辑 (System) 与数据 (Component) 分离，使得代码更易于维护和扩展。
3.  **灵活性 (Flexibility)**: 可以通过动态添加/删除组件来改变实体的行为，而无需复杂的类继承层级。

#### 3.3.3 EnTT 在本项目中的用法
本项目使用 `EnTT` 库作为 ECS 框架。核心类是 `Scene`，它持有一个 `entt::registry` 对象。

*   **创建实体**:
    ```cpp
    // 在 Scene::CreateEntityWithUUID 中
    Entity entity = { m_Registry.create(), this };
    ```

*   **添加组件**:
    ```cpp
    // 使用 emplace 添加组件
    entity.AddComponent<TransformComponent>(glm::vec3{ 0.0f });
    // 内部调用: m_Registry.emplace<TransformComponent>(entityHandle, args...);
    ```

*   **获取组件**:
    ```cpp
    // 获取组件引用
    auto& transform = entity.GetComponent<TransformComponent>();
    // 内部调用: m_Registry.get<TransformComponent>(entityHandle);
    ```

*   **遍历实体 (View)**:
    这是 ECS 最强大的功能，可以高效遍历拥有特定组件集合的所有实体。
    ```cpp
    // 示例：Scene::OnUpdateRuntime 中更新物理位置
    // 创建一个视图，包含所有拥有 Rigidbody2DComponent 的实体
    auto view = m_Registry.view<Rigidbody2DComponent>();
    for (auto e : view)
    {
        Entity entity = { e, this };
        auto& tc = entity.GetComponent<TransformComponent>();
        auto& rb2c = entity.GetComponent<Rigidbody2DComponent>();

        // 同步 Box2D 物理位置到 Transform 组件
        b2Body* body = (b2Body*)rb2c.RuntimeBody;
        tc.Translation.x = body->GetPosition().x;
        tc.Translation.y = body->GetPosition().y;
        tc.Rotation.z = body->GetAngle();
    }
    ```

*   **组 (Group)**:
    对于需要同时拥有多个组件的遍历，使用 `group` 可以进一步优化性能 (通过排序保证内存连续性)。
    ```cpp
    // 示例：渲染所有带 Sprite 的实体
    // 获取所有同时拥有 TransformComponent 和 SpriteComponent 的实体
    auto& group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
    for (auto entity : group)
    {
        // 结构化绑定获取组件
        auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);
        Renderer2D::DrawQuadSprite(transform.GetTransform(), sprite, (int)entity);
    }
    ```

### 3.4 编辑器 (Hazelnut)
位于 `Hazelnut` 项目，基于 ImGui 构建的可视化开发环境。

*   **EditorLayer**: 编辑器的核心层，负责布局管理和场景渲染。
*   **面板 (Panels)**:
    *   `SceneHierarchyPanel`: 显示场景实体树，支持创建/销毁实体。
    *   `PropertiesPanel`: (集成在 Hierarchy 中) 检查和修改选中实体的组件属性。
    *   `ContentBrowserPanel`: 浏览项目资源 (`assets` 目录)，支持拖拽资源到场景。
    *   `ToolbarPanel`: 顶部工具栏，控制 播放/停止/暂停 场景。
*   **视口 (Viewport)**: 将 `Framebuffer` 的纹理渲染为 ImGui 图像，集成 `ImGuizmo` 实现 3D 变换操作 (移动/旋转/缩放)。
*   **鼠标拾取 (Mouse Picking)**: 利用 Framebuffer 的 `RED_INTEGER` 附件存储实体 ID，实现点击视口选中实体。

## 4. 第三方库及其作用

本项目集成了多个高质量的开源库，它们在引擎中扮演着关键角色：

| 库名称 | 版本/来源 | 主要作用 | 在本项目中的应用 |
| :--- | :--- | :--- | :--- |
| **GLFW** | Latest | 窗口与输入 | 跨平台窗口创建、OpenGL 上下文管理、键盘/鼠标输入轮询。 |
| **Glad** | Latest | OpenGL 加载器 | 加载 OpenGL 函数指针，管理 OpenGL 版本兼容性。 |
| **ImGui** | Docking Branch | GUI 界面 | 构建 Hazelnut 编辑器的所有 UI (面板、菜单、窗口停靠)。 |
| **EnTT** | Latest | ECS 框架 | 核心 ECS 实现，管理实体注册表 (`entt::registry`) 和组件存储。 |
| **glm** | Latest | 数学库 | 向量 (`vec3`, `vec4`)、矩阵 (`mat4`) 运算，变换计算。 |
| **spdlog** | Latest | 日志系统 | 提供高性能的控制台日志输出 (`HZ_CORE_INFO`, `HZ_WARN` 等宏)。 |
| **yaml-cpp** | Latest | 序列化 | 场景文件的保存与加载 (`.hazel` 文件)，将 ECS 数据序列化为 YAML 格式。 |
| **stb_image** | Latest | 图像加载 | 加载 `.png`, `.jpg` 等纹理文件数据。 |
| **ImGuizmo** | Latest | 3D 操作手柄 | 在编辑器视口中提供可视化的物体变换手柄 (Gizmos)。 |
| **Box2D** | Latest | 2D 物理引擎 | 处理 2D 刚体动力学、碰撞检测 (`Rigidbody2D`, `Collider2D`)。 |
| **Assimp** | Latest | 模型加载 | 加载各种 3D 模型格式 (`.obj`, `.fbx`, `.gltf`) 到引擎的 Mesh 数据结构中。 |

## 5. 物理系统集成

引擎集成了 **Box2D** 用于 2D 物理模拟。
*   **运行时**: 在 `Scene::OnRuntimeStart` 时创建 `b2World`。
*   **同步**: 每一帧 `Scene::OnUpdateRuntime` 会调用 `b2World::Step` 进行物理模拟，并将 Box2D 的刚体位置同步回实体的 `TransformComponent`。
*   **组件映射**: `Rigidbody2DComponent` 映射为 `b2Body`，`BoxCollider2DComponent` 映射为 `b2Fixture`。

## 6. 脚本系统

目前支持 **Native Scripting** (原生 C++ 脚本)。
*   开发者继承 `ScriptableEntity` 类。
*   实现 `OnCreate()`, `OnDestroy()`, `OnUpdate(Timestep ts)` 方法。
*   在编辑器中通过 `NativeScriptComponent` 绑定该类。
*   运行时，引擎会自动实例化脚本对象并调用生命周期函数。

## 7. 资源管理

*   **Texture2D**: 封装 OpenGL 纹理，支持从文件加载。
*   **Shader**: 封装 OpenGL Shader Program，支持从 GLSL 源码编译。
*   **ShaderLibrary**: 管理已加载的 Shader，防止重复加载。
*   **Asset Manager**: (开发中) 简单的路径管理，目前主要通过文件路径直接访问。
