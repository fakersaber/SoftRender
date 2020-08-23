#include <Public/YRenderLabConfig.h>

#include <Public/3rdPart/assimp/pbrmaterial.h>
#include <Public/Scene/AssimpLoader.h>
#include <Public/Scene/Yobject.h>
#include <Public/Scene/TransformComponent.h>
#include <Public/Scene/MeshComponent.h>
#include <Public/Basic/Mesh/TriMesh.h>
#include <Public/Basic/MaterialComponent.h>
#include <Public/Basic/BSDF_StandardPBR.h>
#include <Public/Basic/StrApi.h>
#include <Public/Basic/Image/Image.h>
#include <Public/YGM/Vector2.hpp>

#include <iostream>


namespace AssimpLoader {

	const std::shared_ptr<YObject> Load(const std::string& path)
	{
		bool isSupport = false;
		const std::string suffix[] = { ".FBX",".obj",".fbx" };
		for (auto& temp : suffix) {
			if (StrAPI::Is_suffix(path, temp)) {
				isSupport = true;
				break;
			}
		}
		if (!isSupport)
			return nullptr;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return nullptr;
		}
		auto dir = path.substr(0, path.find_last_of('/'));
		std::unordered_map<std::string, std::shared_ptr<Image>> image_table;

		//��Root��ȡ������
		auto root = New<YObject>(scene->mRootNode->mName.C_Str());
		const auto& Transform = New<TransformComponent>(root);
		Transform->SetWorldScale(Vector3(0.2f, 0.2f, 0.2f));
		LoadNodes(image_table, dir, scene->mRootNode, scene, root);
		return root;
	}

	void LoadNodes
	(
		std::unordered_map<std::string, std::shared_ptr<Image>>& image_table,
		const std::string& dir,
		aiNode* node,
		const aiScene* scene,
		std::shared_ptr<YObject> parent
	)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto meshObj = New<YObject>(node->mName.C_Str() + std::to_string(i), parent);
			New<TransformComponent>(meshObj);
			LoadMesh(image_table, dir, mesh, scene, meshObj);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			auto Newparent = New<YObject>(node->mChildren[i]->mName.C_Str(),parent);
			New<TransformComponent>(Newparent);
			LoadNodes(image_table, dir, node->mChildren[i], scene, Newparent);
		}
	}

	void LoadMesh(
		std::unordered_map<std::string, std::shared_ptr<Image>>& image_table,
		const std::string& dir,
		aiMesh* mesh,
		const aiScene* scene,
		std::shared_ptr<YObject> meshobj
	)
	{
		std::vector<Vector3> poses;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;
		std::vector<unsigned int> indices;
		std::vector<Vector3> tangents;
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {

			Vector3 vertices(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
#ifdef YRENDER_REVERSE
			vertices = CoreDefine::ModelReverse.TrasformPoint(vertices);
#endif
			poses.emplace_back(vertices);

			//ģ��ת������ת�þ�����ԭ������ͬ������ֱ��ʹ��ModelReverse
			Vector3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
#ifdef YRENDER_REVERSE
			normal = CoreDefine::ModelReverse.TrasformVec(normal);
#endif
			normals.emplace_back(normal);

			if (mesh->mTextureCoords[0]) {
				texcoords.emplace_back(
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
				);
			}
			else
				texcoords.emplace_back(0.f, 0.f);

			if (mesh->mTangents) {
				Vector3 Tangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
#ifdef YRENDER_REVERSE
				Tangent = CoreDefine::ModelReverse.TrasformVec(Tangent);
#endif
				tangents.emplace_back(Tangent);
			}
		}
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j]);
			}
		}
		auto TriMeshPtr = New<TriMesh>(std::move(indices), std::move(poses), std::move(normals), std::move(texcoords), std::move(tangents));
		New<MeshComponent>(meshobj, TriMeshPtr);

		auto bsdf = New<BSDF_StandardPBR>();
		New<MaterialComponent>(meshobj, bsdf);

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		/*for (int i = 0; i <= 12; i++) {
			auto n = material->GetTextureCount(static_cast<aiTextureType>(i));
			printf("%d : %d\n", i, n);
		}*/

		//1024 * 1024 һ����ͼ16M
		bsdf->albedoTexture = LoadTexture(image_table, dir, material, aiTextureType_DIFFUSE);
		bsdf->normalTexture = LoadTexture(image_table, dir, material, aiTextureType_HEIGHT);
		bsdf->metallicTexture = LoadTexture(image_table, dir, material, aiTextureType_SPECULAR);
		bsdf->aoTexture = LoadTexture(image_table, dir, material, aiTextureType_AMBIENT);
		bsdf->roughnessTexture = LoadTexture(image_table, dir, material, aiTextureType_EMISSIVE);

	}

	std::shared_ptr<Image> AssimpLoader::LoadTexture(
		std::unordered_map<std::string, std::shared_ptr<Image>>& image_table,
		const std::string& dir,
		aiMaterial* material,
		aiTextureType type
	)
	{
		auto num = material->GetTextureCount(type);
		if (num == 0)
			return nullptr;

		if (num >= 2) {
			printf("WARNNING::AssimpLoader::LoadMaterial:\n"
				"\t""texture(type %d) >= 2\n", static_cast<int>(type));
		}

		aiString str;
		material->GetTexture(type, 0, &str);

		std::string path = dir + "\\" + str.C_Str();
		//#ifdef _DEBUG
		//	std::cout << path << std::endl;
		//#endif 
		if (image_table.find(path) != image_table.end())
			return image_table[path];

		auto img = New<Image>(path);
		if (!img->IsValid()) {
			printf("ERROR::AssimpLoader::LoadMeshTexture:"
				"\t""[%s] load fail.\n", path.c_str());
			return nullptr;
		}
		//image_table.insert(std::make_pair(path, img));
		image_table[path] = img;
		return img;
	}
}
