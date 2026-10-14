#include "ModelLoader.h"
#include "Scene/Scene.h"
#include "TextureCache.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <type_traits>

static std::string firstTexture(aiMaterial *m, aiTextureType t) {
	if (m->GetTextureCount(t) == 0)
		return "";
	aiString s;
	m->GetTexture(t, 0, &s);
	return s.C_Str();
}

bool ModelLoader::import(const std::string &path, ModelData &out) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(
		path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
	if (!scene || (scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE) ||
		!scene->mRootNode) {
		std::cerr << "Assimp::" << importer.GetErrorString() << std::endl;
		return false;
	}

	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial *m = scene->mMaterials[i];
		MaterialData md;
		md.diffPath = firstTexture(m, aiTextureType_DIFFUSE);
		md.specPath = firstTexture(m, aiTextureType_SPECULAR);
		md.normPath = firstTexture(m, aiTextureType_NORMALS);
		if (md.normPath.empty())
			md.normPath = firstTexture(m, aiTextureType_HEIGHT);
		float shin;
		if (m->Get(AI_MATKEY_SHININESS, shin) == AI_SUCCESS && shin > 1.0f)
			md.shininess = shin;
		out.materials.push_back(md);
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *am = scene->mMeshes[i];
		SubMeshData sm;
		sm.materialIndex = am->mMaterialIndex;
		sm.vertices.resize(am->mNumVertices);

		for (unsigned v = 0; v < am->mNumVertices; v++) {
			Vertex &vx = sm.vertices[v];
			vx.position = {am->mVertices[v].x, am->mVertices[v].y,
						   am->mVertices[v].z};
			if (am->HasNormals())
				vx.normal = {am->mNormals[v].x, am->mNormals[v].y,
							 am->mNormals[v].z};
			if (am->mTextureCoords[0])
				vx.texCoords = {am->mTextureCoords[0][v].x,
								am->mTextureCoords[0][v].y};
			if (am->HasTangentsAndBitangents()) {
				vx.tangent = {am->mTangents[v].x, am->mTangents[v].y,
							  am->mTangents[v].z};
				vx.bitangent = {am->mBitangents[v].x, am->mBitangents[v].y,
								am->mBitangents[v].z};
			}
		}

		for (unsigned f = 0; f < am->mNumFaces; f++)
			for (unsigned k = 0; k < am->mFaces[f].mNumIndices; k++)
				sm.indices.push_back(am->mFaces[f].mIndices[k]);
		out.meshes.push_back(std::move(sm));

	}

	return true;
}

bool ModelLoader::instantiate(Scene& scene, const ModelData& data,
	TextureCache& textures, const std::string& name)
{
	std::vector<Material *> mats;

	for (auto &md : data.materials) {
		Material m{};
		if (!md.diffPath.empty())
			m.diff = textures.get(data.directory + "/" + md.diffPath, true);
		if (!md.specPath.empty())
			m.spec = textures.get(data.directory + "/" + md.specPath, false);
		m.shininess = md.shininess;
		mats.push_back(&scene.addMaterial(m));
	}

	for (auto &sm : data.meshes) {
		Mesh& mesh = scene.addMesh(sm.vertices, sm.indices);
		scene.addObject(name.c_str(), &mesh, mats[sm.materialIndex]);
	}
}

//HERE IS THE FORMAT
//  [4] magic "H32M" | u32 version | u32 sizeof(Vertex) | u32 materialCount | u32 meshCount
//  per material: 3 strings (u32 length + bytes), float shininess
//  per mesh: u32 materialIndex | u32 vertexCount | u32 indexCount | Vertex[] | Index[]

static void writeStr(std::ostream& out, const std::string& s) { 
	uint32_t n = (uint32_t)s.size();
	out.write((char *)&n, 4);
	out.write(s.data(), n);
}

static bool readStr(std::istream &in, std::string &s) {
	uint32_t n;
	if (!in.read((char *)&n, 4) || n > 4096)
		return false;
	s.resize(n);
	return (bool)in.read(s.data(), n);
}

bool ModelLoader::writeBinary(const std::string& file, ModelData& data) {
	std::ofstream o(file, std::ios::binary);
	if (!o)
		return false;
	
	o.write("H32M", 4);
	uint32_t hdr[4] = {1, (uint32_t)sizeof(Vertex),
					   (uint32_t)data.materials.size(), (uint32_t)data.meshes.size()};
	o.write((char *)hdr, sizeof(hdr));

	for (auto& m : data.materials)
	{
		writeStr(o, m.diffPath);
		writeStr(o, m.specPath);
		writeStr(o, m.normPath);
		o.write((char *)&m.shininess, 4);
	}

	for (auto &s : data.meshes)
	{
		uint32_t h[3] = {s.materialIndex, (uint32_t)s.vertices.size(),
						 (uint32_t)s.indices.size()};

		o.write((char *)h, sizeof(h));
		o.write((char *)s.vertices.data(), s.vertices.size() * sizeof(Vertex));
		o.write((char *)s.indices.data(), s.indices.size() * sizeof(unsigned int));
	}

	return (bool)o;
}

bool ModelLoader::readBinary(const std::string& file, ModelData& out) {
	std::ifstream in(file, std::ios::binary);
	if (!in)
		return false;

	char magic[4];
	uint32_t hdr[4];

	if (!in.read(magic, 4) || std::memcpy(magic, "H32M", 4) != 0)
		return false;
	if (!in.read((char *)hdr, sizeof(hdr)) || hdr[0] != 1 ||
		hdr[1] != sizeof(Vertex))
		return false;

	out.materials.resize(hdr[2]);
	
	for (auto &m : out.materials) {
		if (!readStr(in, m.diffPath) || !readStr(in, m.specPath) ||
			!readStr(in, m.normPath))
			return false;
		if (!in.read((char *)&m.shininess, 4))
			return false;
	}

	out.meshes.resize(hdr[3]);

	for (auto &s : out.meshes) {
		uint32_t h[3];

		if (!in.read((char *)h, sizeof(h)) || h[1] > 50'000'000 ||
			h[2] > 200'000'000)
			return false;

		s.materialIndex = h[0];
		s.vertices.resize(h[1]);
		s.indices.resize(h[2]);

		if (!in.read((char *)s.vertices.data(), h[1] * sizeof(Vertex)))
			return false;
		if (!in.read((char *)s.indices.data(), h[2] * sizeof(unsigned int)))
			return false;
	}
	return true;
}

bool ModelLoader::load(const std::string &path, ModelData &out) {
	namespace fs = std::filesystem;

	out.directory = fs::path(path).parent_path().string();
	std::string cache = path + ".h32m";

	if (fs::exists(cache) &&
		fs::last_write_time(cache) >= fs::last_write_time(path) &&
		readBinary(cache, out))
		return true;

	out = ModelData{ out.directory, {}, {} };
	
	if (!import(path, out))
		return false;

	writeBinary(cache, out);

	return true;
}