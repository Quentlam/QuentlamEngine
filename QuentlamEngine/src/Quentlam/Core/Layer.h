#pragma once
#include "qlpch.h"
#include "Quentlam/Core/Core.h"
#include "Quentlam/Events/Event.h"
#include "Quentlam/Core/Timestep.h"

namespace Quentlam
{
	class QUENTLAM_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiLayer() {}
		inline const std::string& GetName() const { return m_DebugName; }

	private:
		std::string m_DebugName;

	};	


}
