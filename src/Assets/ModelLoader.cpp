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

static void dumpMaterial(aiMaterial *m, int index) {
	aiString name;
	m->Get(AI_MATKEY_NAME, name);
	std::cout << "[material " << index << "] " << name.C_Str() << "\n";
	for (int t = aiTextureType_NONE + 1; t < AI_TEXTURE_TYPE_MAX; t++) {
		if (!m->GetTextureCount((aiTextureType)t))
			continue;
		aiString p;
		m->GetTexture((aiTextureType)t, 0, &p);
		std::cout << "  " << aiTextureTypeToString((aiTextureType)t) << " -> "
				  << p.C_Str() << "\n";
	}
}

static void fillMaterial(aiMaterial* m, MaterialData& md) {
	md.diffPath = firstTexture(m, aiTextureType_BASE_COLOR);
	if (md.diffPath.empty())
		md.diffPath = firstTexture(m, aiTextureType_DIFFUSE);
	md.specPath = firstTexture(m, aiTextureType_SPECULAR);
	md.normPath = firstTexture(m, aiTextureType_NORMALS);
	if (md.normPath.empty())
		md.normPath = firstTexture(m, aiTextureType_HEIGHT); // OBJ map_Bump
	md.heightPath = firstTexture(m, aiTextureType_DISPLACEMENT);

	md.metallicPath = firstTexture(m, aiTextureType_METALNESS);
	md.roughnessPath = firstTexture(m, aiTextureType_DIFFUSE_ROUGHNESS);
	md.aoPath = firstTexture(m, aiTextureType_AMBIENT_OCCLUSION);
	if (md.aoPath.empty())
		md.aoPath = firstTexture(m, aiTextureType_LIGHTMAP); // glTF occlusion
	
	aiColor4D c;
	if (m->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS)
		md.baseColor = {c.r, c.g, c.b};
	bool hasM = m->Get(AI_MATKEY_METALLIC_FACTOR, md.metallic) == AI_SUCCESS;
	bool hasR = m->Get(AI_MATKEY_ROUGHNESS_FACTOR, md.roughness) == AI_SUCCESS;

	// glTF's packed metallic-roughness texture: Assimp 5.x usually reports it
	// as UNKNOWN
	if (md.metallicPath.empty() && md.roughnessPath.empty() && (hasM || hasR)) {
		md.metalRoughPath = firstTexture(m, aiTextureType_UNKNOWN);
		md.packedMR = !md.metalRoughPath.empty();
	}

	md.isPBR = hasM || hasR || md.packedMR || !md.metallicPath.empty() ||
			   !md.roughnessPath.empty() || !md.aoPath.empty();

	float shin;
	if (m->Get(AI_MATKEY_SHININESS, shin) == AI_SUCCESS && shin > 1.0f)
		md.shininess = shin;
}

bool ModelLoader::import(const std::string &path, ModelData &out) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(
		path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
		!scene->mRootNode) {
		std::cerr << "Assimp::" << importer.GetErrorString() << std::endl;
		return false;
	}

	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial *m = scene->mMaterials[i];
		MaterialData md;
		fillMaterial(m, md);
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

std::vector<size_t> ModelLoader::instantiate(Scene& scene, const ModelData& data,
	TextureCache& textures, const std::string& name, const TextureFlips& flips)
{
	std::vector<Material *> mats;

	for (auto &md : data.materials) {
		Material m{};
		if (!md.diffPath.empty())
			m.diff = textures.get(data.directory + "/" + md.diffPath, true, flips.diffuse);
		if (!md.specPath.empty())
			m.spec = textures.get(data.directory + "/" + md.specPath, false, flips.specular);
		if (!md.normPath.empty())
			m.norm = textures.get(data.directory + "/" + md.normPath, false, flips.normal);
		
		if (md.isPBR)
		{
			m.model = ShadingModel::PBR;
			if (md.packedMR) {
				m.metallicMap = m.roughnessMap =
					textures.get(data.directory + "/" + md.metalRoughPath, false, flips.pbr);
				m.packedMR = true;
			} else {
				if (!md.metallicPath.empty())
					m.metallicMap = textures.get(data.directory + "/" + md.metallicPath,
										  false, flips.pbr);
				if (!md.roughnessPath.empty())
					m.roughnessMap = textures.get(data.directory + "/" + md.roughnessPath,
									 false, flips.pbr);
			}
			if (!md.aoPath.empty())
				m.aoMap = textures.get(data.directory + "/" + md.aoPath, 
					false, flips.pbr);
			m.metallic = md.metallic;
			m.roughness = md.roughness;
		} 
		else
		{
			m.model = ShadingModel::BLINNPHONG;
			if (!md.heightPath.empty())
			m.parallaxMap = textures.get(data.directory + "/" + md.heightPath, false, flips.height);
			m.shininess = md.shininess;
		} 

		mats.push_back(&scene.addMaterial(m));
	}

	std::vector<size_t> ids;
	int n = 0;

	for (auto &sm : data.meshes) {
		Mesh& mesh = scene.addMesh(sm.vertices, sm.indices);
		scene.addObject((name + "_" + std::to_string(n++)).c_str(), &mesh,
						mats[sm.materialIndex]);
		ids.push_back(scene.getObjects().size() - 1);
	}

	return ids;
}

//HERE IS THE FORMAT
//  [4] magic "H32M" | u32 version | u32 sizeof(Vertex) | u32 materialCount | u32 meshCount
//  per material: 3 strings (u32 length + bytes), float shininess
//  per mesh: u32 materialIndex | u32 vertexCount | u32 indexCount | Vertex[] | Index[]

using PathMember = std::string MaterialData::*;

static constexpr PathMember kPaths[] = {
	&MaterialData::diffPath,   &MaterialData::specPath,     &MaterialData::normPath,
    &MaterialData::heightPath, &MaterialData::metallicPath, &MaterialData::roughnessPath,
    &MaterialData::aoPath,     &MaterialData::metalRoughPath
};

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

static void writeMaterial(std::ostream &o, const MaterialData &m) {
	for (PathMember p : kPaths)
		writeStr(o, m.*p);

	float f[6] = {m.baseColor.r, m.baseColor.g, m.baseColor.b,
				  m.shininess,	 m.metallic,	m.roughness};

	uint32_t flags = (m.isPBR ? 1u : 0u) | (m.packedMR ? 2u : 0u);

	o.write((const char *)f, sizeof(f));
	o.write((const char *)&flags, sizeof(4));
}

static bool readMaterial(std::ifstream& in, MaterialData& m) {
	for (PathMember p : kPaths)
		if (!readStr(in, m.*p))
			return false;

	float f[6];
	uint32_t flags;

	if (!in.read((char *)f, sizeof(f)) || !in.read((char *)&flags, 4))
		return false;

	m.baseColor = {f[0], f[1], f[2]};
	m.shininess = f[3];
	m.metallic = f[4];
	m.roughness = f[5];
	m.isPBR = (flags & 1u) != 0;
	m.packedMR = (flags & 2u) != 0;

	return true;
}

static_assert(std::is_trivially_copyable_v<Vertex>);

bool ModelLoader::writeBinary(const std::string& file, ModelData& data) {
	std::ofstream o(file, std::ios::binary);
	if (!o)
		return false;
	
	o.write("H32M", 4);
	uint32_t hdr[4] = {3, (uint32_t)sizeof(Vertex),
					   (uint32_t)data.materials.size(), (uint32_t)data.meshes.size()};
	o.write((char *)hdr, sizeof(hdr));

	for (auto& m : data.materials)
	{
		writeMaterial(o, m);
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

	if (!in.read(magic, 4) || std::memcmp(magic, "H32M", 4) != 0)
		return false;
	if (!in.read((char *)hdr, sizeof(hdr)) || hdr[0] != 3 ||
		hdr[1] != sizeof(Vertex))
		return false;

	out.materials.resize(hdr[2]);
	
	for (auto &m : out.materials) {
		if (!readMaterial(in, m))
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
	{
		std::cout << "MODEL::" << path << "::H32M_FOUND!\n";
		return true;
	}
	
	std::string dir = out.directory;
	out = ModelData{dir, {}, {}};
	//out = ModelData{ out.directory, {}, {} };
	
	std::cout << "MODEL::" << path << "::H32M_NOT_FOUND!\n";

	if (!import(path, out))
		return false;

	writeBinary(cache, out);

	return true;
}