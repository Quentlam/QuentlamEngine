#pragma once


#ifdef QL_PLATFORM_WINDOWS

#include "Quentlam/Debug/CrashReporter.h"

extern Quentlam::Application* Quentlam::CreateApplication();
	
int main(int argc,char** argv)
{
	Quentlam::CrashReporterConfig crashConfig;
	crashConfig.Enable = true;
	Quentlam::CrashReporter::Init(crashConfig);

	Quentlam::Log::Init();

	QL_PROFILE_BEGIN_SESSION("Startup", "QuentlamProfile-Startup.json");
	QL_CORE_WARN("Initialized Log!");
	auto app = Quentlam::CreateApplication();
	QL_PROFILE_END_SESSION();


	QL_PROFILE_BEGIN_SESSION("Runtime", "QuentlamProfile-Runtime.json");
	app->Run();
	QL_PROFILE_END_SESSION();


	QL_PROFILE_BEGIN_SESSION("Shutdown", "QuentlamProfile-Shutdown.json");
	delete app;
	QL_PROFILE_END_SESSION();

	Quentlam::CrashReporter::Shutdown();

	return 0;
}





#endif



