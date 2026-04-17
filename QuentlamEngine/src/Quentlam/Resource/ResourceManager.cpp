#include "qlpch.h"
#include "ResourceManager.h"
#include "stb_image.h"

namespace Quentlam {

	std::unordered_map<std::type_index, Scope<ResourceManager::ResourceCache>> ResourceManager::s_Caches;
	std::mutex ResourceManager::s_CachesMutex;

	Scope<ThreadPool> ResourceManager::s_ThreadPool;

	std::vector<std::function<void()>> ResourceManager::s_MainThreadTasks;
	std::mutex ResourceManager::s_MainThreadTasksMutex;

	void ResourceManager::Init()
	{
		QL_CORE_INFO("Initializing ResourceManager...");
		s_ThreadPool = CreateScope<ThreadPool>(4); // 4 background threads for loading
	}

	void ResourceManager::Shutdown()
	{
		QL_CORE_INFO("Shutting down ResourceManager...");
		s_ThreadPool.reset();
		s_Caches.clear();
	}

	void ResourceManager::Update()
	{
		std::vector<std::function<void()>> tasks;
		{
			std::lock_guard<std::mutex> lock(s_MainThreadTasksMutex);
			tasks.swap(s_MainThreadTasks);
		}

		for (auto& task : tasks)
		{
			task();
		}
	}

	void ResourceManager::GarbageCollect()
	{
		std::lock_guard<std::mutex> lock(s_CachesMutex);
		for (auto& [type, cache] : s_Caches)
		{
			std::lock_guard<std::mutex> cacheLock(cache->Mutex);
			for (auto it = cache->Pool.begin(); it != cache->Pool.end(); )
			{
				if (it->second.expired())
				{
					it = cache->Pool.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	void ResourceManager::SubmitMainThreadTask(std::function<void()> task)
	{
		std::lock_guard<std::mutex> lock(s_MainThreadTasksMutex);
		s_MainThreadTasks.push_back(task);
	}

	void ResourceManager::LoadTexture2DAsyncImpl(const std::string& name, const std::string& path, std::function<void(Ref<Texture2D>)> onComplete)
	{
		s_ThreadPool->Enqueue([name, path, onComplete]() {
			int width, height, channels;
			stbi_set_flip_vertically_on_load(1);
			stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

			if (data)
			{
				SubmitMainThreadTask([name, data, width, height, channels, onComplete]() {
					auto resource = Texture2D::Create();
					if (resource)
					{
						resource->LoadFromData(data, width, height, channels);
						auto& cache = GetCache<Texture2D>();
						{
							std::lock_guard<std::mutex> lock(cache.Mutex);
							cache.Pool[name] = resource;
						}
					}
					stbi_image_free(data);
					if (onComplete) onComplete(resource);
				});
			}
			else
			{
				QL_CORE_ERROR("Failed to load texture asynchronously: {0}", path);
				if (onComplete) onComplete(nullptr);
			}
		});
	}

	void ResourceManager::LoadModelAsyncImpl(const std::string& name, const std::string& path, std::function<void(Ref<Model>)> onComplete)
	{
		s_ThreadPool->Enqueue([name, path, onComplete]() {
			// Load from file without initializing GPU buffers (initGPU = false)
			auto resource = CreateRef<Model>(path, false);
			
			SubmitMainThreadTask([name, resource, onComplete]() {
				if (resource)
				{
					resource->InitGPU();
					auto& cache = GetCache<Model>();
					{
						std::lock_guard<std::mutex> lock(cache.Mutex);
						cache.Pool[name] = resource;
					}
				}
				if (onComplete) onComplete(resource);
			});
		});
	}

}