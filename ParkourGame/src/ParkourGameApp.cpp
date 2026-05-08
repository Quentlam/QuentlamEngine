#include <Quentlam.h>
#include "Quentlam/Core/EntryPoint.h"
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

Quentlam::Application* Quentlam::CreateApplication()
{
	int width = 1280;
	int height = 720;
	return new ParkourGameApp("ParkourGame", width, height);
}

