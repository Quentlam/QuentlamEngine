#pragma once
#include "qlpch.h"
#include "spdlog/spdlog.h"
#include "Core.h"

namespace Quentlam
{
	 class QUENTLAM_API Log
	{

	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }


	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};


}




//Core Logger macros
#define QL_CORE_TRACE(...)   ::Quentlam::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define QL_CORE_INFO(...)    ::Quentlam::Log::GetCoreLogger()->info(__VA_ARGS__)
#define QL_CORE_WARN(...)    ::Quentlam::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define QL_CORE_ERROR(...)   ::Quentlam::Log::GetCoreLogger()->error(__VA_ARGS__)
#define QL_CORE_FATAL(...)   ::Quentlam::Log::GetCoreLogger()->fatal(__VA_ARGS__)


//Client Logger macros
#define QL_TRACE(...)   ::Quentlam::Log::GetClientLogger()->trace(__VA_ARGS__)
#define QL_INFO(...)    ::Quentlam::Log::GetClientLogger()->info(__VA_ARGS__)
#define QL_WARN(...)    ::Quentlam::Log::GetClientLogger()->warn(__VA_ARGS__)
#define QL_ERROR(...)   ::Quentlam::Log::GetClientLogger()->error(__VA_ARGS__)
#define QL_FATAL(...)   ::Quentlam::Log::GetClientLogger()->fatal(__VA_ARGS__)
