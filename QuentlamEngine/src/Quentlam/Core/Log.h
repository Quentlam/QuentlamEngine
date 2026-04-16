#pragma once
#include "qlpch.h"
#include "spdlog/spdlog.h"
#include "Base.h"

namespace Quentlam
{
	 class QUENTLAM_API Log
	{

	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetBaseLogger() { return s_BaseLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }


	private:
		static std::shared_ptr<spdlog::logger> s_BaseLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};


}




//Base Logger macros
#define QL_Base_TRACE(...)   ::Quentlam::Log::GetBaseLogger()->trace(__VA_ARGS__)
#define QL_Base_INFO(...)    ::Quentlam::Log::GetBaseLogger()->info(__VA_ARGS__)
#define QL_CORE_WARN(...)    ::Quentlam::Log::GetBaseLogger()->warn(__VA_ARGS__)
#define QL_Base_ERROR(...)   ::Quentlam::Log::GetBaseLogger()->error(__VA_ARGS__)
#define QL_Base_FATAL(...)   ::Quentlam::Log::GetBaseLogger()->fatal(__VA_ARGS__)


//Client Logger macros
#define QL_TRACE(...)   ::Quentlam::Log::GetClientLogger()->trace(__VA_ARGS__)
#define QL_INFO(...)    ::Quentlam::Log::GetClientLogger()->info(__VA_ARGS__)
#define QL_WARN(...)    ::Quentlam::Log::GetClientLogger()->warn(__VA_ARGS__)
#define QL_ERROR(...)   ::Quentlam::Log::GetClientLogger()->error(__VA_ARGS__)
#define QL_FATAL(...)   ::Quentlam::Log::GetClientLogger()->fatal(__VA_ARGS__)
