#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <mesh.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define FLIPPED_D 0
#define FLIPPED_S 1
#define FLIPPED_DS 2
#define FLIPPED_0 3

class Model 
{
public:
	int isFlipped;
	Model(std::string path, int isFlip)
	{
		isFlipped = isFlip;
		loadModel(path);
	}
	void Draw(Shader& shader);
	std::vector<Mesh> meshes;
	std::vector<mTexture> textures_loaded;
	//inline static bool model_gamma_correction = true;

	inline static float tRawMem = 0.0f;
	inline static float tEstMemWMip = 0.0f;

private:
	std::string directory;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<mTexture> loadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif
