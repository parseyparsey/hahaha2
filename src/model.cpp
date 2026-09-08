#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <mesh.h>
#include <model.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int mi = 0;

void Model::Draw(Shader& shader) {
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void Model::loadModel(std::string path) {
	textures_loaded.clear();  // <-- Clear previously cached textures //051025
	meshes.clear(); //051025
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	std::cout << "MODEL" << mi << ": " << path << "\n";
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
	mi++;
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	//process node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	//iterate through node's children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<mTexture> textures;

	//process vertex positions, normals, and texcoords
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		if (mesh->HasNormals()) {
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	//for indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	//for materials
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<mTexture> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<mTexture> specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

unsigned int TextureFromFile(const char* path, const std::string& directory, bool flip, bool gamma_corrected)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	if (flip == true)
		stbi_set_flip_vertically_on_load(true);
	else
		stbi_set_flip_vertically_on_load(false);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	float rawMB = (width * height * nrComponents) / (1024.0f * 1024.0f);
	float estVRAM_MB = rawMB * 1.33f;

	Model::tRawMem += rawMB;
	Model::tEstMemWMip += estVRAM_MB;

	std::cout << "MODELTXT" << mi << ": " << path << " | " << width << "x"
			  << height << " (" << nrComponents << "ch)" << " | " << rawMB << "MB"
			  << " | ~" << estVRAM_MB << " MB (w/ mips)" << std::endl;

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);

		if (gamma_corrected) {
			if (nrComponents == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB,
							 GL_UNSIGNED_BYTE, data);
			else if (nrComponents == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0,
							 GL_RGBA, GL_UNSIGNED_BYTE, data);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}

		
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (flip == true)
			stbi_set_flip_vertically_on_load(false);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

std::vector<mTexture> Model::loadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<mTexture> textures;
	for (unsigned i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			mTexture texture;
			switch (isFlipped) {
				case 0: if (typeName == "texture_diffuse")
							texture.id = TextureFromFile(str.C_Str(), directory, true, true);
						if (typeName == "texture_specular")
							texture.id = TextureFromFile(str.C_Str(), directory, false, false);
						break;
				case 1: if (typeName == "texture_diffuse")
							texture.id = TextureFromFile(str.C_Str(), directory, false, true);
						if (typeName == "texture_specular")
							texture.id = TextureFromFile(str.C_Str(), directory, true, false);
						break;
				case 2: if (typeName == "texture_diffuse")
							texture.id = TextureFromFile(str.C_Str(), directory, true, true);
						if (typeName == "texture_specular")
							texture.id = TextureFromFile(str.C_Str(), directory, true, false);
						break;
				case 3: if (typeName == "texture_diffuse")
							texture.id = TextureFromFile(str.C_Str(), directory, false, true);
						if (typeName == "texture_specular")
							texture.id = TextureFromFile(str.C_Str(), directory, false, false);
						break;
			}
			texture.type = typeName;
			texture.path = std::string(str.C_Str());
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}
