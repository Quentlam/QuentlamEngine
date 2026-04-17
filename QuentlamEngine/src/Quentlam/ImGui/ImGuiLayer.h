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
		void OnEvent(Event& e)override;
		void Begin();
		void End();


		void BlockEvents(bool block) { m_BlockEvents = block; }
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;

	};

}