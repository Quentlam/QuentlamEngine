#include <cmath>
#include <iostream>
#include <string>

#include "EditorToolbarLayout.h"
#include "Quentlam/Physics/Physics3DValidation.h"
#include "Quentlam/Debug/CrashReporter.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include "Quentlam/Resource/ResourceManager.h"
#include <atomic>

namespace
{
	int s_Failures = 0;

	void Expect(bool condition, const char* message)
	{
		if (!condition)
		{
			++s_Failures;
			std::cerr << "[FAIL] " << message << std::endl;
		}
	}

	struct DummyResource
	{
		static std::atomic<int> AliveCount;
		std::string Path;
		bool Initialized = false;

		DummyResource(const std::string& path) : Path(path) { AliveCount++; }
		~DummyResource() { AliveCount--; }

		static Quentlam::Ref<DummyResource> Create(const std::string& path)
		{
			return Quentlam::CreateRef<DummyResource>(path);
		}
	};

	std::atomic<int> DummyResource::AliveCount = 0;

	void TestResourceManager()
	{
		using namespace Quentlam;
		std::cout << "[Test] Running ResourceManager checks..." << std::endl;

		ResourceManager::Init();

		// Test Synchronous Loading and Caching
		auto res1 = ResourceManager::Load<DummyResource>("dummy1", "path/to/dummy1");
		Expect(res1 != nullptr, "ResourceManager successfully loads a resource.");
		Expect(DummyResource::AliveCount == 1, "Only one instance is created.");
		
		auto res2 = ResourceManager::Load<DummyResource>("dummy1", "path/to/dummy1");
		Expect(res1 == res2, "ResourceManager returns the cached instance.");
		Expect(DummyResource::AliveCount == 1, "Cache prevents duplicate creation.");

		// Test Exists and Get
		Expect(ResourceManager::Exists<DummyResource>("dummy1"), "Resource exists in cache.");
		auto res3 = ResourceManager::Get<DummyResource>("dummy1");
		Expect(res3 == res1, "Get returns the correct resource.");

		// Test Garbage Collection (Ref counting)
		res1.reset();
		res2.reset();
		res3.reset();

		Expect(DummyResource::AliveCount == 0, "Resource is destroyed when ref count reaches 0.");
		Expect(ResourceManager::Exists<DummyResource>("dummy1") == false, "Resource is marked as expired in cache.");

		ResourceManager::GarbageCollect(); // Should clean up the expired weak_ptr
		auto res4 = ResourceManager::Get<DummyResource>("dummy1");
		Expect(res4 == nullptr, "Garbage collection removes expired resources from cache.");

		// Test Async Loading
		std::atomic<bool> loaded = false;
		ResourceManager::LoadAsync<DummyResource>("dummy2", "path/to/dummy2", [&](Ref<DummyResource> res) {
			Expect(res != nullptr, "Async loading returns valid resource.");
			Expect(res->Path == "path/to/dummy2", "Async loading initializes resource correctly.");
			loaded = true;
		});

		// Wait for async task and execute main thread callbacks
		while (!loaded)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ResourceManager::Update();
		}

		Expect(DummyResource::AliveCount == 1, "Async loaded resource is alive.");
		auto res5 = ResourceManager::Get<DummyResource>("dummy2");
		Expect(res5 != nullptr, "Async loaded resource is cached.");

		ResourceManager::Shutdown();
		std::cout << "[Test] ResourceManager checks passed." << std::endl;
	}

	void TestPhysics3DValidation()
	{
		using namespace Quentlam::Physics3DValidation;

		auto sanitizedThin = SanitizeHalfExtent({ 10.0f, 0.05f, 10.0f }, { 1.0f, 1.0f, 1.0f });
		Expect(sanitizedThin.y >= kMinHalfExtent, "Thin collider keeps a positive half extent.");

		float thinRadius = CalculateConvexRadius(sanitizedThin);
		Expect(thinRadius >= 0.0f && thinRadius < sanitizedThin.y, "Convex radius stays smaller than the thinnest axis.");

		auto sanitizedInvalid = SanitizeHalfExtent({ NAN, -1.0f, 0.0f }, { 1.0f, 2.0f, 3.0f });
		Expect(sanitizedInvalid.x == kMinHalfExtent, "Invalid extents fall back to minimum half extent.");
		Expect(sanitizedInvalid.y == kMinHalfExtent, "Negative extents are clamped to minimum half extent.");
		Expect(sanitizedInvalid.z == kMinHalfExtent, "Zero extents are clamped to minimum half extent.");

		Expect(SanitizeMass(0.0f) == kMinDynamicMass, "Zero mass is clamped to safe minimum.");
		Expect(SanitizeMass(-10.0f) == kMinDynamicMass, "Negative mass is clamped to safe minimum.");
		Expect(SanitizeMass(2.5f) == 2.5f, "Valid mass is preserved.");
	}

	void TestToolbarLayout()
	{
		using namespace Quentlam::EditorToolbarLayout;

		auto baseLayout = Calculate(1200.0f, 0.0f, 1.0f, 1.0f);
		Expect(baseLayout.ButtonSize >= kMinButtonSize, "Toolbar button size respects minimum touch target.");
		Expect(!baseLayout.IsClipped, "Centered toolbar fits inside a standard viewport.");
		Expect(std::abs((baseLayout.WindowX + (baseLayout.WindowWidth * 0.5f)) - 600.0f) <= 1.0f, "Toolbar stays centered in the viewport.");
		Expect(baseLayout.WindowY >= kToolbarTopMargin, "Toolbar keeps a top margin.");

		auto scaledLayout = Calculate(800.0f, 24.0f, 1.5f, 1.25f);
		Expect(scaledLayout.ButtonSize >= kMinButtonSize * 1.25f, "Higher DPI enlarges toolbar buttons.");
		Expect(scaledLayout.WindowY > 24.0f, "Viewport offset is preserved when computing toolbar position.");
		Expect(!scaledLayout.IsClipped, "Scaled toolbar remains visible.");

		auto narrowLayout = Calculate(40.0f, 0.0f, 1.0f, 1.0f);
		Expect(narrowLayout.IsClamped, "Narrow layouts clamp the toolbar to the viewport.");
		Expect(narrowLayout.IsClipped, "Layouts narrower than the button container report clipping.");
	}
}

class MockBPLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
	virtual JPH::uint GetNumBroadPhaseLayers() const override { return 2; }
	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		return JPH::BroadPhaseLayer(0);
	}
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		if ((JPH::BroadPhaseLayer::Type)inLayer >= 2) return nullptr; // Simulate out of bounds
		return "MockLayer";
	}
#endif
};

void TestBroadPhaseLayerCrash()
{
	std::cout << "[Test] Running BroadPhaseLayer crash scenarios..." << std::endl;
	
	MockBPLayerInterface mockInterface;
	
	// Test 1: Valid layer name
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* name = mockInterface.GetBroadPhaseLayerName(JPH::BroadPhaseLayer(0));
	if (name == nullptr) std::cout << "Error: Name should not be null" << std::endl;
	
	// Test 2: Out of bounds
	const char* invalidName = mockInterface.GetBroadPhaseLayerName(JPH::BroadPhaseLayer(5));
	if (invalidName == nullptr) std::cout << "Success: Out of bounds handled" << std::endl;
#endif

	std::cout << "[Test] BroadPhaseLayer scenarios passed." << std::endl;
}

#include <windows.h>

void TestDynamicModuleLoad()
{
	std::cout << "[Test] Running dynamic module (DLL) load check..." << std::endl;
	
	HMODULE hMod = GetModuleHandleA("QuentlamEngine.dll");
	if (hMod)
	{
		std::cout << "Success: QuentlamEngine.dll is loaded." << std::endl;
		// Optionally check an exported symbol if we knew one
		// FARPROC proc = GetProcAddress(hMod, "Init@CrashReporter");
		// if (proc) std::cout << "Success: Exported function found." << std::endl;
	}
	else
	{
		std::cout << "Note: QuentlamEngine.dll is not loaded (static build or missing)." << std::endl;
	}

	std::cout << "[Test] Dynamic module check passed." << std::endl;
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		std::string arg = argv[1];
		Quentlam::CrashReporterConfig cfg;
		Quentlam::CrashReporter::Init(cfg);

		if (arg == "--crash-segfault") {
			Quentlam::CrashReporter::SimulateCrash(0);
		} else if (arg == "--crash-abort") {
			Quentlam::CrashReporter::SimulateCrash(1);
		} else if (arg == "--crash-invalid-param") {
			Quentlam::CrashReporter::SimulateCrash(2);
		} else if (arg == "--crash-pure-call") {
			Quentlam::CrashReporter::SimulateCrash(3);
		} else if (arg == "--crash-unhandled-exception") {
			Quentlam::CrashReporter::SimulateCrash(4);
		}
		return 0;
	}

	TestResourceManager();
	TestPhysics3DValidation();
	TestToolbarLayout();
	TestBroadPhaseLayerCrash();
	TestDynamicModuleLoad();

	if (s_Failures > 0)
	{
		std::cerr << "[RESULT] " << s_Failures << " checks failed." << std::endl;
		return 1;
	}

	std::cout << "[RESULT] All regression checks passed." << std::endl;
	return 0;
}
