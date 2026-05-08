#include "qlpch.h"
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include <Quentlam.h>
#include "EditorLayer.h"
#include "EditorToolbarLayout.h"
#include "Quentlam/Events/ApplicationEvent.h"
#include "Quentlam/Physics/Physics3D.h"
#include "ParkourSystem.h"
#include "Quentlam/Debug/CrashReporter.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <ImGuizmo.h>

// OpenGL Stencil definitions
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_ALWAYS                         0x0207
#define GL_NOTEQUAL                       0x0205

#include "Quentlam/Renderer/VertexArray.h"
#include "Glad/include/glad/glad.h"
#include <chrono>
#include <excpt.h>
#include <filesystem>

namespace Quentlam
{
	namespace
	{
		constexpr float kToolbarTransitionDurationSeconds = 0.2f;
		constexpr ImU32 kToolbarPanelBackground = IM_COL32(45, 48, 54, 255);
		constexpr ImU32 kToolbarPanelBorder = IM_COL32(78, 82, 90, 255);

		struct ToolbarButtonTheme
		{
			ImVec4 Normal = ImVec4(0.19f, 0.21f, 0.24f, 1.0f);
			ImVec4 Hovered = ImVec4(0.26f, 0.28f, 0.31f, 1.0f);
			ImVec4 Active = ImVec4(0.31f, 0.34f, 0.38f, 1.0f);
			ImVec4 Toggled = ImVec4(0.24f, 0.31f, 0.42f, 1.0f);
			ImVec4 Border = ImVec4(0.35f, 0.38f, 0.44f, 1.0f);
			ImVec4 Tint = ImVec4(0.92f, 0.94f, 0.97f, 1.0f);
		};

		bool HasToolbarIcon(const Ref<Texture2D>& icon)
		{
			return icon && icon->GetRendererID() != 0;
		}

		void DrawToolbarGroupBackground(const ImVec2& min, const ImVec2& max, float rounding)
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(min, max, kToolbarPanelBackground, rounding);
			drawList->AddRect(min, max, kToolbarPanelBorder, rounding);
		}

#if defined(_MSC_VER)
		int EditorExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
		{
			// If it's a C++ exception (0xE06D7363), let it pass to C++ catch blocks
			if (code == 0xE06D7363)
			{
				return EXCEPTION_CONTINUE_SEARCH;
			}

			QL_CORE_ERROR("Structured exception (0x{0:X}) caught in Editor. Generating crash dump...", code);
			CrashReporter::HandleCrash(ep);
			
			// Recover from the crash to prevent the editor from terminating!
			return EXCEPTION_EXECUTE_HANDLER;
		}
#endif

		bool ExecuteSceneRuntimeStart(Scene* scene, bool& started)
		{
			started = false;
#if defined(_MSC_VER)
			__try
			{
				started = scene && scene->OnRuntimeStart();
				return true;
			}
			__except (EditorExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
			{
				QL_CORE_ERROR("Play request triggered a structured exception ({0}) while starting runtime.", static_cast<unsigned int>(GetExceptionCode()));
				return false;
			}
#else
			started = scene && scene->OnRuntimeStart();
			return true;
#endif
		}

		void ExecuteSceneRuntimeStop(Scene* scene)
		{
#if defined(_MSC_VER)
			__try
			{
				if (scene)
					scene->OnRuntimeStop();
			}
			__except (EditorExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
			{
				QL_CORE_ERROR("Stop request triggered a structured exception ({0}) while leaving runtime.", static_cast<unsigned int>(GetExceptionCode()));
			}
#else
			if (scene)
				scene->OnRuntimeStop();
#endif
		}

		bool DrawToolbarActionButton(const char* id, const char* fallbackLabel, const Ref<Texture2D>& icon, const ImVec2& size, float iconInset, bool toggled = false)
		{
			const ToolbarButtonTheme theme;
			ImGui::PushID(id);
			bool clicked = false;
			if (HasToolbarIcon(icon))
			{
				clicked = ImGui::InvisibleButton("##ToolbarAction", size);
				const bool hovered = ImGui::IsItemHovered();
				const bool held = ImGui::IsItemActive();
				const ImVec2 min = ImGui::GetItemRectMin();
				const ImVec2 max = ImGui::GetItemRectMax();
				const ImVec4 background = held ? theme.Active : (hovered ? (toggled ? theme.Active : theme.Hovered) : (toggled ? theme.Toggled : theme.Normal));
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				drawList->AddRectFilled(min, max, ImGui::GetColorU32(background), 4.0f);
				drawList->AddRect(min, max, ImGui::GetColorU32(theme.Border), 4.0f);
				const ImVec2 imageMin(min.x + iconInset, min.y + iconInset);
				const ImVec2 imageMax(max.x - iconInset, max.y - iconInset);
				drawList->AddImage((ImTextureID)(intptr_t)icon->GetRendererID(), imageMin, imageMax, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(theme.Tint));
			}
			else
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Border, theme.Border);
				ImGui::PushStyleColor(ImGuiCol_Button, toggled ? theme.Toggled : theme.Normal);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, toggled ? theme.Active : theme.Hovered);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.Active);
				clicked = ImGui::Button(fallbackLabel, size);
				ImGui::PopStyleColor(4);
				ImGui::PopStyleVar(2);
			}
			ImGui::PopID();
			return clicked;
		}
	}

	static void SetUEStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.38f, 0.45f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.23f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.TabRounding = 0.0f;
	}

	EditorLayer::EditorLayer()
		:Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_PerspCameraController(35.0f, 1280.0f / 720.0f)
	{

	}

	void EditorLayer::OnScenePlay()
	{
		if (m_IsSceneTransitioning || m_SceneState != SceneState::Edit)
			return;

		if (!m_ActiveScene)
		{
			QL_CORE_ERROR("Play request ignored: active scene is null.");
			return;
		}

		m_LastPlayFailure.clear();
		std::string validationFailure;
		if (!m_ActiveScene->ValidateRuntimeState(&validationFailure))
		{
			m_LastPlayFailure = validationFailure;
			QL_CORE_ERROR("Play request blocked by runtime validation: {0}", validationFailure);
			QL_CORE_WARN("Play request was safely rejected before entering runtime mode.");
			return;
		}

		m_IsSceneTransitioning = true;
		bool started = false;
		try
		{
			if (!ExecuteSceneRuntimeStart(m_ActiveScene.get(), started))
				m_LastPlayFailure = "Structured exception raised while starting runtime scene.";
		}
		catch (const std::exception& e)
		{
			m_LastPlayFailure = e.what();
			QL_CORE_ERROR("Play request failed with exception: {0}", e.what());
		}
		catch (...)
		{
			m_LastPlayFailure = "Unknown exception raised while starting runtime scene.";
			QL_CORE_ERROR("Play request failed with unknown exception.");
		}

		if (started)
		{
			ParkourSystem::OnRuntimeStart(m_ActiveScene.get(), true);
			m_SceneState = SceneState::Play;
			m_bIsPaused = false;
			m_ToolbarTransitionProgress = 0.0f;
		}
		else
		{
			m_SceneState = SceneState::Edit;
			m_bIsPaused = false;
			if (m_LastPlayFailure.empty())
				m_LastPlayFailure = "Runtime start returned false without an explicit error.";
			QL_CORE_WARN("Play request was safely rejected before entering runtime mode.");
		}

		m_IsSceneTransitioning = false;
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_IsSceneTransitioning || m_SceneState == SceneState::Edit)
			return;

		if (!m_ActiveScene)
		{
			m_SceneState = SceneState::Edit;
			m_bIsPaused = false;
			return;
		}

		m_IsSceneTransitioning = true;
		try
		{
			ExecuteSceneRuntimeStop(m_ActiveScene.get());
			ParkourSystem::OnRuntimeStop(m_ActiveScene.get());
		}
		catch (const std::exception& e)
		{
			QL_CORE_ERROR("Stop request failed with exception: {0}", e.what());
		}
		catch (...)
		{
			QL_CORE_ERROR("Stop request failed with unknown exception.");
		}

		m_SceneState = SceneState::Edit;
		m_bIsPaused = false;
		m_ToolbarTransitionProgress = 0.0f;
		m_IsSceneTransitioning = false;
	}

	void EditorLayer::OnScenePause()
	{
		if (m_IsSceneTransitioning || m_SceneState == SceneState::Edit)
			return;

		m_SceneState = SceneState::Pause;
		m_bIsPaused = true;
		m_ToolbarTransitionProgress = 0.0f;
	}

	void EditorLayer::ResumeScenePlay()
	{
		if (m_IsSceneTransitioning || m_SceneState != SceneState::Pause)
			return;

		m_SceneState = SceneState::Play;
		m_bIsPaused = false;
		m_ToolbarTransitionProgress = 0.0f;
	}

	void EditorLayer::OnAttach()
	{
		SetUEStyle();
		m_CameraController.SetZoomLevel(8.0f);

		// Configure ImGuizmo Style (UE-like)
		ImGuizmo::Style& style = ImGuizmo::GetStyle();
		style.TranslationLineThickness = 4.0f;
		style.TranslationLineArrowSize = 8.0f;
		style.RotationLineThickness = 4.0f;
		style.RotationOuterLineThickness = 3.0f;
		style.ScaleLineThickness = 4.0f;
		style.ScaleLineCircleSize = 8.0f;
		style.HatchedAxisLineThickness = 6.0f;
		style.CenterCircleSize = 6.0f;

		m_Texture2D = Texture2D::Create("assets/texture/background.png");
		m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
		m_TextureStairs = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7,6 }, { 128,128 });
		m_TextureBarrel = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8,2 }, { 128,128 });
		m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2,1 }, { 128,128 }, { 1,2 });



		m_DirectoryIcon = Texture2D::Create("assets/icons/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("assets/icons/FileIcon.png");
		m_IconFBX = Texture2D::Create("assets/icons/Icon_FBX.png");
		m_IconPNG = Texture2D::Create("assets/icons/Icon_PNG.png");
		m_IconWAV = Texture2D::Create("assets/icons/Icon_WAV.png");
		m_IconUASSET = Texture2D::Create("assets/icons/Icon_UASSET.png");
		m_IconUMAP = Texture2D::Create("assets/icons/Icon_UMAP.png");

		m_IconPlay = Texture2D::Create("assets/icons/PlayButton.png");
		m_IconPause = Texture2D::Create("assets/icons/PauseButton.png");
		m_IconStop = Texture2D::Create("assets/icons/StopButton.png");
		m_IconAdd = Texture2D::Create("assets/icons/AddButton.png");

		m_OutlineShader = Shader::Create("assets/shaders/OutlineShader.glsl");
		glCreateVertexArrays(1, &m_EmptyVAO);

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;

		m_Framebuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
		if (!ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
		{
			QL_CORE_WARN("Failed to load scene preset. Building default scene.");
			ParkourSystem::BuildScene(m_ActiveScene.get());
		}
	}

	void EditorLayer::OnDetach()
	{
		QL_PROFILE_FUNCTION();

		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		Physics3D::Shutdown();

		if (m_EmptyVAO)
			glDeleteVertexArrays(1, &m_EmptyVAO);
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (m_Is3DCamera)
			m_PerspCameraController.OnEvent(event);
		else
			m_CameraController.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(QL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		if (control && !shift && e.GetKeyCode() == Key::S)
		{
			if (m_SceneState == SceneState::Edit)
			{
				ParkourSystem::SaveScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene");
				QL_CORE_INFO("Saved Scene to assets/configs/ParkourTemplate.scene via shortcut");
				m_ShowSaveNotification = true;
				m_SaveNotificationTimer = 2.0f; // Show for 2 seconds
			}
			else
			{
				QL_CORE_WARN("Cannot save scene while playing. Please stop the scene first.");
				m_ShowSaveNotification = true;
				m_SaveNotificationTimer = 3.0f;
			}
		}

		switch (e.GetKeyCode())
		{
			case Key::Delete:
			{
				if (m_SelectedEntity)
				{
					m_ActiveScene->GetRegistry().destroy(m_SelectedEntity);
					m_SelectedEntity = {};
				}
				break;
			}
			case Key::A:
			{
				if (control && shift)
				{
					m_ShowQuickAddPanel = !m_ShowQuickAddPanel;
				}
				break;
			}
			// Gizmos
			case Key::Q:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			case Key::W:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case Key::F:
			{
				if (!m_Is3DCamera)
				{
					if (m_SelectedEntity && m_SelectedEntity.HasComponent<TransformComponent>())
					{
						auto& tc = m_SelectedEntity.GetComponent<TransformComponent>().Transform;
						m_CameraController.SetPosition({ tc[3].x, tc[3].y, 0.0f });
						m_CameraController.SetZoomLevel(5.0f);
					}
					else
					{
						m_CameraController.SetPosition({ 0.0f, 0.0f, 0.0f });
						m_CameraController.SetZoomLevel(12.0f);
					}
					// Update projection matrix
					m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
				}
				break;
			}
		}

		return false;
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		if (m_ShowSaveNotification)
		{
			m_SaveNotificationTimer -= ts;
			if (m_SaveNotificationTimer <= 0.0f)
				m_ShowSaveNotification = false;
		}

		if (m_ViewportFocused)
		{
			if (m_Is3DCamera)
				m_PerspCameraController.OnUpdate(ts);
			else
				m_CameraController.OnUpdate(ts);
		}


		//Renderer
		Renderer2D::ResetStats();
		Renderer3D::ResetStats();
		m_Framebuffer->Bind();
		m_Framebuffer->ClearAttachment(1, -1);
		RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		RenderCommand::Clear();

		// Ensure depth test is enabled before rendering the scene
		RenderCommand::SetDepthTest(true);

		if (m_Is3DCamera)
		{
			Renderer3D::BeginScene(m_PerspCameraController.GetCamera());
			
			// Draw the default cube
			if (m_CubeEntity)
			{
				auto& tc = m_CubeEntity.GetComponent<TransformComponent>();
				Renderer3D::DrawCube(tc.Transform, { 0.8f, 0.2f, 0.3f, 1.0f }, (int)(uint32_t)m_CubeEntity);
			}

			auto view = m_ActiveScene->GetRegistry().view<TransformComponent, CubeRendererComponent>();
			view.each([](auto entityID, auto& tc, auto& cube)
			{
				Renderer3D::DrawCube(tc.Transform, cube.Color, (int)(uint32_t)entityID, cube.AmbientStrength, cube.DiffuseStrength, cube.SpecularStrength, cube.Shininess);
			});
			
			auto primView = m_ActiveScene->GetRegistry().view<TransformComponent, PrimitiveRendererComponent>();
			primView.each([](auto entityID, auto& tc, auto& prim)
			{
				// Render all primitives as cubes for now until mesh generation is implemented
				Renderer3D::DrawCube(tc.Transform, prim.Color, (int)(uint32_t)entityID, prim.AmbientStrength, prim.DiffuseStrength, prim.SpecularStrength, prim.Shininess);
			});

			if (m_ShowPhysicsColliders)
			{
				Renderer3D::EndScene(); // Flush filled geometry

				Renderer3D::BeginScene(m_PerspCameraController.GetCamera());
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(2.0f);

				auto view = m_ActiveScene->GetRegistry().view<TransformComponent, BoxCollider3DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc3d] = view.get<TransformComponent, BoxCollider3DComponent>(entity);
					if (!bc3d.ShowCollider) continue;
					glm::vec3 scale(
						glm::length(glm::vec3(tc.Transform[0])),
						glm::length(glm::vec3(tc.Transform[1])),
						glm::length(glm::vec3(tc.Transform[2]))
					);
					glm::vec3 position = tc.Transform[3];
					glm::vec3 offset = bc3d.Offset;
					// Apply the same rotation as the transform component
					glm::mat4 rotationMatrix = tc.Transform;
					// Zero out translation
					rotationMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					// Normalize columns to remove scale
					rotationMatrix[0] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[0])), 0.0f);
					rotationMatrix[1] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[1])), 0.0f);
					rotationMatrix[2] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[2])), 0.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
						rotationMatrix * 
						glm::translate(glm::mat4(1.0f), offset) *
						glm::scale(glm::mat4(1.0f), bc3d.HalfExtent * scale * 2.0f);
					Renderer3D::DrawCube(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)(uint32_t)entity);
				}

				Renderer3D::EndScene(); // Flush lines
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore fill mode

				// Begin scene again for the model
				Renderer3D::BeginScene(m_PerspCameraController.GetCamera());
			}

			if (m_Model && m_ModelEntity)
			{
				auto& tc = m_ModelEntity.GetComponent<TransformComponent>();
				Renderer3D::DrawModel(tc.Transform, *m_Model, glm::vec4(1.0f), (int)(uint32_t)m_ModelEntity);
			}
			Renderer3D::EndScene();
			
			Renderer2D::BeginScene(m_PerspCameraController.GetCamera());
		}
		else
		{
			Renderer3D::BeginScene(m_CameraController.GetCamera());
			if (m_CubeEntity)
			{
				auto& tc = m_CubeEntity.GetComponent<TransformComponent>();
				Renderer3D::DrawCube(tc.Transform, { 0.8f, 0.2f, 0.3f, 1.0f }, (int)(uint32_t)m_CubeEntity);
			}
			auto view = m_ActiveScene->GetRegistry().view<TransformComponent, CubeRendererComponent>();
			view.each([](auto entityID, auto& tc, auto& cube)
			{
				Renderer3D::DrawCube(tc.Transform, cube.Color, (int)(uint32_t)entityID, cube.AmbientStrength, cube.DiffuseStrength, cube.SpecularStrength, cube.Shininess);
			});

			auto primView = m_ActiveScene->GetRegistry().view<TransformComponent, PrimitiveRendererComponent>();
			primView.each([](auto entityID, auto& tc, auto& prim)
			{
				if (prim.Type == PrimitiveRendererComponent::PrimitiveType::Capsule)
				{
					Renderer3D::DrawCapsule(tc.Transform, prim.Color, (int)(uint32_t)entityID, prim.AmbientStrength, prim.DiffuseStrength, prim.SpecularStrength, prim.Shininess);
				}
				else
				{
					// Render other primitives as cubes for now until more mesh generation is implemented
					Renderer3D::DrawCube(tc.Transform, prim.Color, (int)(uint32_t)entityID, prim.AmbientStrength, prim.DiffuseStrength, prim.SpecularStrength, prim.Shininess);
				}
			});

			if (m_ShowPhysicsColliders)
			{
				Renderer3D::EndScene(); // Flush filled geometry

				Renderer3D::BeginScene(m_CameraController.GetCamera());
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(2.0f);

				auto view = m_ActiveScene->GetRegistry().view<TransformComponent, BoxCollider3DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc3d] = view.get<TransformComponent, BoxCollider3DComponent>(entity);
					if (!bc3d.ShowCollider) continue;
					glm::vec3 scale(
						glm::length(glm::vec3(tc.Transform[0])),
						glm::length(glm::vec3(tc.Transform[1])),
						glm::length(glm::vec3(tc.Transform[2]))
					);
					glm::vec3 position = tc.Transform[3];
					glm::vec3 offset = bc3d.Offset;
					glm::mat4 rotationMatrix = tc.Transform;
					rotationMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					rotationMatrix[0] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[0])), 0.0f);
					rotationMatrix[1] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[1])), 0.0f);
					rotationMatrix[2] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[2])), 0.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
						rotationMatrix * 
						glm::translate(glm::mat4(1.0f), offset) *
						glm::scale(glm::mat4(1.0f), bc3d.HalfExtent * scale * 2.0f);
					Renderer3D::DrawCube(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)(uint32_t)entity);
				}

				Renderer3D::EndScene(); // Flush lines
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore fill mode

				// Begin scene again for the model
				Renderer3D::BeginScene(m_CameraController.GetCamera());
			}

			Renderer3D::EndScene();

			Renderer2D::BeginScene(m_CameraController.GetCamera());
		}

		//update scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				// In Edit mode, continuously sync the Player's visual Transform back to the StartPosition
				// so that wherever the user dragged it, it will be saved correctly and start there when Play is pressed.
				auto playerView = m_ActiveScene->GetRegistry().view<PlayerControllerComponent, TransformComponent>();
				for (auto entity : playerView)
				{
					auto& pc = playerView.get<PlayerControllerComponent>(entity);
					auto& tc = playerView.get<TransformComponent>(entity);
					pc.StartPosition = { tc.Transform[3].x, tc.Transform[3].y };
				}

				m_ActiveScene->OnUpdate(ts);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(ts);
				ParkourSystem::OnUpdate(m_ActiveScene.get(), ts);
				break;
			}
			case SceneState::Pause:
			{
				m_ActiveScene->OnUpdate(ts); // Just render, no physics/script update
				break;
			}
		}

				// Make camera follow player in both Edit and Play states
		if (!m_Is3DCamera)
		{
			bool isMainMenu = false;
			auto uiView = m_ActiveScene->GetRegistry().view<UIFlowComponent>();
			for (auto entity : uiView)
			{
				if (uiView.get<UIFlowComponent>(entity).CurrentState == UIFlowComponent::State::MainMenu)
					isMainMenu = true;
			}

			auto view = m_ActiveScene->GetRegistry().view<PlayerControllerComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto& tc = view.get<TransformComponent>(entity);
				auto& player = view.get<PlayerControllerComponent>(entity);

				if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause)
				{
					if (m_ActiveScene->GetRegistry().valid(entity) && m_ActiveScene->GetRegistry().all_of<SimpleParticleSystemComponent>(entity))
					{
						auto& psc = m_ActiveScene->GetRegistry().get<SimpleParticleSystemComponent>(entity);
						for (auto& particle : psc.ParticlePool)
						{
							if (!particle.Active) continue;
							float life = particle.LifeRemaining / particle.LifeTime;
							glm::vec4 color = glm::mix(particle.ColorEnd, particle.ColorBegin, life);
							color.a = color.a * life;
							float size = glm::mix(particle.SizeEnd, particle.SizeBegin, life);
							Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
						}
					}
				}

				glm::vec3 currentPos = m_CameraController.GetPosition();
				
				glm::vec3 targetPos;
				float lerpSpeed;
				
				if (isMainMenu)
				{
					// Follow slowly and ignore Y (hover effect) to act like a background plane
					targetPos = { tc.Transform[3].x, 0.0f, 0.0f };
					lerpSpeed = 2.0f;
				}
				else
				{
					// Smoothly track the ship in game (or Edit mode)
					targetPos = { tc.Transform[3].x, tc.Transform[3].y, 0.0f };
					lerpSpeed = 20.0f;
				}
				
				// Apply smoothing only in Play or Pause state, otherwise snap immediately to avoid crazy fast scrolling in Edit mode
				if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause)
				{
					glm::vec3 newPos = currentPos + (targetPos - currentPos) * (lerpSpeed * (float)ts);
					m_CameraController.SetPosition(newPos);
				}
				else
				{
					// If we have selected the player ship in Edit mode, don't force snap camera!
					// Let the user drag it freely with ImGuizmo without the camera fighting them.
					if (!m_SelectedEntity || m_SelectedEntity != entity)
					{
						m_CameraController.SetPosition(targetPos);
					}
				}
				
				break;
			}
		}
		
		Renderer2D::EndScene(); // Flush all filled geometry first

		if (m_ShowPhysicsColliders)
		{
			// Start a new batch for lines
			if (m_Is3DCamera)
				Renderer2D::BeginScene(m_PerspCameraController.GetCamera());
			else
				Renderer2D::BeginScene(m_CameraController.GetCamera());

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(2.0f);

			auto view = m_ActiveScene->GetRegistry().view<TransformComponent, BoxCollider2DComponent>();
			for (auto entity : view)
			{
				auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);
				if (!bc2d.ShowCollider) continue;
				
				glm::vec3 scale(
					glm::length(glm::vec3(tc.Transform[0])),
					glm::length(glm::vec3(tc.Transform[1])),
					glm::length(glm::vec3(tc.Transform[2]))
				);

				glm::vec3 position = tc.Transform[3];
				// Z-offset so it renders slightly in front of the sprite
				position.z += 0.01f;

				glm::vec3 offset = glm::vec3(bc2d.Offset.x, bc2d.Offset.y, 0.0f);
				
				glm::mat4 rotationMatrix = tc.Transform;
				rotationMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				rotationMatrix[0] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[0])), 0.0f);
				rotationMatrix[1] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[1])), 0.0f);
				rotationMatrix[2] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[2])), 0.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
					rotationMatrix *
					glm::translate(glm::mat4(1.0f), offset) *
					glm::scale(glm::mat4(1.0f), glm::vec3(bc2d.Size.x * scale.x, bc2d.Size.y * scale.y, 1.0f));

				Renderer2D::DrawQuad(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)(uint32_t)entity);
			}

			auto triView = m_ActiveScene->GetRegistry().view<TransformComponent, TriangleCollider2DComponent>();
			for (auto entity : triView)
			{
				auto [tc, tc2d] = triView.get<TransformComponent, TriangleCollider2DComponent>(entity);
				if (!tc2d.ShowCollider) continue;
				
				glm::vec3 scale(
					glm::length(glm::vec3(tc.Transform[0])),
					glm::length(glm::vec3(tc.Transform[1])),
					glm::length(glm::vec3(tc.Transform[2]))
				);

				glm::vec3 position = tc.Transform[3];
				position.z += 0.01f;

				glm::vec3 offset = glm::vec3(tc2d.Offset.x, tc2d.Offset.y, 0.0f);
				
				glm::mat4 rotationMatrix = tc.Transform;
				rotationMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				rotationMatrix[0] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[0])), 0.0f);
				rotationMatrix[1] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[1])), 0.0f);
				rotationMatrix[2] = glm::vec4(glm::normalize(glm::vec3(rotationMatrix[2])), 0.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
					rotationMatrix *
					glm::translate(glm::mat4(1.0f), offset) *
					glm::scale(glm::mat4(1.0f), glm::vec3(tc2d.Size.x * scale.x, tc2d.Size.y * scale.y, 1.0f));

				Renderer2D::DrawTriangle(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)(uint32_t)entity);
			}

			Renderer2D::EndScene(); // Flush lines
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore fill mode
		}

		
		// Outline Rendering (Post-Process via Sobel Edge Detection on EntityID)
		if (m_SelectedEntity)
		{
			bool hasSprite = m_SelectedEntity.HasComponent<SpriteTransformComponent>();
			bool hasTri = m_SelectedEntity.HasComponent<TriangleRendererComponent>();
			bool isCube = (m_SelectedEntity == m_CubeEntity);
			bool isModel = (m_SelectedEntity == m_ModelEntity);

			if (hasSprite || hasTri || isCube || isModel)
			{
				// 1. Disable Depth Test to draw over everything
				RenderCommand::SetDepthTest(false);
				
				// 2. Disable writing to EntityID attachment (we only want to draw into Color attachment 0)
				GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, drawBuffers);
				
				// 3. Enable blending so outline overlays the scene
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				// 4. Bind Shader and set uniforms
				m_OutlineShader->Bind();
			uint32_t width = m_Framebuffer->GetSpecification().Width;
			uint32_t height = m_Framebuffer->GetSpecification().Height;
			m_OutlineShader->SetFloat2("u_TexSize", glm::vec2(1.0f / width, 1.0f / height));
			m_OutlineShader->SetInt("u_SelectedEntity", (int)(uint32_t)m_SelectedEntity);
			m_OutlineShader->SetFloat4("u_OutlineColor", m_OutlineColor);
			m_OutlineShader->SetInt("u_OutlineWidth", m_OutlineWidth);
			m_OutlineShader->SetFloat("u_OutlineIntensity", m_OutlineIntensity); // 3 pixels outline width
				
				// 5. Bind EntityID texture to unit 1 and tell shader to use it
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_Framebuffer->GetColorAttachmentRendererID(1));
				m_OutlineShader->SetInt("u_EntityIDTexture", 1);
				
				// 6. Draw fullscreen quad (triangle)
				glBindVertexArray(m_EmptyVAO);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glBindVertexArray(0);
				
				// 7. Restore states
				GLenum allBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
				glDrawBuffers(2, allBuffers); // Restore writing to both color attachments
				
				glDisable(GL_BLEND);
				RenderCommand::SetDepthTest(true);
			}
		}
		
		auto[mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			// Check if pixelData is valid and is a valid entity ID
			if (pixelData != -1 && m_ActiveScene->GetRegistry().valid((entt::entity)pixelData))
			{
				m_HoveredEntity = Entity((entt::entity)pixelData, m_ActiveScene.get());
			}
			else
			{
				m_HoveredEntity = Entity();
			}
		}
		else
		{
			m_HoveredEntity = Entity();
		}

		m_Framebuffer->UnBind();

	}

	void EditorLayer::UI_Toolbar()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		if (!viewport)
			return;

		const float uiScale = ImGui::GetIO().FontGlobalScale > 0.0f ? ImGui::GetIO().FontGlobalScale : 1.0f;
		const auto layout = EditorToolbarLayout::Calculate(2, viewport->WorkSize.x, viewport->WorkPos.y, uiScale, viewport->DpiScale);

		ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + layout.WindowX, layout.WindowY), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(layout.WindowWidth, layout.WindowHeight), ImGuiCond_Always);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

		ImGui::Begin("##Toolbar", nullptr,
			ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoNavFocus
			| ImGuiWindowFlags_NoNavInputs);

		const bool isPlaying = m_SceneState == SceneState::Play;
		const bool isPaused = m_SceneState == SceneState::Pause;
		ImVec2 buttonSize(layout.ButtonSize, layout.ButtonSize);
		const ImVec2 toolbarPos = ImGui::GetWindowPos();
		const ImVec2 toolbarSize = ImGui::GetWindowSize();
		DrawToolbarGroupBackground(toolbarPos, ImVec2(toolbarPos.x + toolbarSize.x, toolbarPos.y + toolbarSize.y), layout.CornerRounding);

		if (m_SceneState != m_LastToolbarVisualState)
		{
			m_LastToolbarVisualState = m_SceneState;
			m_ToolbarTransitionProgress = 0.0f;
		}
		m_ToolbarTransitionProgress = glm::min(m_ToolbarTransitionProgress + (ImGui::GetIO().DeltaTime / kToolbarTransitionDurationSeconds), 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.65f + (0.35f * m_ToolbarTransitionProgress));
		ImGui::SetCursorPos(ImVec2(layout.ButtonX, layout.ButtonY));

		const char* buttonId = "PlayScenePrimary";
		const char* fallbackLabel = "播放";
		const Ref<Texture2D>* icon = &m_IconPlay;
		const char* tooltip = "播放场景";

		if (isPlaying)
		{
			buttonId = "StopScenePrimary";
			fallbackLabel = "停止";
			icon = &m_IconStop;
			tooltip = "停止场景";
		}
		else if (isPaused)
		{
			buttonId = "ResumeScenePrimary";
			fallbackLabel = "播放";
			icon = &m_IconPlay;
			tooltip = "继续播放";
		}

		if (DrawToolbarActionButton(buttonId, fallbackLabel, *icon, buttonSize, layout.IconInset, isPlaying))
		{
			if (isPlaying)
				OnSceneStop();
			else if (isPaused)
				ResumeScenePlay();
			else
				OnScenePlay();
		}
		if (ImGui::IsItemHovered())
		{
			if (!m_LastPlayFailure.empty() && !isPlaying && !isPaused)
				ImGui::SetTooltip("播放失败: %s", m_LastPlayFailure.c_str());
			else
				ImGui::SetTooltip("%s", tooltip);
		}

		ImGui::SameLine(0, std::round(8.0f * uiScale * viewport->DpiScale));
		
		Ref<Texture2D> nullIcon;
		if (DrawToolbarActionButton("RestartScene", "重置", nullIcon, buttonSize, layout.IconInset, false))
		{
			if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause)
			{
				OnSceneStop();
				
				// Clear selections before destroying scene
				m_SelectedEntity = {};
				m_HoveredEntity = {};

				// Reload the scene to reset everything to initial state
				m_ActiveScene = CreateRef<Scene>();
				if (!ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
				{
					ParkourSystem::BuildScene(m_ActiveScene.get());
				}

				OnScenePlay();
				OnScenePause();
			}
			else
			{
				// Clear selections before destroying scene
				m_SelectedEntity = {};
				m_HoveredEntity = {};

				// Even in edit mode, reload to ensure it's fresh, then start and pause
				m_ActiveScene = CreateRef<Scene>();
				if (!ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
				{
					ParkourSystem::BuildScene(m_ActiveScene.get());
				}

				OnScenePlay();
				OnScenePause();
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("重新开始 (Restart)");
		}

		ImGui::PopStyleVar();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}

	void EditorLayer::OnImGuiLayer()
	{
		QL_PROFILE_FUNCTION();

		ImGuizmo::BeginFrame();

		bool dockSpaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;


		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("停靠空间", &dockSpaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("文件"))
			{
				if (ImGui::MenuItem("保存场景预设", "Ctrl+S"))
				{
					if (m_SceneState == SceneState::Edit)
					{
						ParkourSystem::SaveScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene");
						QL_CORE_INFO("Saved Scene to assets/configs/ParkourTemplate.scene");
						m_ShowSaveNotification = true;
						m_SaveNotificationTimer = 2.0f;
					}
					else
					{
						QL_CORE_WARN("Cannot save scene while playing. Please stop the scene first.");
						m_ShowSaveNotification = true;
						m_SaveNotificationTimer = 3.0f;
					}
				}
				if (ImGui::MenuItem("退出"))
					Application::Get().Close();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("窗口"))
			{
				ImGui::MenuItem("内容浏览器", NULL, &m_IsContentBrowserOpen);
				ImGui::MenuItem("显示物理碰撞箱", NULL, &m_ShowPhysicsColliders);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("游戏对象"))
			{
				if (ImGui::BeginMenu("3D 对象"))
				{
					if (ImGui::MenuItem("物理演示平面"))
					{
						Entity plane = m_ActiveScene->CreateEntity("PhysicsDemoPlane");
						auto& tc = plane.GetComponent<TransformComponent>();
						tc.Transform = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f));

						plane.AddComponent<CubeRendererComponent>(glm::vec4(0.5f, 0.8f, 0.5f, 1.0f));

						auto& rb3d = plane.AddComponent<Rigidbody3DComponent>();
						rb3d.Type = Rigidbody3DComponent::BodyType::Kinematic;
						rb3d.Mass = 1.0f;

						auto& bc3d = plane.AddComponent<BoxCollider3DComponent>();
						bc3d.HalfExtent = glm::vec3(5.0f, 0.05f, 5.0f); // 10 * 0.5, 0.1 * 0.5, 10 * 0.5
						
						m_SelectedEntity = plane;
					}

					ImGui::Separator();

					auto createPrimitive = [&](const char* name, PrimitiveRendererComponent::PrimitiveType type) {
						static int counter = 1;
						char buffer[64];
						snprintf(buffer, sizeof(buffer), "%s_%03d", name, counter++);
						Entity entity = m_ActiveScene->CreateEntity(buffer);
						entity.AddComponent<PrimitiveRendererComponent>(type);
						m_SelectedEntity = entity;
					};

					if (ImGui::MenuItem("立方体")) createPrimitive("Cube", PrimitiveRendererComponent::PrimitiveType::Cube);
					if (ImGui::MenuItem("球体")) createPrimitive("Sphere", PrimitiveRendererComponent::PrimitiveType::Sphere);
					if (ImGui::MenuItem("圆柱体")) createPrimitive("Cylinder", PrimitiveRendererComponent::PrimitiveType::Cylinder);
					if (ImGui::MenuItem("胶囊体")) createPrimitive("Capsule", PrimitiveRendererComponent::PrimitiveType::Capsule);
					if (ImGui::MenuItem("圆锥体")) createPrimitive("Cone", PrimitiveRendererComponent::PrimitiveType::Cone);
					if (ImGui::MenuItem("圆环")) createPrimitive("Torus", PrimitiveRendererComponent::PrimitiveType::Torus);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		UI_Toolbar();

		if (m_ShowSaveNotification)
		{
			ImGui::SetNextWindowPos(ImVec2(m_ViewportBounds[0].x + (m_ViewportBounds[1].x - m_ViewportBounds[0].x) * 0.5f, m_ViewportBounds[0].y + (m_ViewportBounds[1].y - m_ViewportBounds[0].y) * 0.8f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowBgAlpha(m_SaveNotificationTimer > 0.5f ? 0.8f : (m_SaveNotificationTimer / 0.5f) * 0.8f);
			ImGui::Begin("Save Notification", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
			if (m_SceneState == SceneState::Edit)
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "场景预设已保存 (Scene Saved) - Ctrl+S");
			else
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "请先停止游戏再保存! (Stop Game to Save)");
			ImGui::End();
		}

		if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause)
		{
			// Render UI
			auto uiView = m_ActiveScene->GetRegistry().view<UIFlowComponent, ScoreSystemComponent>();
			for (auto [entity, ui, score] : uiView.each())
			{
				ImGui::Begin("游戏UI (Game UI)", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs);
				ImGui::SetWindowPos(ImVec2(m_ViewportBounds[0].x + 20, m_ViewportBounds[0].y + 20));
				ImGui::SetWindowFontScale(2.0f);
				if (ui.CurrentState == UIFlowComponent::State::GameOver)
				{
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "游戏结束 (GAME OVER)");
					ImGui::Text("最终得分 (Final Score): %d", score.CurrentScore);
				}
				else
				{
					ImGui::Text("得分 (Score): %d", score.CurrentScore);
				}
				ImGui::SetWindowFontScale(1.0f);
				ImGui::End();
			}
		}

		// Scene Hierarchy Panel
		ImGui::Begin("场景层级");

		// Extract entities to sort and group
		std::vector<entt::entity> allEntities;
		for (auto entityID : m_ActiveScene->GetRegistry().view<TagComponent>())
		{
			allEntities.push_back(entityID);
		}

		auto drawEntityNode = [&](entt::entity entityID) {
			Entity entity{ entityID, m_ActiveScene.get() };
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf; // Make leaf so it doesn't look like folder
			
			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityID, flags, tag.c_str());
			if (ImGui::IsItemClicked())
			{
				m_SelectedEntity = entity;
			}
			
			bool entityDeleted = false;
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("删除实体"))
					entityDeleted = true;
				ImGui::EndPopup();
			}
			
			if (opened)
			{
				ImGui::TreePop();
			}
			
			if (entityDeleted)
			{
				if (m_SelectedEntity == entity)
					m_SelectedEntity = {};
				m_ActiveScene->GetRegistry().destroy(entityID);
			}
		};

		// 1. Group "Pillars"
		if (ImGui::TreeNodeEx("障碍物组", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (auto it = allEntities.begin(); it != allEntities.end();)
			{
				Entity entity{ *it, m_ActiveScene.get() };
				if (entity.GetComponent<TagComponent>().Tag.find("Pillar") == 0 || entity.GetComponent<TagComponent>().Tag == "ObstacleSpawner")
				{
					drawEntityNode(*it);
					it = allEntities.erase(it);
				}
				else
				{
					++it;
				}
			}
			ImGui::TreePop();
		}

		// 2. Group "Environment"
		if (ImGui::TreeNodeEx("环境组", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			for (auto it = allEntities.begin(); it != allEntities.end();)
			{
				Entity entity{ *it, m_ActiveScene.get() };
				const auto& tag = entity.GetComponent<TagComponent>().Tag;
				if (tag == "Background" || tag == "Floor" || tag == "Ceiling")
				{
					drawEntityNode(*it);
					it = allEntities.erase(it);
				}
				else
				{
					++it;
				}
			}
			ImGui::TreePop();
		}

		// 3. Draw remaining ungrouped entities
		for (auto entityID : allEntities)
		{
			drawEntityNode(entityID);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectedEntity = {};

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("创建空实体"))
				m_ActiveScene->CreateEntity("Empty Entity");
			ImGui::EndPopup();
		}
		ImGui::End();

		// Properties Panel
		ImGui::Begin("属性");
		if (m_SelectedEntity)
		{
			bool has2D = m_SelectedEntity.HasComponent<Rigidbody2DComponent>() || m_SelectedEntity.HasComponent<BoxCollider2DComponent>();
			bool has3D = m_SelectedEntity.HasComponent<Rigidbody3DComponent>() || m_SelectedEntity.HasComponent<BoxCollider3DComponent>();
			bool hasConflict = has2D && has3D;

			if (m_SelectedEntity.HasComponent<TagComponent>())
			{
				auto& tag = m_SelectedEntity.GetComponent<TagComponent>().Tag;
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, sizeof(buffer), tag.c_str());
				if (ImGui::InputText("标签", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}
			}

			if (m_SelectedEntity.HasComponent<TransformComponent>())
			{
				// 使用 ImGui::TreeNodeEx 结合图标美化属性面板
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
				
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, "🛠 Transform (变换)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& tc = m_SelectedEntity.GetComponent<TransformComponent>().Transform;
					
					float translation[3], rotation[3], scale[3];
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(tc), translation, rotation, scale);

					bool changed = false;
					if (ImGui::DragFloat3("位置 (Position)", translation, 0.1f)) changed = true;
					if (ImGui::DragFloat3("旋转 (Rotation)", rotation, 0.1f)) changed = true;
					if (ImGui::DragFloat3("缩放 (Scale)", scale, 0.1f)) changed = true;

					if (changed)
					{
						ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, glm::value_ptr(tc));
					}
					
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<SpriteTransformComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(SpriteTransformComponent).hash_code(), treeNodeFlags, "🎨 Sprite Renderer (2D渲染)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& color = m_SelectedEntity.GetComponent<SpriteTransformComponent>().Color;
					ImGui::Text("2D 精灵渲染组件，用于控制颜色和基础材质。");
					ImGui::Spacing();
					ImGui::ColorEdit4("基础颜色 (Base Color)", glm::value_ptr(color));
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<TriangleRendererComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(TriangleRendererComponent).hash_code(), treeNodeFlags, "🔺 Triangle Renderer (2D三角形渲染)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& color = m_SelectedEntity.GetComponent<TriangleRendererComponent>().Color;
					ImGui::Text("2D 三角形渲染组件。");
					ImGui::Spacing();
					ImGui::ColorEdit4("基础颜色 (Base Color)", glm::value_ptr(color));
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<CubeRendererComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(CubeRendererComponent).hash_code(), treeNodeFlags, "🧊 Cube Renderer (3D渲染)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& color = m_SelectedEntity.GetComponent<CubeRendererComponent>().Color;
					ImGui::ColorEdit4("基础颜色 (Base Color)", glm::value_ptr(color));
					
					auto& cube = m_SelectedEntity.GetComponent<CubeRendererComponent>();
					ImGui::DragFloat("环境光 (Ambient)", &cube.AmbientStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("漫反射 (Diffuse)", &cube.DiffuseStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("高光强度 (Specular)", &cube.SpecularStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("高光粗糙度 (Shininess)", &cube.Shininess, 1.0f, 2.0f, 256.0f);

					ImGui::TreePop();
				}
			}
			
			if (m_SelectedEntity.HasComponent<PrimitiveRendererComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(PrimitiveRendererComponent).hash_code(), treeNodeFlags, "🔺 Primitive Renderer (基础图元渲染)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& prim = m_SelectedEntity.GetComponent<PrimitiveRendererComponent>();
					ImGui::ColorEdit4("基础颜色 (Base Color)", glm::value_ptr(prim.Color));
					
					int type = (int)prim.Type;
					const char* typeStrings[] = { "Cube", "Sphere", "Cylinder", "Capsule", "Cone", "Torus" };
					if (ImGui::Combo("图元类型 (Type)", &type, typeStrings, 6))
						prim.Type = (PrimitiveRendererComponent::PrimitiveType)type;

					if (prim.Type == PrimitiveRendererComponent::PrimitiveType::Sphere || 
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Cylinder || 
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Capsule ||
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Cone ||
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Torus)
					{
						ImGui::DragInt("分段数 (Segments)", &prim.Segments, 1, 3, 64);
						ImGui::DragFloat("半径 (Radius)", &prim.Radius, 0.05f, 0.1f, 10.0f);
					}

					if (prim.Type == PrimitiveRendererComponent::PrimitiveType::Cylinder || 
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Capsule ||
						prim.Type == PrimitiveRendererComponent::PrimitiveType::Cone)
					{
						ImGui::DragFloat("高度 (Height)", &prim.Height, 0.05f, 0.1f, 10.0f);
					}

					ImGui::DragFloat("环境光 (Ambient)", &prim.AmbientStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("漫反射 (Diffuse)", &prim.DiffuseStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("高光强度 (Specular)", &prim.SpecularStrength, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("高光粗糙度 (Shininess)", &prim.Shininess, 1.0f, 2.0f, 256.0f);

					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<Rigidbody2DComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				if (hasConflict) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(Rigidbody2DComponent).hash_code(), treeNodeFlags, "🏃 Rigidbody 2D (2D刚体)");
				if (hasConflict) ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (open)
				{
					if (hasConflict) ImGui::TextColored(ImVec4(1,0,0,1), "冲突: 同时存在3D组件! (Conflict with 3D components!)");
					auto& rb2d = m_SelectedEntity.GetComponent<Rigidbody2DComponent>();
					ImGui::Text("刚体类型 (Body Type)");
					int bodyType = (int)rb2d.Type;
					const char* bodyTypeStrings[] = { "静态 (Static)", "动态 (Dynamic)", "运动学 (Kinematic)" };
					if (ImGui::Combo("##BodyType", &bodyType, bodyTypeStrings, 3))
						rb2d.Type = (Rigidbody2DComponent::BodyType)bodyType;
					
					ImGui::Checkbox("固定旋转 (Fixed Rotation)", &rb2d.FixedRotation);
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<BoxCollider2DComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				if (hasConflict) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(BoxCollider2DComponent).hash_code(), treeNodeFlags, "📦 Box Collider 2D (2D碰撞体)");
				if (hasConflict) ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (open)
				{
					if (hasConflict) ImGui::TextColored(ImVec4(1,0,0,1), "冲突: 同时存在3D组件! (Conflict with 3D components!)");
					auto& bc2d = m_SelectedEntity.GetComponent<BoxCollider2DComponent>();
					ImGui::Checkbox("显示碰撞箱 (Show Collider)", &bc2d.ShowCollider);
					ImGui::DragFloat2("偏移 (Offset)", glm::value_ptr(bc2d.Offset), 0.1f);
					ImGui::DragFloat2("尺寸 (Size)", glm::value_ptr(bc2d.Size), 0.1f);
					ImGui::DragFloat("密度 (Density)", &bc2d.Density, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("摩擦力 (Friction)", &bc2d.Friction, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("弹性 (Restitution)", &bc2d.Restitution, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("弹性阈值 (Restitution Threshold)", &bc2d.RestitutionThreshold, 0.01f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<TriangleCollider2DComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				if (hasConflict) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(TriangleCollider2DComponent).hash_code(), treeNodeFlags, "🔺 Triangle Collider 2D (2D三角形碰撞体)");
				if (hasConflict) ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (open)
				{
					if (hasConflict) ImGui::TextColored(ImVec4(1,0,0,1), "冲突: 同时存在3D组件! (Conflict with 3D components!)");
					auto& tc2d = m_SelectedEntity.GetComponent<TriangleCollider2DComponent>();
					ImGui::Checkbox("显示碰撞箱 (Show Collider)", &tc2d.ShowCollider);
					ImGui::DragFloat2("偏移 (Offset)", glm::value_ptr(tc2d.Offset), 0.1f);
					ImGui::DragFloat2("尺寸 (Size)", glm::value_ptr(tc2d.Size), 0.1f);
					ImGui::DragFloat("密度 (Density)", &tc2d.Density, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("摩擦力 (Friction)", &tc2d.Friction, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("弹性 (Restitution)", &tc2d.Restitution, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("弹性阈值 (Restitution Threshold)", &tc2d.RestitutionThreshold, 0.01f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<Rigidbody3DComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				if (hasConflict) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(Rigidbody3DComponent).hash_code(), treeNodeFlags, "🏃 Rigidbody 3D (3D刚体)");
				if (hasConflict) ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (open)
				{
					if (hasConflict) ImGui::TextColored(ImVec4(1,0,0,1), "冲突: 同时存在2D组件! (Conflict with 2D components!)");
					auto& rb3d = m_SelectedEntity.GetComponent<Rigidbody3DComponent>();
					ImGui::Text("刚体类型 (Body Type)");
					int bodyType = (int)rb3d.Type;
					const char* bodyTypeStrings[] = { "静态 (Static)", "动态 (Dynamic)", "运动学 (Kinematic)" };
					if (ImGui::Combo("##BodyType3D", &bodyType, bodyTypeStrings, 3))
						rb3d.Type = (Rigidbody3DComponent::BodyType)bodyType;
					
					if (rb3d.Type != Rigidbody3DComponent::BodyType::Static)
					{
						ImGui::DragFloat("质量 (Mass)", &rb3d.Mass, 0.1f, 0.01f, 1000.0f);
					}
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<BoxCollider3DComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				if (hasConflict) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(BoxCollider3DComponent).hash_code(), treeNodeFlags, "📦 Box Collider 3D (3D碰撞体)");
				if (hasConflict) ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (open)
				{
					if (hasConflict) ImGui::TextColored(ImVec4(1,0,0,1), "冲突: 同时存在2D组件! (Conflict with 2D components!)");
					auto& bc3d = m_SelectedEntity.GetComponent<BoxCollider3DComponent>();
					ImGui::Checkbox("显示碰撞箱 (Show Collider)", &bc3d.ShowCollider);
					ImGui::DragFloat3("偏移 (Offset)", glm::value_ptr(bc3d.Offset), 0.1f);
					ImGui::DragFloat3("半尺寸 (Half Extent)", glm::value_ptr(bc3d.HalfExtent), 0.1f);
					ImGui::DragFloat("摩擦力 (Friction)", &bc3d.Friction, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("弹性 (Restitution)", &bc3d.Restitution, 0.01f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<PlayerControllerComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(PlayerControllerComponent).hash_code(), treeNodeFlags, "🚀 Player Controller (玩家控制)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& pc = m_SelectedEntity.GetComponent<PlayerControllerComponent>();
					ImGui::DragFloat("引擎推力 (Engine Power)", &pc.EnginePower, 0.1f, 0.0f, 50.0f);
					ImGui::DragFloat("最大上升速度 (Max Velocity Y)", &pc.MaxVelocityY, 0.1f, 0.0f, 50.0f);
					ImGui::DragFloat2("初始速度 (Start Velocity)", glm::value_ptr(pc.StartVelocity), 0.1f);
					ImGui::DragFloat("旋转速度-慢 (Rotation Speed Slow)", &pc.RotationSpeedSlow, 1.0f);
					ImGui::DragFloat("旋转速度-快 (Rotation Speed Fast)", &pc.RotationSpeedFast, 1.0f);
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<ObstacleSpawnerComponent>())
			{
				const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(ObstacleSpawnerComponent).hash_code(), treeNodeFlags, "⚙️ Obstacle Spawner (障碍物生成器)");
				ImGui::PopStyleVar();

				if (open)
				{
					auto& osc = m_SelectedEntity.GetComponent<ObstacleSpawnerComponent>();
					ImGui::DragFloat("最小间隙 (Min Gap)", &osc.MinGap, 0.1f, 1.0f, 20.0f);
					ImGui::DragFloat("最大间隙 (Max Gap)", &osc.MaxGap, 0.1f, 1.0f, 20.0f);
					ImGui::DragFloat("X轴间距 (Spacing X)", &osc.SpacingX, 0.1f, 1.0f, 50.0f);
					ImGui::DragFloat("中心偏移变化 (Center Variation)", &osc.CenterVariation, 0.1f, 0.0f, 20.0f);
					ImGui::DragFloat("障碍物移动速度 (Move Speed X)", &osc.MoveSpeedX, 0.1f, -50.0f, 0.0f);
					ImGui::TreePop();
				}
			}

			ImGui::Separator();
			
			float windowWidth = ImGui::GetWindowSize().x;
			float buttonWidth = 150.0f;
			ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
			if (ImGui::Button("添加组件 (Add Component)", ImVec2(buttonWidth, 30.0f)))
			{
				ImGui::OpenPopup("AddComponentPopup");
			}

			bool showComponentError = false;

			if (ImGui::BeginPopup("AddComponentPopup"))
			{
				bool has2D = m_SelectedEntity.HasComponent<Rigidbody2DComponent>() || m_SelectedEntity.HasComponent<BoxCollider2DComponent>();
				bool has3D = m_SelectedEntity.HasComponent<Rigidbody3DComponent>() || m_SelectedEntity.HasComponent<BoxCollider3DComponent>();

				auto addComponentSafe = [&](const char* name, bool is2DComp, bool is3DComp, auto addFunc) {
					if (ImGui::MenuItem(name))
					{
						if ((is2DComp && has3D) || (is3DComp && has2D))
						{
							showComponentError = true;
						}
						else
						{
							addFunc();
						}
					}
				};

				if (!m_SelectedEntity.HasComponent<TransformComponent>())
				{
					addComponentSafe("变换 (Transform)", false, false, [&]() { m_SelectedEntity.AddComponent<TransformComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<SpriteTransformComponent>())
				{
					addComponentSafe("2D 精灵渲染 (Sprite Renderer)", false, false, [&]() { m_SelectedEntity.AddComponent<SpriteTransformComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<CubeRendererComponent>())
				{
					addComponentSafe("3D 立方体渲染 (Cube Renderer)", false, false, [&]() { m_SelectedEntity.AddComponent<CubeRendererComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<PrimitiveRendererComponent>())
				{
					addComponentSafe("基础图元渲染 (Primitive Renderer)", false, false, [&]() { m_SelectedEntity.AddComponent<PrimitiveRendererComponent>(); });
				}
				
				if (!m_SelectedEntity.HasComponent<Rigidbody2DComponent>())
				{
					addComponentSafe("2D 刚体 (Rigidbody 2D)", true, false, [&]() { m_SelectedEntity.AddComponent<Rigidbody2DComponent>(); });
				}
				
				if (!m_SelectedEntity.HasComponent<BoxCollider2DComponent>())
				{
					addComponentSafe("2D 碰撞体 (Box Collider 2D)", true, false, [&]() { m_SelectedEntity.AddComponent<BoxCollider2DComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<TriangleCollider2DComponent>())
				{
					addComponentSafe("2D 三角形碰撞体 (Triangle Collider 2D)", true, false, [&]() { m_SelectedEntity.AddComponent<TriangleCollider2DComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<TriangleRendererComponent>())
				{
					addComponentSafe("2D 三角形渲染 (Triangle Renderer)", false, false, [&]() { m_SelectedEntity.AddComponent<TriangleRendererComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<Rigidbody3DComponent>())
				{
					addComponentSafe("3D 刚体 (Rigidbody 3D)", false, true, [&]() { m_SelectedEntity.AddComponent<Rigidbody3DComponent>(); });
				}
				
				if (!m_SelectedEntity.HasComponent<BoxCollider3DComponent>())
				{
					addComponentSafe("3D 碰撞体 (Box Collider 3D)", false, true, [&]() { m_SelectedEntity.AddComponent<BoxCollider3DComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<PlayerControllerComponent>())
				{
					addComponentSafe("玩家控制 (Player Controller)", false, false, [&]() { m_SelectedEntity.AddComponent<PlayerControllerComponent>(); });
				}

				if (!m_SelectedEntity.HasComponent<ObstacleSpawnerComponent>())
				{
					addComponentSafe("障碍物生成器 (Obstacle Spawner)", false, false, [&]() { m_SelectedEntity.AddComponent<ObstacleSpawnerComponent>(); });
				}

				ImGui::EndPopup();
			}

			if (showComponentError)
			{
				ImGui::OpenPopup("Component Compatibility Error");
			}

			bool openErrorModal = true;
			if (ImGui::BeginPopupModal("Component Compatibility Error", &openErrorModal, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Component Compatibility Error! (组件兼容性冲突)");
				ImGui::Separator();
				ImGui::Text("Cannot mix 2D and 3D physics components on the same entity.");
				ImGui::Text("无法在同一个实体上混用 2D 和 3D 物理组件。");
				ImGui::Separator();
				if (ImGui::Button("确定 (OK)", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

		}
		ImGui::End();

		// Settings Panel
		ImGui::Begin("设置 (Settings)");
		auto stats = Renderer2D::GetStatistics();
		ImGui::Text("Renderer2D 统计信息:");
		ImGui::Text("绘制调用 (Draw Calls): %d", stats.DrawCalls);
		ImGui::Text("四边形数量 (Quads): %d", stats.QuadCount);
		ImGui::Text("顶点数 (Vertices): %d", stats.GetTotalVertexCount());
		ImGui::Text("索引数 (Indices): %d", stats.GetTotalIndexCount());
		ImGui::Separator();
		
		ImGui::Text("相机设置 (Camera Settings)");
		if (ImGui::RadioButton("2D 相机", !m_Is3DCamera)) m_Is3DCamera = false;
		ImGui::SameLine();
		if (ImGui::RadioButton("3D 相机", m_Is3DCamera)) m_Is3DCamera = true;
		
		ImGui::Separator();
		ImGui::Text("描边设置 (Outline Settings)");
		ImGui::ColorEdit4("描边颜色 (Outline Color)", glm::value_ptr(m_OutlineColor));
		ImGui::SliderInt("描边宽度 (Outline Width)", &m_OutlineWidth, 1, 10);
		ImGui::SliderFloat("描边强度 (Outline Intensity)", &m_OutlineIntensity, 0.1f, 5.0f);

		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("视口 (Viewport)");


		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);//当视口窗口被选中时，阻止事件传递到其他层



		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize) && viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f)
		{
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_PerspCameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(intptr_t)textureID, ImVec2{ m_ViewportSize.x,m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path fullPath = std::filesystem::path("assets") / path;
				
				if (fullPath.extension() == ".obj" || fullPath.extension() == ".fbx")
				{
					QL_CORE_INFO("Loading Model: {0}", fullPath.string());
					// Create a new entity with the dropped model
					Entity newEntity = m_ActiveScene->CreateEntity(fullPath.filename().string());
					m_ModelEntity = newEntity;
					m_Model = CreateRef<Model>(fullPath.string());
					m_SelectedEntity = newEntity;
				}
				else if (fullPath.extension() == ".png" || fullPath.extension() == ".jpg")
				{
					QL_CORE_INFO("Loading Texture: {0}", fullPath.string());
					// Create a new 2D Sprite Entity
					Entity newEntity = m_ActiveScene->CreateEntity(fullPath.filename().string());
					newEntity.AddComponent<SpriteTransformComponent>(); // Texture2D rendering handles will be added here
					m_SelectedEntity = newEntity;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Mouse picking check
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver())
		{
			if (m_HoveredEntity)
			{
				m_SelectedEntity = m_HoveredEntity;
			}
			else
			{
				m_SelectedEntity = {};
			}
		}

		if (m_ShowQuickAddPanel)
		{
			const float quickAddScale = ImGui::GetMainViewport() ? ImGui::GetMainViewport()->DpiScale : 1.0f;
			ImGui::SetNextWindowSize(ImVec2(320.0f * quickAddScale, 420.0f * quickAddScale), ImGuiCond_FirstUseEver);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f * quickAddScale, 12.0f * quickAddScale));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f * quickAddScale, 7.0f * quickAddScale));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f * quickAddScale, 8.0f * quickAddScale));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f * quickAddScale);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.15f, 0.98f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.13f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.22f, 0.26f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.27f, 0.30f, 0.35f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.31f, 0.35f, 0.40f, 1.0f));
			if (ImGui::Begin("快速添加实体 (Quick Add)", &m_ShowQuickAddPanel))
			{
				ImGui::TextUnformatted("快速添加 (Quick Add)");
				ImGui::Separator();
				ImGui::InputTextWithHint("##Search", "搜索预设体...", m_QuickAddSearchBuffer, sizeof(m_QuickAddSearchBuffer));
				ImGui::Separator();

				std::string searchStr = m_QuickAddSearchBuffer;
				std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

				auto addPrefab = [&](const std::string& name, auto instantiateFunc) {
					std::string lowerName = name;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
					if (searchStr.empty() || lowerName.find(searchStr) != std::string::npos)
					{
						if (ImGui::Button(name.c_str(), ImVec2(-1.0f, 36.0f * quickAddScale)))
						{
							instantiateFunc();
						}
					}
				};

				ImGui::BeginChild("PrefabList");
				
				addPrefab("物理立方体 (Physics Cube)", [&]() {
					Entity entity = m_ActiveScene->CreateEntity("Physics Cube");
					auto& tc = entity.GetComponent<TransformComponent>();
					// If hovered on viewport and not over gizmo, place at mouse, else origin
					if (m_HoveredEntity)
					{
						auto& hitTc = m_HoveredEntity.GetComponent<TransformComponent>();
						// very rough approximation: place on top of hovered entity
						tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(hitTc.Transform[3]) + glm::vec3(0.0f, 1.0f, 0.0f));
					}
					entity.AddComponent<CubeRendererComponent>(glm::vec4(0.8f, 0.3f, 0.2f, 1.0f));
					auto& rb = entity.AddComponent<Rigidbody3DComponent>();
					rb.Type = Rigidbody3DComponent::BodyType::Dynamic;
					rb.Mass = 1.0f;
					auto& bc = entity.AddComponent<BoxCollider3DComponent>();
					bc.HalfExtent = glm::vec3(0.5f);
					m_SelectedEntity = entity;
				});

				addPrefab("物理球体 (Physics Sphere)", [&]() {
					Entity entity = m_ActiveScene->CreateEntity("Physics Sphere");
					auto& tc = entity.GetComponent<TransformComponent>();
					if (m_HoveredEntity)
					{
						auto& hitTc = m_HoveredEntity.GetComponent<TransformComponent>();
						tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(hitTc.Transform[3]) + glm::vec3(0.0f, 1.0f, 0.0f));
					}
					entity.AddComponent<CubeRendererComponent>(glm::vec4(0.2f, 0.3f, 0.8f, 1.0f));
					auto& rb = entity.AddComponent<Rigidbody3DComponent>();
					rb.Type = Rigidbody3DComponent::BodyType::Dynamic;
					rb.Mass = 1.0f;
					auto& bc = entity.AddComponent<BoxCollider3DComponent>();
					bc.HalfExtent = glm::vec3(0.5f); // Using box collider as proxy for now
					m_SelectedEntity = entity;
				});

				addPrefab("物理演示平面 (Physics Demo Plane)", [&]() {
					Entity plane = m_ActiveScene->CreateEntity("PhysicsDemoPlane");
					auto& tc = plane.GetComponent<TransformComponent>();
					tc.Transform = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f));
					plane.AddComponent<CubeRendererComponent>(glm::vec4(0.5f, 0.8f, 0.5f, 1.0f));
					auto& rb3d = plane.AddComponent<Rigidbody3DComponent>();
					rb3d.Type = Rigidbody3DComponent::BodyType::Kinematic;
					rb3d.Mass = 1.0f;
					auto& bc3d = plane.AddComponent<BoxCollider3DComponent>();
					bc3d.HalfExtent = glm::vec3(5.0f, 0.05f, 5.0f);
					m_SelectedEntity = plane;
				});

				addPrefab("2D 物理方块 (2D Physics Box)", [&]() {
					Entity entity = m_ActiveScene->CreateEntity("2D Physics Box");
					entity.AddComponent<SpriteTransformComponent>(glm::vec4{ 0.2f,0.8f,0.2f,1.0f });
					auto& rb2d = entity.AddComponent<Rigidbody2DComponent>();
					rb2d.Type = Rigidbody2DComponent::BodyType::Dynamic;
					entity.AddComponent<BoxCollider2DComponent>();
					m_SelectedEntity = entity;
				});

				ImGui::EndChild();
			}
			ImGui::End();
			ImGui::PopStyleColor(5);
			ImGui::PopStyleVar(4);
		}

		// Gizmos
		Entity selectedEntity = m_SelectedEntity;
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(!m_Is3DCamera);
			ImGuizmo::SetDrawlist();
			
			// Fix Gizmo interactivity: Use viewport bounds instead of window width/height, 
			// because the ImGui window includes title bar and tabs, which offsets the mouse calculation!
			float windowWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
			float windowHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, windowWidth, windowHeight);

			// Camera
			glm::mat4 cameraProjection = m_Is3DCamera ? m_PerspCameraController.GetCamera().GetProjectionMatrix() : m_CameraController.GetCamera().GetProjectionMatrix();
			glm::mat4 cameraView = m_Is3DCamera ? m_PerspCameraController.GetCamera().GetViewMatrix() : m_CameraController.GetCamera().GetViewMatrix();

			// [Debug] Store matrices before manipulation to verify if perspective distortion occurs
			glm::mat4 preManipProj = cameraProjection;
			glm::mat4 preManipView = cameraView;

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.Transform;

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			// 提取物体的缩放并创建一个单位缩放的矩阵用于 Translate/Rotate 的绘制和拾取，消除缩放导致的箭头偏移
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), matrixTranslation, matrixRotation, matrixScale);
			
			glm::mat4 gizmoTransform = transform;
			if (m_GizmoType != ImGuizmo::OPERATION::SCALE)
			{
				float unitScale[3] = { 1.0f, 1.0f, 1.0f };
				ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, unitScale, glm::value_ptr(gizmoTransform));
			}

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(gizmoTransform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				// Debug output to verify if the camera matrices were accidentally modified by ImGuizmo
				glm::mat4 postManipProj = m_Is3DCamera ? m_PerspCameraController.GetCamera().GetProjectionMatrix() : m_CameraController.GetCamera().GetProjectionMatrix();
				glm::mat4 postManipView = m_Is3DCamera ? m_PerspCameraController.GetCamera().GetViewMatrix() : m_CameraController.GetCamera().GetViewMatrix();

				if (preManipProj != postManipProj || preManipView != postManipView)
				{
					QL_CORE_ERROR("Camera perspective/view distorted after object transform!");
				}

				if (m_GizmoType == ImGuizmo::OPERATION::SCALE)
				{
					tc.Transform = gizmoTransform;
				}
				else
				{
					float newTranslation[3], newRotation[3], newScale[3];
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(gizmoTransform), newTranslation, newRotation, newScale);
					ImGuizmo::RecomposeMatrixFromComponents(newTranslation, newRotation, matrixScale, glm::value_ptr(tc.Transform));
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		// Content Browser Panel
		if (m_IsContentBrowserOpen)
		{
			ImGui::Begin("内容浏览器 (Content Browser)", &m_IsContentBrowserOpen);
			if (m_CurrentDirectory != "assets")
			{
				if (ImGui::Button("<- 返回 (Back)"))
				{
					m_CurrentDirectory = std::filesystem::path(m_CurrentDirectory).parent_path().string();
				}
			}

			static float padding = 16.0f;
			static float thumbnailSize = 128.0f;
			float cellSize = thumbnailSize + padding;

			float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			ImGui::Columns(columnCount, 0, false);

			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, "assets");
				std::string filenameString = relativePath.filename().string();
				std::string extension = path.extension().string();

				Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
				
				if (!directoryEntry.is_directory())
				{
					if (extension == ".uasset") icon = m_IconUASSET;
					else if (extension == ".umap") icon = m_IconUMAP;
					else if (extension == ".fbx" || extension == ".obj" || extension == ".gltf") icon = m_IconFBX;
					else if (extension == ".wav" || extension == ".ogg" || extension == ".mp3") icon = m_IconWAV;
					else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") icon = m_IconPNG;
				}

				ImVec4 tintColor = ImVec4(1, 1, 1, 1);
				ImGui::PushID(filenameString.c_str());
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				
				if (ImGui::ImageButton((ImTextureID)(intptr_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 }, -1, ImVec4(0,0,0,0), tintColor))
				{
					if (directoryEntry.is_directory())
					{
						m_CurrentDirectory = (std::filesystem::path(m_CurrentDirectory) / path.filename()).string();
					}
				}
				
				if (ImGui::BeginDragDropSource())
				{
					auto relativePath = std::filesystem::relative(path, "assets");
					const wchar_t* itemPath = relativePath.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
					ImGui::EndDragDropSource();
				}

				ImGui::PopStyleColor();
				ImGui::PopID();

				ImGui::TextWrapped("%s", filenameString.c_str());
				ImGui::NextColumn();
			}

			ImGui::Columns(1);
			ImGui::End();
		}

		ImGui::End();

	}

}
