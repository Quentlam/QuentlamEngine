#include "qlpch.h"
#include "CrashReporter.h"
#include "Quentlam/Core/Log.h"
#include <csignal>

#ifdef QL_PLATFORM_WINDOWS
#include <windows.h>
#include <DbgHelp.h>
#include <crtdbg.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>
#include <filesystem>
#include <Shlobj.h>

#pragma comment(lib, "DbgHelp.lib")

namespace Quentlam {

	CrashReporterConfig CrashReporter::s_Config;
	std::string CrashReporter::s_CurrentScene = "Unknown";
	bool CrashReporter::s_Initialized = false;
	void* CrashReporter::s_PreviousFilter = nullptr;

	static LONG WINAPI CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
	{
		if (CrashReporter::IsEnabled())
		{
			// Try to handle crash
			CrashReporter::HandleCrash(ExceptionInfo);
		}

		if (CrashReporter::s_PreviousFilter)
		{
			return ((PTOP_LEVEL_EXCEPTION_FILTER)CrashReporter::s_PreviousFilter)(ExceptionInfo);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	static std::string WStringToString(const wchar_t* wstr)
	{
		if (!wstr) return "unknown";
		std::wstring ws(wstr);
		return std::string(ws.begin(), ws.end());
	}

	static void CustomInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
	{
		QL_CORE_FATAL("Invalid parameter detected in {0}, function {1}, line {2}", 
			WStringToString(file), 
			WStringToString(function), 
			line);

		// Raise an exception so that the UnhandledExceptionFilter catches it
		RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	static void CustomPureCallHandler()
	{
		QL_CORE_FATAL("Pure virtual function call detected");
		RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	static void SignalHandler(int signum)
	{
		QL_CORE_FATAL("Signal {0} received", signum);
		// Not perfect, as EXCEPTION_POINTERS is missing, but better than nothing
		RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	void CrashReporter::Init(const CrashReporterConfig& config)
	{
		if (s_Initialized) return;

		s_Config = config;
		s_Initialized = true;

		if (s_Config.Enable)
		{
			SetupExceptionHandlers();
		}
	}

	void CrashReporter::Shutdown()
	{
		if (!s_Initialized) return;
		RestoreExceptionHandlers();
		s_Initialized = false;
	}

	void CrashReporter::SetEnabled(bool enabled)
	{
		if (s_Config.Enable == enabled) return;

		s_Config.Enable = enabled;
		if (enabled)
		{
			SetupExceptionHandlers();
		}
		else
		{
			RestoreExceptionHandlers();
		}
	}

	bool CrashReporter::IsEnabled()
	{
		return s_Config.Enable;
	}

	void CrashReporter::SetSceneTag(const std::string& sceneName)
	{
		s_CurrentScene = sceneName;
	}

	void CrashReporter::SetupExceptionHandlers()
	{
		s_PreviousFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
		_set_invalid_parameter_handler(CustomInvalidParameterHandler);
		_set_purecall_handler(CustomPureCallHandler);

		signal(SIGABRT, SignalHandler);
		signal(SIGFPE, SignalHandler);
		signal(SIGILL, SignalHandler);
		signal(SIGSEGV, SignalHandler);
		signal(SIGTERM, SignalHandler);
	}

	void CrashReporter::RestoreExceptionHandlers()
	{
		SetUnhandledExceptionFilter((PTOP_LEVEL_EXCEPTION_FILTER)s_PreviousFilter);
		_set_invalid_parameter_handler(nullptr);
		_set_purecall_handler(nullptr);
		
		signal(SIGABRT, SIG_DFL);
		signal(SIGFPE, SIG_DFL);
		signal(SIGILL, SIG_DFL);
		signal(SIGSEGV, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
	}

	void CrashReporter::HandleCrash(EXCEPTION_POINTERS* exceptionInfo)
	{
		// Sampling check
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		if (dis(gen) > s_Config.SampleRate)
		{
			return; // Skip reporting based on sample rate
		}

		// Ensure directory exists
		std::string crashDir = "CrashDumps";
		if (!std::filesystem::exists(crashDir))
		{
			std::filesystem::create_directory(crashDir);
		}

		// Generate timestamp string
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
		std::string timeStr = oss.str();

		std::string baseName = crashDir + "/Crash_" + timeStr;
		std::string dumpPath = baseName + ".dmp";
		std::string jsonPath = baseName + ".json";

		// Write MiniDump
		WriteMiniDump(exceptionInfo, dumpPath);

		// Write Metadata and Logs
		WriteMetadataAndLogs(jsonPath);

		// Trigger Upload (Spawn background process)
		TriggerUploadProcess(dumpPath, jsonPath);

		QL_CORE_FATAL("Engine crashed! Dump saved to {0}", dumpPath);
	}

	void CrashReporter::WriteMiniDump(EXCEPTION_POINTERS* exceptionInfo, const std::string& dumpPath)
	{
		HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION mdei;
			mdei.ThreadId = GetCurrentThreadId();
			mdei.ExceptionPointers = exceptionInfo;
			mdei.ClientPointers = FALSE;

			MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithProcessThreadData | MiniDumpWithFullMemoryInfo);

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, &mdei, NULL, NULL);
			CloseHandle(hFile);
		}
	}

	void CrashReporter::WriteMetadataAndLogs(const std::string& jsonPath)
	{
		std::ofstream file(jsonPath);
		if (!file.is_open()) return;

		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

		// Basic device info
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		file << "{\n";
		file << "  \"timestamp\": \"" << oss.str() << "\",\n";
		file << "  \"appVersion\": \"" << s_Config.AppVersion << "\",\n";
		file << "  \"buildNumber\": \"" << s_Config.BuildNumber << "\",\n";
		file << "  \"channel\": \"" << s_Config.Channel << "\",\n";
		file << "  \"userId\": \"" << s_Config.UserId << "\",\n";
		file << "  \"scene\": \"" << s_CurrentScene << "\",\n";
		file << "  \"device\": {\n";
		file << "    \"processors\": " << sysInfo.dwNumberOfProcessors << "\n";
		file << "  }\n";
		file << "}\n";

		file.close();
	}

	void CrashReporter::TriggerUploadProcess(const std::string& dumpPath, const std::string& jsonPath)
	{
		// Launch background python/powershell script to handle zip and upload
		// We pass the config via command line args
		std::string cmd = "powershell -NoProfile -ExecutionPolicy Bypass -File tools/crash_server.ps1 -Action Upload -DumpPath \"" + dumpPath + "\" -JsonPath \"" + jsonPath + "\" -Url \"" + s_Config.UploadUrl + "\"";
		
		STARTUPINFOA si = { sizeof(STARTUPINFOA) };
		PROCESS_INFORMATION pi;
		if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	void CrashReporter::SimulateCrash(int type)
	{
		switch (type)
		{
		case 0: // Segfault
		{
			int* ptr = nullptr;
			*ptr = 42;
			break;
		}
		case 1: // Abort
			abort();
			break;
		case 2: // Invalid Parameter
			_invalid_parameter(L"Simulated", L"SimulateCrash", L"CrashReporter.cpp", 1, 0);
			break;
		case 3: // Pure call
			_purecall();
			break;
		case 4: // Throw unhandled exception
			throw std::runtime_error("Simulated unhandled C++ exception");
			break;
		default:
			break;
		}
	}

}
#endif // QL_PLATFORM_WINDOWS
