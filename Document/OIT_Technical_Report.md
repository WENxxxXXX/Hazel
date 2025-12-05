# 技术总结报告：Weighted Blended OIT 渲染闪烁与伪影修复

## 1. 问题描述
在实现加权混合顺序无关透明度（Weighted Blended OIT）算法时，**Composite Pass（合成阶段）** 出现了严重的画面闪烁和伪影现象。
*   **现象**：当在 Shader 中采样 `Accum`（累积）和 `Reveal`（显露）纹理时，画面出现随机跳变、噪点或完全错误的颜色。
*   **特例**：如果在 C++ 代码中直接通过 `ReadPixel` 读取纹理数据（CPU端读取），数据是正确的；但在 Shader 中采样（GPU端读取）则出错。

## 2. 根本原因分析
问题的根源在于 **OpenGL 反馈循环（Feedback Loop）** 引发的 **读写冲突（Read-Modify-Write Hazard）**。

### 2.1 资源状态冲突
*   **FBO 配置**：当前的帧缓冲对象（Framebuffer Object）绑定了 4 个颜色附件：
    *   Attachment 0: Color (RGBA)
    *   Attachment 1: EntityID (Integer)
    *   Attachment 2: Accum (RGBA16F) - **OIT 数据源**
    *   Attachment 3: Reveal (R8) - **OIT 数据源**
*   **管线状态**：在进入 Composite Pass 时，虽然使用了 `glColorMaski` 屏蔽了 Attachment 2 和 3 的写入，但 FBO 的 **Draw Buffers** 列表仍然包含这 4 个附件。这意味着 GPU 仍然将它们视为“活跃的渲染目标”。
*   **Shader 行为**：在 Fragment Shader 中，Attachment 2 和 3 被绑定为 `sampler2D` 进行纹理采样（读取）。

### 2.2 未定义行为 (Undefined Behavior)
OpenGL 规范明确规定：**如果一个纹理对象同时被绑定为当前 FBO 的写入目标（即使被 Mask 屏蔽）和 Shader 的读取源，渲染结果是未定义的。**

*   **硬件层面**：GPU 的纹理缓存（Texture Cache）和渲染输出缓存（Render Cache/Write Cache）之间缺乏同步。
*   **冲突机制**：GPU 调度器检测到资源既处于“写状态”又处于“读状态”，导致管线竞争。纹理单元可能读取到未刷新的脏数据、压缩格式错误的显存块，甚至是随机内存值，从而导致闪烁。

## 3. 解决方案
通过显式修改 OpenGL 的 **Draw Buffers** 状态，切断反馈循环。

### 3.1 代码修正 (`Renderer3D.cpp`)
在 `CompositePass` 函数中执行以下操作：

1.  **切断写入路径**：
    在绘制全屏四边形前，调用 `glDrawBuffers` 仅启用 Attachment 0。
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
    绘制结束后，恢复所有附件的写入能力，以免影响后续 Pass（如下一帧的几何体绘制）。
    ```cpp
    GLenum buffers4[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, ... };
    glDrawBuffers(4, buffers4);
    ```

### 3.2 补充修正
*   **深度测试**：在 Composite Pass 中禁用了深度测试（`SetDepthTest(false)`），防止全屏四边形被场景中的几何体遮挡或发生 Z-Fighting。
*   **EntityID 保护**：在透明物体渲染 Pass 中，显式禁止了对 Attachment 1 (EntityID) 的写入，防止混合模式下的整数缓冲区写入错误。

## 4. 结论
该问题并非 Shader 逻辑错误，而是 **OpenGL 状态机配置** 问题。`glColorMask` 仅控制片元写入阶段，不足以解除 GPU 内部的资源读写依赖。必须通过 `glDrawBuffers` 明确告知驱动程序哪些附件是活动的，才能安全地在同一 Pass 中对同一 FBO 的附件进行“采样”和“写入”操作（Ping-Pong 技术或仅写入非采样附件）。
