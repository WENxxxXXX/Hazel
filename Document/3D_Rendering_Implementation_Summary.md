# Hazel 引擎 3D 渲染实现方案总结

本文档详细总结了 Hazel 引擎中的 3D 渲染实现方案，重点分析了渲染管线、核心组件以及加权混合顺序无关透明度（Weighted Blended Order-Independent Transparency, OIT）的实现。

## 1. 概述

Hazel 引擎的 3D 渲染系统主要由 `Renderer3D` 类管理，采用现代 OpenGL (4.5+) API。其核心特性包括：
- **基于物理的渲染基础**：虽然目前主要是 Blinn-Phong 光照模型，但架构上支持材质属性（环境光、漫反射、镜面反射、光泽度）。
- **顺序无关透明度 (OIT)**：实现了 Weighted Blended OIT 算法，解决了传统透明物体渲染需要排序的问题。
- **多渲染目标 (MRT)**：利用 FrameBuffer 的多附件特性，同时处理颜色、实体 ID（用于拾取）和 OIT 数据。
- **批处理与实例化**：虽然代码中主要展示了 `DrawIndexed`，但架构设计允许扩展批处理。

## 2. 核心组件架构

### 2.1 Renderer3D (`Hazel/Renderer/Renderer3D.h/cpp`)
这是 3D 渲染的入口点，负责管理渲染状态、着色器和全局 Uniforms。

*   **初始化 (`Init`)**：加载默认模型、编译着色器（Blinn-Phong, OIT Transparent, OIT Composite），并创建用于后期处理的全屏四边形（Screen Quad）。
*   **场景开始 (`BeginScene`)**：设置视图投影矩阵（ViewProjection Matrix）和场景光照信息（方向光）。
*   **绘制 (`DrawIndexed`)**：根据材质类型选择对应的着色器并提交绘制命令。
*   **合成 (`CompositePass`)**：执行 OIT 的最后一步合成操作。

### 2.2 FrameBuffer (`Hazel/Renderer/FrameBuffer.h`)
FrameBuffer 被配置为支持多种附件，以满足 MRT 需求。在 `EditorLayer::OnAttach` 中可以看到其配置：

```cpp
m_Framebuffer = Hazel::FrameBuffer::Create({ 1280, 720, 1, 
    {
        FrameBufferAttachmentFormat::RGBA8,       // 0: 主颜色缓冲 (Color)
        FrameBufferAttachmentFormat::RED_INTEGER, // 1: 实体 ID (EntityID, 用于鼠标拾取)
        FrameBufferAttachmentFormat::RGBA16F,     // 2: 累积缓冲 (Accumulation, OIT)
        FrameBufferAttachmentFormat::R8,          // 3: 显露缓冲 (Revealage, OIT)
        FrameBufferAttachmentFormat::Depth        // 4: 深度/模板缓冲
    } 
});
```

### 2.3 Model (`Hazel/Renderer/Model.h`)
负责加载和存储网格数据。使用 Assimp 库加载模型文件，并将数据转换为引擎内部的 `Vertex` 结构（位置、法线、纹理坐标、颜色）。

## 3. 渲染管线流程

### 3.1 初始化阶段
在 `Renderer3D::Init` 中，初始化了 `Renderer3DData` 结构体，其中包含了渲染所需的关键资源：
*   **Shader Maps**：存储不同类型的 Shader（`BlinnPhong`, `WeightedBlendOIT`）。
*   **Composite Shader**：用于 OIT 合成的 Shader。
*   **Screen Quad**：用于全屏后处理的 VAO/VBO。

### 3.2 场景渲染循环 (`EditorLayer::OnUpdate`)

1.  **清除缓冲**：
    *   EntityID 清除为 -1。
    *   Accumulation 缓冲清除为 `vec4(0.0f)`。
    *   Revealage 缓冲清除为 `1.0f` (表示完全可见)。

2.  **Begin Scene**：
    *   绑定 Shader。
    *   上传 `u_ViewProjection` 和 `viewPos`。
    *   上传方向光数据 (`dirLight.direction`, `ambient`, `diffuse`, `specular`)。

3.  **物体绘制 (`Renderer3D::DrawIndexed`)**：
    *   根据 `MaterialComponent` 的 `shaderType` 切换 Shader。
    *   上传模型矩阵 `u_Transform` 和实体 ID `u_entityID`。
    *   上传材质属性。
    *   调用 `RendererCommand::DrawIndexed`。

4.  **OIT 合成 (`Renderer3D::CompositePass`)**：
    *   在所有透明物体绘制完成后调用。
    *   关闭深度测试，设置混合模式。
    *   绑定 Accumulation 和 Revealage 纹理。
    *   绘制全屏四边形进行合成。

## 4. 顺序无关透明度 (OIT) 实现细节

项目采用了 **Weighted Blended OIT** (McGuire and Bavoil, 2013) 算法。该算法通过两个 Pass 实现：透明几何体累积 Pass 和 全屏合成 Pass。

### 4.1 Pass 1: 透明几何体累积
**Shader**: `assets/shaders/WeightedBlend_Transparent.glsl`

在此阶段，透明物体被绘制，并且不进行深度写入（但在 `RendererCommand` 中通常会开启深度测试以处理遮挡，OIT 算法本身不需要排序）。

**Fragment Shader 逻辑**：
1.  **计算光照**：使用 Blinn-Phong 模型计算基础颜色。
2.  **计算权重 (Weight)**：
    权重函数倾向于保留更近（深度值小）和更不透明（Alpha 值大）的像素。
    ```glsl
    float weight = clamp(
        pow(min(1.0, color.a * 10.0) + 0.01, 3.0) *
        1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0),
        1e-2,
        3e3
    );
    ```
3.  **输出到 MRT**：
    *   **Accum (Location 2)**: 存储加权颜色。`vec4(color.rgb * color.a, color.a) * weight`。
    *   **Reveal (Location 3)**: 存储透射率（Transmittance）。`color.a` (这里实际上是存储覆盖率，合成时使用 `1.0 - revealage` 或直接存储透射率，代码中 `reveal = color.a` 看起来是存储了 Alpha，合成公式需对应)。
    *   *注意：代码中 `reveal = color.a`，通常标准实现是 `reveal = zero` (初始 1.0) `reveal.r = alpha` 混合模式为 `GL_ZERO, GL_ONE_MINUS_SRC_COLOR` 实现 `dest = dest * (1-src)`。需检查混合模式设置。*

### 4.2 Pass 2: 全屏合成
**Shader**: `assets/shaders/WeightedBlend_Composite.glsl`
**C++ 函数**: `Renderer3D::CompositePass`

在此阶段，将累积的透明颜色混合到背景上。

**混合设置**：
```cpp
RendererCommand::SetBlendFunc(RendererAPI::BlendFactor::SrcAlpha, RendererAPI::BlendFactor::OneMinusSrcAlpha);
```

**Fragment Shader 逻辑**：
1.  读取 `Accum` 和 `Reveal` 纹理。
2.  计算平均颜色：
    ```glsl
    vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);
    ```
3.  输出最终颜色：
    ```glsl
    frag = vec4(average_color, 1.0 - revealage);
    ```
    这里 `1.0 - revealage` 作为最终的 Alpha 值，与背景进行混合。

## 5. 光照模型 (Blinn-Phong)

在 `Blinn-phong.glsl` 和 `WeightedBlend_Transparent.glsl` 中均实现了标准 Blinn-Phong 光照。

```glsl
vec3 CalcDirLight(...)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Combine
    vec3 _ambient = light.ambient * ambient;
    vec3 _diffuse = light.diffuse * diff * diffuse;
    vec3 _specular = light.specular * spec * specular;
    return _ambient + _diffuse + _specular;
}
```

## 6. 总结

Hazel 的 3D 渲染模块构建了一个支持高级透明度处理的框架。通过利用 MRT 和 OIT，它能够正确渲染复杂的透明场景而无需对物体进行 CPU 排序。当前的实现主要依赖于前向渲染（Forward Rendering）路径，并结合了后期处理 Pass 来完成图像合成。
