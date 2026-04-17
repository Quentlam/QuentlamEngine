# 商业级边缘检测与描边渲染方案 (跨引擎实现)

## 1. 方案概述

本方案旨在解决物体选中时的边缘线渲染问题（如两倍大小包裹、偏移、厚度不均等），提供一种基于后处理（Post-Processing）和 Sobel 算子的商业级描边解决方案。方案不仅在自研引擎 (Quentlam Engine) 中实现了基于 EntityID 的精确描边，还提供了在主流商业引擎（Unity 和 Unreal Engine）中基于深度与法线的标准实现。

## 2. 自研引擎 (Quentlam Engine) 实现
自研引擎中，我们利用了多渲染目标（MRT），将实体的 ID（EntityID）输出到独立的帧缓冲附件（RED_INTEGER）。在后处理阶段，全屏绘制时利用 Sobel 算子对 EntityID 纹理进行边缘检测。这种做法可以避免因模型缩放带来的“双重轮廓”或“偏移”问题。

**特点**：
- **精确度高**：基于实体 ID 的硬边界检测，避免了深度或法线由于平滑过渡带来的误判。
- **性能优异**：只需进行一次 3x3 邻域采样，避免了复杂的几何膨胀运算。
- **参数可调**：在 ImGui 面板中提供了 `Outline Color`, `Outline Width`, 和 `Outline Intensity` 的实时调节。

## 3. Unity 引擎实现 (URP)
在 Unity (Universal Render Pipeline) 中，我们提供了一个基于深度与法线的后处理 Shader 以及对应的 C# Render Feature 脚本。

### Shader (`SobelOutline.shader`)
详见 `Unity/SobelOutline.shader`。利用了 `_CameraDepthTexture` 和 `_CameraNormalsTexture`，通过 Sobel 算子计算深度差和法线差，合成最终的边缘强度。

### C# 脚本 (`SobelOutlineFeature.cs`)
详见 `Unity/SobelOutlineFeature.cs`。作为 URP 的 Render Feature 插入到渲染管线中（AfterRenderingTransparents 阶段），并暴露出颜色、宽度、强度等参数供美术调节。

## 4. Unreal Engine (UE4/UE5) 实现
在虚幻引擎中，我们通过材质编辑器（Material Editor）构建后处理材质。

### 材质设置 (Post-Process Material)
详见 `Unreal/PostProcess_Outline.md` 中的材质节点搭建指南。
- **Material Domain**: Post Process
- **Blendable Location**: Before Tonemapping
- **核心逻辑**：使用 `SceneTexture: SceneDepth` 和 `SceneTexture: WorldNormal` 节点，对 UV 进行上下左右偏移采样（步长由 Width 参数控制），计算 Sobel 梯度，最终输出 Emissive Color。

## 5. 性能基准与内存占用报告
详见 `Reports/PerformanceBenchmark.md`。

## 6. 自动化测试用例
详见 `Tests/OutlineStabilityTests.md`。

## 7. 美术验收清单
详见 `Reports/ArtAcceptanceList.md`。