#pragma once

#include <string>
#include "ModelData.h"

class Scene;
class TextureCache;

class ModelLoader {
public:
	static bool load(const std::string &path, ModelData &out);

	static void instantiate(Scene& scene, const ModelData &data, 
		TextureCache& textures, const std::string& name);

private:
	static bool import(const std::string &path, ModelData &out);
	static bool readBinary(const std::string &file, ModelData &out);
	static bool writeBinary(const std::string &file, ModelData &data);

	ModelLoader() = delete;
};