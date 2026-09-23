#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader
{
public:
	//the shader program id
	unsigned int ID;

	//constructor to read and build the shader
	Shader(const char* vertexPath, const char* fragmentPath, const char* geoPath = "");

	//activate the shader
	void use();

	//for uniforms
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec3f(const std::string& name, float x, float y, float z) const;
	void loadGeometry(const char* geometryPath) const;
};



#endif
