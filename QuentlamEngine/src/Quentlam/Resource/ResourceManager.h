#pragma once
#include "Quentlam/Core/Base.h"
#include "Quentlam/Core/ThreadPool.h"
#include "Quentlam/Core/Log.h"

#include <unordered_map>
#include <string>
#include <mutex>
#include <typeindex>
#include <functional>
#include <vector>
#include <memory>

#include "Quentlam/Renderer/Texture.h"
#include "Quentlam/Renderer/Model.h"

namespace Quentlam {

	class QUENTLAM_API ResourceManager
	{
	public:
		static void Init();
		static void Shutdown();
		static void Update(); // Execute main thread tasks

		// Load resource synchronously
		template<typename T, typename... Args>
		static Ref<T> Load(const std::string& name, const std::string& path, Args&&... args)
		{
			auto& cache = GetCache<T>();
			{
				std::lock_guard<std::mutex> lock(cache.Mutex);
				if (cache.Pool.find(name) != cache.Pool.end())
				{
					if (auto resource = std::static_pointer_cast<T>(cache.Pool[name].lock()))
					{
						return resource;
					}
				}
			}

			// Not in cache or expired, create new
			Ref<T> resource = CreateResource<T>(path, std::forward<Args>(args)...);
			
			if (resource)
			{
				std::lock_guard<std::mutex> lock(cache.Mutex);
				cache.Pool[name] = resource;
			}
			return resource;
		}

		// Load resource asynchronously
		template<typename T, typename... Args>
		static void LoadAsync(const std::string& name, const std::string& path, std::function<void(Ref<T>)> onComplete, Args... args)
		{
			auto& cache = GetCache<T>();
			{
				std::lock_guard<std::mutex> lock(cache.Mutex);
				if (cache.Pool.find(name) != cache.Pool.end())
				{
					if (auto resource = std::static_pointer_cast<T>(cache.Pool[name].lock()))
					{
						if (onComplete) onComplete(resource);
						return;
					}
				}
			}

			PerformAsyncLoad<T>(name, path, onComplete, std::forward<Args>(args)...);
		}

		// Get resource if it exists and is loaded
		template<typename T>
		static Ref<T> Get(const std::string& name)
		{
			auto& cache = GetCache<T>();
			std::lock_guard<std::mutex> lock(cache.Mutex);
			if (cache.Pool.find(name) != cache.Pool.end())
			{
				return std::static_pointer_cast<T>(cache.Pool[name].lock());
			}
			return nullptr;
		}

		template<typename T>
		static bool Exists(const std::string& name)
		{
			auto& cache = GetCache<T>();
			std::lock_guard<std::mutex> lock(cache.Mutex);
			if (cache.Pool.find(name) != cache.Pool.end())
			{
				return !cache.Pool[name].expired();
			}
			return false;
		}

		// Force remove unused resources (weak_ptr expired)
		static void GarbageCollect();

		// Submit task to main thread (useful for GPU initialization after async I/O)
		static void SubmitMainThreadTask(std::function<void()> task);

		// Get thread pool for async tasks
		static ThreadPool& GetThreadPool() { return *s_ThreadPool; }

	private:
		static void LoadTexture2DAsyncImpl(const std::string& name, const std::string& path, std::function<void(Ref<Texture2D>)> onComplete);
		static void LoadModelAsyncImpl(const std::string& name, const std::string& path, std::function<void(Ref<Model>)> onComplete);

		template<typename T, typename... Args>
		static void PerformAsyncLoad(const std::string& name, const std::string& path, std::function<void(Ref<T>)> onComplete, Args... args)
		{
			if constexpr (std::is_same_v<T, Texture2D>)
			{
				LoadTexture2DAsyncImpl(name, path, onComplete);
			}
			else if constexpr (std::is_same_v<T, Model>)
			{
				LoadModelAsyncImpl(name, path, onComplete);
			}
			else
			{
				s_ThreadPool->Enqueue([name, path, onComplete, args...]() mutable {
					Ref<T> resource = CreateResource<T>(path, std::forward<Args>(args)...);

					if (resource)
					{
						auto& cacheRef = GetCache<T>();
						std::lock_guard<std::mutex> lock(cacheRef.Mutex);
						cacheRef.Pool[name] = resource;
					}

					if (onComplete)
					{
						SubmitMainThreadTask([resource, onComplete]() {
							onComplete(resource);
						});
					}
				});
			}
		}

		template<typename T, typename... Args>
		static Ref<T> CreateResource(const std::string& path, Args&&... args)
		{
			if constexpr (requires { T::Create(path, std::forward<Args>(args)...); })
			{
				return T::Create(path, std::forward<Args>(args)...);
			}
			else
			{
				return CreateRef<T>(path, std::forward<Args>(args)...);
			}
		}

		struct ResourceCache {
			std::unordered_map<std::string, std::weak_ptr<void>> Pool;
			std::mutex Mutex;
		};

		template<typename T>
		static ResourceCache& GetCache()
		{
			std::lock_guard<std::mutex> lock(s_CachesMutex);
			auto it = s_Caches.find(typeid(T));
			if (it == s_Caches.end())
			{
				s_Caches[typeid(T)] = CreateScope<ResourceCache>();
			}
			return *s_Caches[typeid(T)];
		}

	private:
		static std::unordered_map<std::type_index, Scope<ResourceCache>> s_Caches;
		static std::mutex s_CachesMutex;

		static Scope<ThreadPool> s_ThreadPool;

		static std::vector<std::function<void()>> s_MainThreadTasks;
		static std::mutex s_MainThreadTasksMutex;
	};
}