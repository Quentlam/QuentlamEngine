#pragma once
#include "Quentlam/Core/Base.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

namespace Quentlam {

	class QUENTLAM_API Model {
	public:
		Model(const std::string& path, bool initGPU = true);

		void InitGPU(); // Initialize all meshes on main thread
		void Draw() const;

	private:
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	private:
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;
	};
}
