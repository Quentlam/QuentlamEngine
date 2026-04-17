#pragma once

#include "Quentlam/Core/Layer.h"

class Sandbox3D : public Quentlam::Layer
{
public:
	Sandbox3D();
	~Sandbox3D() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(Quentlam::Event& event) override;
	void OnUpdate(Quentlam::Timestep ts) override;
	void OnImGuiLayer() override;

private:
	Quentlam::PerspectiveCameraController m_CameraController;
	Quentlam::Ref<Quentlam::Texture2D> m_CheckerboardTexture;
	Quentlam::Ref<Quentlam::Model> m_Model;

	glm::vec4 m_CubeColor{ 0.8f, 0.2f, 0.3f, 1.0f };
};
