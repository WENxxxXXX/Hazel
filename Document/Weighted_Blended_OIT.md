# Weighted Blended Order-Independent Transparency (OIT) 技术文档

## 1. 背景与动机 (Background & Motivation)

在实时渲染中，处理半透明物体一直是一个挑战。传统的半透明渲染通常使用 **Alpha Blending**（Alpha 混合），其公式通常为：
$$ C_{final} = C_{src} \cdot \alpha + C_{dst} \cdot (1 - \alpha) $$

这种混合方式对**绘制顺序**非常敏感。为了得到正确的结果，必须按照**从远到近 (Back-to-Front)** 的顺序绘制半透明物体。

### 现有方案的局限性
*   **CPU 排序**：在每一帧对所有半透明物体进行排序。
    *   *缺点*：对于复杂的几何体（如自身重叠的网格、粒子系统），仅对物体级别排序是不够的，需要对三角形级别排序，计算量巨大且难以在 GPU 上高效实现。
*   **错误排序的后果**：如果顺序错误，处于后面的物体可能无法透过前面的物体显示出来，或者混合颜色错误，导致视觉上的瑕疵。

为了解决这个问题，我们需要一种**与绘制顺序无关**的透明度算法，即 **Order-Independent Transparency (OIT)**。

## 2. OIT 方案概览 (Overview of OIT Techniques)

业界存在多种 OIT 解决方案，各有优劣：

| 方案 | 原理 | 优点 | 缺点 |
| :--- | :--- | :--- | :--- |
| **Depth Peeling (深度剥离)** | 多次 Pass 渲染，每次剥离最靠近的一层深度。 | 结果精确，无排序错误。 | 性能开销极大，需要 N 个 Pass 才能剥离 N 层，不适合实时游戏。 |
| **Per-Pixel Linked Lists (PPLL)** | 利用原子操作在显存中为每个像素构建链表，存储所有片段，最后排序混合。 | 结果精确，一次 Pass。 | 显存占用不可控（可能溢出），原子操作影响性能，实现复杂。 |
| **Weighted Blended OIT (WBOIT)** | 基于权重的近似算法。通过加权平均来模拟覆盖关系，距离相机近且不透明度高的像素权重更大。 | **性能极高**（单次 Pass），显存占用固定，适合大多数游戏场景。 | 是一种近似算法，对于某些特定遮挡情况（如不同颜色的半透明物体紧密重叠）可能产生颜色偏差。 |

本项目选择了 **Weighted Blended OIT**，因为它在性能和画质之间取得了最佳平衡，非常适合游戏引擎的实时渲染需求。

## 3. Weighted Blended OIT 原理

Weighted Blended OIT 的核心思想是将传统的顺序依赖混合公式修改为加权和的形式。

### 核心公式
最终颜色的计算公式近似为：
$$ C_{final} = \left( \frac{\sum C_i \cdot \alpha_i \cdot w(z_i, \alpha_i)}{\sum \alpha_i \cdot w(z_i, \alpha_i)} \right) \cdot (1 - \prod (1 - \alpha_i)) + C_{bg} \cdot \prod (1 - \alpha_i) $$

其中：
*   $C_i$：片段颜色
*   $\alpha_i$：片段透明度
*   $z_i$：片段深度
*   $w(z, \alpha)$：权重函数，通常深度越小（越近）、Alpha 越大，权重越高。

### 渲染流程
该算法需要两个额外的渲染目标（Render Targets）：
1.  **Accumulation Texture (RGBA16F)**: 累积加权颜色和权重。
    *   RGB 通道存储 $\sum C_i \cdot \alpha_i \cdot w_i$
    *   A 通道存储 $\sum \alpha_i \cdot w_i$
2.  **Revealage Texture (R8)**: 存储背景的显露程度（即所有物体遮挡后的剩余透明度）。
    *   存储 $\prod (1 - \alpha_i)$。初始值为 1.0，每次乘以 $(1 - \alpha)$。

## 4. 项目实现详解

项目中的实现分为两个阶段（Pass）：**Transparent Pass** 和 **Composite Pass**。

### 4.1 Transparent Pass (几何体渲染)

在此阶段，渲染所有半透明物体。不进行颜色混合，而是进行加权累积。

**Shader**: `WeightedBlend_Transparent.glsl`

#### Vertex Shader
标准的 MVP 变换，计算世界坐标位置和法线。

#### Fragment Shader
1.  **光照计算**：首先计算片段的原始光照颜色（Directional Light + Material）。
2.  **权重计算**：使用 McGuire (2013) 提出的经验权重函数：
    ```glsl
    float weight = clamp(
        pow(min(1.0, color.a * 10.0) + 0.01, 3.0) *
        1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0),
        1e-2,
        3e3
    );
    ```
    *   `color.a` 越大，权重越大。
    *   `gl_FragCoord.z` (深度) 越小（越近），权重越大。
3.  **输出到 MRT (Multiple Render Targets)**：
    *   `accum` (Location 2): `vec4(color.rgb * color.a, color.a) * weight`
    *   `reveal` (Location 3): `color.a` (注意：这里输出的是 alpha，混合模式会处理乘积)

#### 混合状态 (Blend State)
这是算法正确的关键。
*   **Accumulation Target**: 使用加法混合。
    *   `glBlendFunc(GL_ONE, GL_ONE)`
*   **Revealage Target**: 使用零加一减源颜色混合。
    *   `glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR)`
    *   原理：目标值 = $0 \cdot Src + (1 - Src) \cdot Dst = Dst \cdot (1 - \alpha)$。这实现了 $\prod (1 - \alpha_i)$ 的连乘效果。

### 4.2 Composite Pass (全屏合成)

在此阶段，绘制一个全屏四边形，将累积的纹理合成到主帧缓冲上。

**Shader**: `WeightedBlend_Composite.glsl`

#### Fragment Shader
1.  **采样**：读取 `u_Accum` 和 `u_Reveal` 纹理。
2.  **计算平均颜色**：
    ```glsl
    vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);
    ```
    这对应公式中的 $\frac{\sum C_i \cdot \alpha_i \cdot w_i}{\sum \alpha_i \cdot w_i}$。
3.  **混合背景**：
    *   `revealage` 纹理存储的是背景显示的比例（1.0 表示完全显示，0.0 表示完全遮挡）。
    *   半透明物体的混合 Alpha 为 `1.0 - revealage`。
    *   输出 `frag = vec4(average_color, 1.0 - revealage)`。

#### 混合状态 (Blend State)
*   使用标准的 Alpha 混合将结果叠加到不透明场景上：
    *   `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`

---

## 5. 常见问题与解决方案：模型闪烁与伪影修复

在实现过程中，**Composite Pass** 曾出现严重的画面闪烁和伪影现象（噪点、随机颜色跳变）。

### 5.1 问题根源：OpenGL 反馈循环 (Feedback Loop)
问题的根本原因在于 **读写冲突 (Read-Modify-Write Hazard)**。

*   **场景**：在 Composite Pass 中，我们绑定了 FBO 进行绘制。该 FBO 包含了 `Accum` 和 `Reveal` 纹理作为附件（Attachment 2 & 3）。
*   **冲突**：
    1.  **写**：虽然我们可能通过 `glColorMask` 屏蔽了对 Accum/Reveal 的写入，但只要它们存在于当前 FBO 的 `DrawBuffers` 列表中，GPU 就会认为它们是“活跃的写入目标”。
    2.  **读**：同时，我们在 Shader 中将这两个纹理作为 `sampler2D` 进行采样（读取）。
*   **结果**：OpenGL 规范定义这种“既是写入目标又是读取源”的行为会导致**未定义结果 (Undefined Behavior)**。GPU 的纹理缓存和渲染缓存之间没有同步，导致读取到脏数据。

### 5.2 解决方案
必须显式修改 OpenGL 的 **Draw Buffers** 状态，切断反馈循环。

**代码修正步骤 (`Renderer3D.cpp`)**:

1.  **切断写入路径**：
    在绘制 Composite Pass 的全屏四边形之前，显式告知 OpenGL 仅写入颜色附件 0（主颜色缓冲），从而断开 Accum 和 Reveal 的写入连接。
    ```cpp
    // 仅允许写入 Color Attachment，彻底断开 Accum 和 Reveal 的写入连接
    GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, buffers);
    ```

2.  **执行绘制**：
    此时 GPU 知道 Accum 和 Reveal 不再是渲染目标，可以安全地将其作为只读纹理进行采样。
    ```cpp
    RendererCommand::DrawIndexed(s_3DData.screenQuadVAO, ...);
    ```

3.  **恢复状态**：
    绘制结束后，立即恢复所有附件的写入能力，以免影响下一帧的几何体绘制（Transparent Pass 需要写入所有附件）。
    ```cpp
    GLenum buffers4[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, buffers4);
    ```

通过这种方式，我们严格遵守了 OpenGL 的读写规范，彻底解决了闪烁问题。
