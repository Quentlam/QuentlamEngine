#pragma once
#include <memory>


#ifdef _WIN32
		#ifdef _WIN64

			#define QL_PALTFORM_WINDOWS

		#else

		#error "x86 Builds are not supported!"

		#endif

#elif	defined(__APPLE__) || defined(__MACH__)
		#include<TargetConditionals.h>
		#if TARGET_IPHONE_SIMULATOR == 1
			#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define QL_PALTFORM_IOS
		#error "IOS is not supported!"
	#elif TAGET_OS_MAC == 1
		#define QL_PALTFORM_MACOS
		#error "Mac OS is not supported!"
	#else
		#error "Unkonwn Apple platform!"
   #endif
	#elif defined(__ANDROID__)
		#define QL_PLATFORM_ANDROID
		#error "Android is not supported!"
	#elif define(__linux__)
		#define QL_PLATFORM_LINUX
		#error "Linux is not supported!"
    #else
		#error "Unkonwn platform!"
#endif



#ifdef QL_PLATFORM_WINDOWS
#if QL_DYNAMIC_LINK
	#ifdef QL_BUILD_DLL
		#define QUENTLAM_API __declspec(dllexport)
		#define IMGUI_API 	__declspec(dllexport)
	#else
		#define QUENTLAM_API __declspec(dllimport)
		#define IMGUI_API 	__declspec(dllimport)
	#endif
#else
	#define QUENTLAM_API
#endif

#else
	#error Quentlam only support Windows!
#endif

#ifdef QL_DEBUG
	#define QL_ENABLE_ASSERTS
#endif

#ifdef QL_ENABLE_ASSERTS
#define QL_ASSERTS(x,...) {if(!(x)) {QL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#define QL_CORE_ASSERTS(x,...) {if(!(x)) {QL_CORE_ERROR("Assertion Failed: {0}",__VA_ARGS__); __debugbreak();}}
#else
#define QL_ASSERTS(x,...)
#define QL_CORE_ASSERTS(x,...)
#endif


#define BIT(x)    (1 << x)

#define QL_BIND_EVENT_FN(fn) std::bind(&fn,this,std::placeholders::_1)

namespace Quentlam
{

	template<typename T>
	using Scope = std::unique_ptr<T>;//唯一指针

	template<typename T>
	using Ref = std::shared_ptr<T>;//共享指针
	template<typename T,typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}



}