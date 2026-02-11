#pragma once


#ifdef QL_PLATFORM_WINDOWS


extern Quentlam::Application* Quentlam::CreateApplication();
	
int main(int argc,char** argv)
{
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

	return 0;
}





#endif



