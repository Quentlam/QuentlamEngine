#pragma once
#include "qlpch.h"
#include "../Core/Layer.h"


#include "Quentlam/Events/ApplicationEvent.h"
#include "Quentlam/Events/KeyEvent.h"
#include "Quentlam/Events/MouseEvent.h"


namespace Quentlam
{

	class QUENTLAM_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach()override;
		void OnDetach()override;
		
		void OnImGuiLayer()override;
		void Begin();
		void End();

	private:
		float m_Time = 0.0f;

	};

}