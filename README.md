# QuentlamEngine
一个基于 C++17 开发的现代 2D/3D 图形渲染引擎（游戏引擎框架）。该引擎旨在提供一套轻量级、模块化、跨平台的底层抽象，帮助开发者快速构建交互式图形应用或游戏。

## 🌟 核心特性
*   **平台无关性抽象**: 提供 Window、Input、Renderer 等底层系统抽象。
*   **渲染器 (Renderer API)**: 支持现代图形 API 抽象（当前集成 OpenGL），包括 VertexBuffer, IndexBuffer, VertexArray, Shader, Texture2D 等管理。
*   **事件系统 (Event System)**: 基于分发机制的事件系统，支持鼠标、键盘、窗口等各种事件交互。
*   **图层架构 (Layer System)**: 通过 `Layer` 和 `LayerStack` 管理渲染与逻辑更新顺序。
*   **摄像机控制器 (Camera Controller)**: 内置正交摄像机（Orthographic Camera）及交互控制器。
*   **ImGui 集成**: 原生集成 Dear ImGui，支持快速构建开发和调试 UI。
*   **日志系统**: 接入 spdlog 提供的轻量化、高性能控制台日志系统。

## 🛠 技术栈与依赖 (Dependencies)
引擎核心库以**静态库**形式编译，依赖以下第三方库（集成在 `vendor` 目录下）：
*   [**GLFW**](https://www.glfw.org/): 窗口与输入管理
*   [**Glad**](https://glad.dav1d.de/): OpenGL 函数加载
*   [**GLM**](https://glm.g-truc.net/): 图形数学库
*   [**Dear ImGui**](https://github.com/ocornut/imgui): 即时模式 GUI
*   [**spdlog**](https://github.com/gabime/spdlog): 快速的 C++ 日志库
*   [**stb_image**](https://github.com/nothings/stb): 图像加载工具

## 🚀 构建与运行 (Build & Run)

项目使用 [Premake5](https://premake.github.io/) 进行跨平台工程文件生成。目前默认配置为 **Windows x64** 平台及 Visual Studio 环境。

### 前置条件
*   Windows 操作系统
*   Visual Studio 2022 (或 2019 等其他版本，若需更改可修改生成脚本)
*   C++17 编译器

### 编译步骤
1. **获取代码**：克隆或下载本仓库代码。
2. **生成工程**：双击运行 `script/GenerateProject.bat`。该脚本会自动调用 `premake5` 并生成 `QuentlamEngine.sln` 解决方案文件。
3. **编译运行**：
   * 打开根目录下的 `QuentlamEngine.sln`
   * 将 `Sandbox` 项目设为**启动项目 (Startup Project)**
   * 选择目标平台架构（如 `Debug` / `Release` - `x64`），然后点击运行即可。

## 📖 使用示例 (Example)
通过继承 `Quentlam::Application` 和 `Quentlam::Layer` 即可快速创建你自己的渲染层：

```cpp
#include <Quentlam.h>
#include <Quentlam/Base/EntryPoint.h> // 包含引擎入口宏

class ExampleLayer : public Quentlam::Layer {
public:
    ExampleLayer() : Layer("Example") {
        // 初始化顶点数据、Shader、纹理等
    }

    void OnUpdate(Quentlam::Timestep ts) override {
        // 每帧更新逻辑、渲染指令提交
        Quentlam::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        Quentlam::RenderCommand::Clear();
        // ... 绘制几何体
    }
};

class SandboxApp : public Quentlam::Application {
public:
    SandboxApp() {
        PushLayer(new ExampleLayer());
    }
    ~SandboxApp() {}
};

// 引擎启动入口
Quentlam::Application* Quentlam::CreateApplication() {
    return new SandboxApp();
}
```

## 📝 目录结构
*   `QuentlamEngine/`: 引擎核心代码 (包含 Application, Events, Renderer 等模块)
*   `Sandbox/`: 基于引擎开发的测试应用及示例项目
*   `vendor/`: 第三方依赖库源码或库文件 (GLFW, Glad, ImGui, glm, stb_image, spdlog)
*   `script/`: 包含 Premake 工程生成等辅助脚本

---
*Developed as a graphic rendering engine learning and practicing project.*