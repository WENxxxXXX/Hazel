# Per-Pixel Linked List Order Independent Transparency (OIT) Implementation Summary

## 1. 概述 (Overview)

本项目实现了一种基于**逐像素链表 (Per-Pixel Linked List)** 的顺序无关透明 (Order Independent Transparency, OIT) 渲染技术。

传统的透明渲染依赖于物体排序（从后往前渲染），但这在物体相互穿插或几何体复杂时往往失效。逐像素链表 OIT 通过在片段着色器中构建每个像素的片段链表，然后在后处理阶段对这些片段进行排序和混合，从而实现了精确的透明渲染，无需预先对物体进行排序。

## 2. 核心数据结构 (Core Data Structures)

为了实现链表构建和存储，使用了以下 OpenGL 资源（在 `Renderer3D` 中进行了抽象）：

### 2.1 头指针纹理 (Head Pointer Texture)
*   **类型**: `Texture2D` (R32UI)
*   **作用**: 屏幕大小的纹理，每个像素存储该位置对应的链表**头节点索引**。
*   **初始化**: 每帧开始时清零 (0)。
*   **Shader 绑定**: `layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;`

### 2.2 链表缓冲区 (Linked List Buffer)
*   **类型**: `TextureBuffer` (RGBA32UI)
*   **作用**: 存储所有透明片段的节点数据。这是一个巨大的线性缓冲区。
*   **节点结构 (uvec4)**:
    *   `x`: **Next Node Index** (指向链表中下一个节点的索引)。
    *   `y`: **Packed Color** (RGBA8，包含光照计算后的颜色和 Alpha)。
    *   `z`: **Depth** (浮点深度值转换为 uint)。
    *   `w`: **Entity ID** (用于拾取或其他用途)。
*   **Shader 绑定**: `layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer list_buffer;`

### 2.3 原子计数器 (Atomic Counter)
*   **类型**: `AtomicCounterBuffer`
*   **作用**: 全局计数器，用于为每个新生成的片段分配唯一的索引（即在 `list_buffer` 中的位置）。
*   **Shader 绑定**: `layout (binding = 0, offset = 0) uniform atomic_uint list_counter;`

---

## 3. 渲染流程 (Rendering Pipeline)

OIT 渲染分为两个主要阶段：**构建阶段 (Build Phase)** 和 **解析阶段 (Resolve Phase)**。

### 3.1 阶段一：构建链表 (Build Phase)

在此阶段，渲染场景中的透明物体。

*   **C++ 设置 (`Renderer3D::BeginScene` / `DrawIndexed`)**:
    1.  **重置资源**:
        *   将原子计数器重置为 0 (或 1，视 Shader 逻辑而定，本项目中 Shader 空指针为 0，所以计数器从 1 开始)。
        *   **清空头指针纹理**: 使用 `PixelUnpackBuffer` (PBO) 进行快速清零。
            *   绑定预先填充了 0 的 PBO。
            *   调用 `headPointTexture->SetData(nullptr, ...)` 触发显存内复制。
            *   这种方法比传统的 `glClearTexImage` 或从 CPU 上传全 0 数据更高效。
    2.  **绑定资源**: 绑定 Image Texture 和 Atomic Counter Buffer。
    3.  **渲染状态**:
        *   **关闭深度写入 (`glDepthMask(GL_FALSE)`)**: 透明物体不应阻挡后续物体的深度测试。
        *   **关闭颜色写入 (`glColorMask(GL_FALSE, ...)`)**: 片段数据直接写入缓冲区，不需要写入帧缓冲颜色附件。
        *   **开启深度测试**: 仍然需要进行深度测试以正确处理遮挡关系（如被不透明物体遮挡）。

*   **Shader 逻辑 (`LinkedListOIT_Build.glsl`)**:
    1.  **分配索引**: `uint index = atomicCounterIncrement(list_counter);`
    2.  **更新头指针**: 使用原子交换操作，将当前像素的头指针更新为新索引，并获取旧的头指针。
        ```glsl
        uint old_head = imageAtomicExchange(head_pointer_image, ivec2(gl_FragCoord.xy), uint(index));
        ```
    3.  **光照计算**: 计算片段的光照颜色。
    4.  **存储节点**: 将数据写入 `list_buffer` 的 `index` 位置。
        ```glsl
        item.x = old_head; // 指向旧的头节点，形成链表
        item.y = packUnorm4x8(vec4(result, material.alpha));
        item.z = floatBitsToUint(gl_FragCoord.z);
        item.w = u_entityID;
        imageStore(list_buffer, int(index), item);
        ```

### 3.2 阶段二：解析与混合 (Resolve Phase)

在此阶段，绘制一个全屏四边形，处理每个像素的链表。

*   **C++ 设置 (`Renderer3D::ResolveOIT`)**:
    1.  **内存屏障 (`glMemoryBarrier`)**: 确保阶段一中的图像写入和原子操作已完成且对当前阶段可见。
    2.  **渲染状态**:
        *   关闭深度测试。
        *   开启混合 (`SrcAlpha`, `OneMinusSrcAlpha`)，以便将解析后的透明颜色混合到背景上。
    3.  **绑定资源**: 绑定头指针纹理、链表缓冲区以及背景纹理（不透明物体的渲染结果）。

*   **Shader 逻辑 (`LinkedListOIT_Resolve.glsl`)**:
    1.  **获取头指针**: `uint current_index = imageLoad(head_pointer_image, ivec2(gl_FragCoord).xy).x;`
    2.  **遍历链表**:
        *   从 `current_index` 开始，通过读取 `list_buffer` 中的 `next` 指针（`.x` 分量）遍历链表。
        *   将所有节点数据（颜色、深度）存储到本地数组 `fragment_list` 中。
        *   设置最大片段数限制 (`MAX_FRAGMENTS`) 以防止性能崩溃。
    3.  **排序**:
        *   对 `fragment_list` 中的片段按深度 (`.z` 分量) 进行排序（通常是从远到近，Back-to-Front）。
    4.  **混合**:
        *   读取背景颜色。
        *   按顺序手动混合透明片段颜色：
            ```glsl
            final_color = mix(final_color, c, c.a);
            ```
    5.  **输出**: 输出最终混合后的颜色。

---

## 4. C++ 抽象层实现 (Abstraction Layer)

为了不直接依赖 OpenGL API，在 `Hazel` 引擎中进行了如下抽象：

### 4.1 `AtomicCounterBuffer`
*   **接口**: `BindBase(index)`, `Reset(value)`, `GetRendererID()`.
*   **OpenGL 实现**: 封装了 `GL_ATOMIC_COUNTER_BUFFER`，使用 `glBindBufferBase` 绑定到 Shader 中的 binding point。

### 4.2 `TextureBuffer`
*   **接口**: 继承自 `Texture`，提供 `Create(size, format)`.
*   **OpenGL 实现**:
    *   创建 `GL_TEXTURE_BUFFER` (TBO)。
    *   同时管理一个 `GL_BUFFER` (用于存储数据) 和一个 `GL_TEXTURE` (用于 Shader 访问)。
    *   使用 `glTextureBuffer` 将 Buffer 关联到 Texture。

### 4.3 `RendererAPI::MemoryBarrierTexFetch`
*   **更新**: 扩展了内存屏障类型，加入了 `GL_SHADER_IMAGE_ACCESS_BARRIER_BIT` 和 `GL_ATOMIC_COUNTER_BARRIER_BIT`，确保 OIT 两个阶段之间的数据同步。

### 4.4 `PixelUnpackBuffer` (PBO)
*   **接口**: `Bind()`, `Unbind()`, `MapBuffer()`, `UnmapBuffer()`.
*   **作用**: 用于高效地传输像素数据。在本项目中，用于**快速清空头指针纹理**。
*   **原理**:
    *   预先在显存中创建一个填充了 0 的缓冲区 (`GL_PIXEL_UNPACK_BUFFER`)。
    *   在每帧开始时，绑定该 PBO，然后调用 `Texture::SetData` (即 `glTexSubImage2D`)。
    *   当 PBO 被绑定时，`glTexSubImage2D` 的最后一个参数（数据指针）被视为 PBO 中的偏移量（此处为 nullptr/0）。
    *   这触发了显存到显存的直接复制 (DMA)，比从 CPU 内存上传数据要快得多，从而避免了每帧清空大纹理时的 CPU-GPU 带宽瓶颈。

## 5. 总结

该方案成功实现了 OIT，解决了透明物体排序问题。
*   **优点**: 渲染结果精确，无需 CPU 排序，支持复杂的几何体穿插。
*   **缺点**:
    *   显存占用较大（需要预分配巨大的链表缓冲区）。
    *   原子操作可能带来性能开销。
    *   Resolve 阶段的排序在 Shader 中进行，片段过多时性能会下降（受 `MAX_FRAGMENTS` 限制）。
