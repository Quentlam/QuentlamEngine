#include <Quentlam.h>
#include "Quentlam/Debug/CrashReporter.h"
#include "GameLayer.h"
#include <string>

#ifdef QL_PLATFORM_WINDOWS
#include <crtdbg.h>
#endif

class ParkourGameApp : public Quentlam::Application
{
public:
	ParkourGameApp(const std::string& name, int width, int height)
		: Quentlam::Application(name, width, height)
	{
		PushLayer(new GameLayer());
	}

	~ParkourGameApp()
	{
	}
};

int main(int argc, char** argv)
{
#ifdef QL_PLATFORM_WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int width = 1280;
	int height = 720;
	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "--test-collision")
		{
			Quentlam::Log::Init();
			Level::RunCollisionTests();
			return 0;
		}
		if (std::string(argv[i]) == "-width" && i + 1 < argc)
			width = std::stoi(argv[i + 1]);
		if (std::string(argv[i]) == "-height" && i + 1 < argc)
			height = std::stoi(argv[i + 1]);
	}

	Quentlam::CrashReporterConfig crashConfig;
	crashConfig.Enable = true;
	Quentlam::CrashReporter::Init(crashConfig);

	Quentlam::Log::Init();

	QL_PROFILE_BEGIN_SESSION("Startup", "QuentlamProfile-Startup.json");
	QL_CORE_WARN("Initialized Log!");
	
	auto app = new ParkourGameApp("ParkourGame", width, height);
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

