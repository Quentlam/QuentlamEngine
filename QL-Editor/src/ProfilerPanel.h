#pragma once

#include "imgui/imgui.h"
#include <string>

namespace Quentlam {

    // ---------------------------------------------------------
    // Performance Profiler Panel (Real-time Metrics)
    // ---------------------------------------------------------

    class ProfilerPanel {
    public:
        // Current metrics collected from Renderer2D/Core
        struct Metrics {
            float FPS = 60.0f;
            int DrawCalls = 0;
            float TextureMemoryMB = 0.0f;
            float GCPeakMB = 0.0f; // For script environment
        };

        Metrics CurrentMetrics;

        void OnImGuiRender() {
            ImGui::Begin("Performance Profiler");

            ImGui::Text("FPS: %.1f", CurrentMetrics.FPS);

            // DrawCall > 60 automatically highlighted red
            if (CurrentMetrics.DrawCalls > 60)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("DrawCalls: %d", CurrentMetrics.DrawCalls);
            ImGui::PopStyleColor();

            // Texture Memory > 120MB automatically highlighted red
            if (CurrentMetrics.TextureMemoryMB > 120.0f)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("Texture Memory: %.1f MB", CurrentMetrics.TextureMemoryMB);
            ImGui::PopStyleColor();

            ImGui::Text("GC Peak: %.1f MB", CurrentMetrics.GCPeakMB);

            ImGui::End();
        }
    };

    // ---------------------------------------------------------
    // Multi-platform Exporter & Previewer
    // ---------------------------------------------------------

    class MultiPlatformExporter {
    public:
        enum class Platform { WebGL, WeChatMiniGame, Android, iOS, Windows };

        void OneClickPreview(Platform targetPlatform) {
            switch(targetPlatform) {
                case Platform::WebGL:
                    // 1. Emscripten compile/bundle
                    // 2. Launch local python http.server
                    // 3. Open browser at localhost:8000
                    break;
                case Platform::WeChatMiniGame:
                    // 1. Pack JS + Wasm
                    // 2. Export project.config.json
                    // 3. Open WeChat DevTools CLI
                    break;
                case Platform::Android:
                    // 1. Build APK via Gradle
                    // 2. ADB install & run on connected device
                    break;
                case Platform::iOS:
                    // 1. Generate Xcode project
                    // 2. xcodebuild & deploy to simulator/device
                    break;
            }
        }
    };
}
