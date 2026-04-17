#pragma once

#include "Quentlam/Core/Base.h"
#include <string>
#include <functional>

namespace Quentlam {

	struct CrashReporterConfig
	{
		bool Enable = true;
		float SampleRate = 1.0f; // 0.0 to 1.0
		uint32_t MaxLocalDumps = 10;
		std::string AppVersion = "1.0.0";
		std::string BuildNumber = "0001";
		std::string Channel = "Release";
		std::string UserId = "Anonymous";
		std::string UploadUrl = "http://localhost:8080/api/crash/upload";
	};

	class QUENTLAM_API CrashReporter
	{
	public:
		static void Init(const CrashReporterConfig& config = CrashReporterConfig());
		static void Shutdown();

		// Manually trigger a crash for testing
		static void SimulateCrash(int type);

		// Dynamic control
		static void SetSceneTag(const std::string& sceneName);
		static void SetEnabled(bool enabled);
		static bool IsEnabled();

		// Expose for exception handlers
		static void HandleCrash(struct _EXCEPTION_POINTERS* exceptionInfo);
		static void* s_PreviousFilter;

	private:
		static void SetupExceptionHandlers();
		static void RestoreExceptionHandlers();
		
		static void WriteMiniDump(struct _EXCEPTION_POINTERS* exceptionInfo, const std::string& dumpPath);
		static void WriteMetadataAndLogs(const std::string& jsonPath);
		static void TriggerUploadProcess(const std::string& dumpPath, const std::string& jsonPath);

		static CrashReporterConfig s_Config;
		static std::string s_CurrentScene;
		static bool s_Initialized;
	};

}
