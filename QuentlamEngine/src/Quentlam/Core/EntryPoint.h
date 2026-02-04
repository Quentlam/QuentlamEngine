#pragma once


#ifdef QL_PLATFORM_WINDOWS


extern Quentlam::Application* Quentlam::CreateApplication();
	
int main(int argc,char** argv)
{
	Quentlam::Log::Init();
	QL_CORE_WARN("Initialized Log!");
	auto app = Quentlam::CreateApplication();
	app->Run();
	delete app;

	return 0;
}





#endif



