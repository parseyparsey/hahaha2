#include <shader.h>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

int vi = 0, fi = 0, li = 0;

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geoPath){
	std::string vertexCode, fragmentCode;
	std::ifstream vShaderFile, fShaderFile;

	vShaderFile.exceptions(std::ifstream::badbit | std::fstream::failbit);
	fShaderFile.exceptions(std::ifstream::badbit | std::fstream::failbit);

	try {
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "SHADER" << vi << ": " << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
	}

	std::string geometryCode;
	std::ifstream gShaderFile;
	gShaderFile.exceptions(std::ifstream::badbit | std::fstream::failbit);

	if (geoPath && geoPath[0] != '\0')
	{
		try
		{
			gShaderFile.open(geoPath);

			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();

			gShaderFile.close();

			geometryCode = gShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "SHADER" << fi << ": " << "ERROR::SHADER::GEOMETRY::FILE_NOT_SUCCESFULLY_READ\n";
		}
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	unsigned int vshader, fshader;
	int success;
	char infolog[512];

	vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vShaderCode, NULL);
	glCompileShader(vshader);

	std::cout << "SHADER" << vi << ": " << vertexPath << std::endl;

	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, 512, NULL, infolog);
		std::cout << "SHADER" << vi << ": " << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" <<
			infolog << std::endl;
	}
	vi++;

	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fShaderCode, NULL);
	glCompileShader(fshader);

	std::cout << "SHADER" << fi << ": " << fragmentPath << std::endl;

	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, 512, NULL, infolog);
		std::cout << "SHADER" << fi << ": " << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" <<
			infolog << std::endl;
	}

	const char* gShaderCode;

	unsigned int gshader;

	if (geoPath && geoPath[0] != '\0')
	{
		gShaderCode = geometryCode.c_str();
		int gsuccess;
		char ginfolog[512];
		gshader = glCreateShader(GL_GEOMETRY_SHADER);

		std::cout << "SHADER" << fi << ": " << geoPath << std::endl;

		glShaderSource(gshader, 1, &gShaderCode, NULL);
		glCompileShader(gshader);

		glGetShaderiv(gshader, GL_COMPILE_STATUS, &gsuccess);
		if (!gsuccess)
		{
			glGetShaderInfoLog(gshader, 512, NULL, ginfolog);
			std::cout << "SHADER" << fi << ": " << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" <<
						infolog << std::endl;
		}
	}

	fi++;

	ID = glCreateProgram();

	glAttachShader(ID, vshader);

	if (geoPath && geoPath[0] != '\0')
	{
		glAttachShader(ID, gshader);
	}

	glAttachShader(ID, fshader);

	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infolog);
		std::cout << "SHADER" << li << ": " << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n\n" <<
			infolog << std::endl;
	}
	else {
		//std::cout << "SHADER" << li << ": Linking Shaders Successed.\n";
	}
	li++;

	glDeleteShader(vshader);
	glDeleteShader(fshader);

}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::loadGeometry(const char* geometryPath) const {
	std::string geometryCode;
	std::ifstream gShaderFile;

	gShaderFile.exceptions(std::ifstream::badbit | std::fstream::failbit);

	try {
		gShaderFile.open(geometryPath);

		std::stringstream gShaderStream;
		gShaderStream << gShaderFile.rdbuf();

		gShaderFile.close();

		geometryCode = gShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "SHADER" << vi << ": " << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
	}

	const char* gShaderCode = geometryCode.c_str();

	int success;
	char infolog[512];

	unsigned int gshader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gshader, 1, &gShaderCode, NULL);
	glCompileShader(gshader);

	glGetShaderiv(gshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(gshader, 512, NULL, infolog);
		std::cout << "SHADER" << fi << ": " << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" <<
			infolog << std::endl;
	}

	glAttachShader(ID, gshader);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infolog);
		std::cout << "SHADER" << li << ": " << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" <<
			infolog << std::endl;
	} else {
		std::cout << "geo succes\n";
	}
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const {
	int loc = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, glm::vec3 value) const {
	int loc = glGetUniformLocation(ID, name.c_str());
	glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::setVec3f(const std::string& name, float x, float y, float z) const {
	int loc = glGetUniformLocation(ID, name.c_str());
	float values[] = { x, y, z };
	glUniform3fv(loc, 1, values/*glm::value_ptr(glm::vec3(x, y, z))*/);
}
