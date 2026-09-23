#pragma once
#include <glad/glad.h>
#include <vector>

struct AttachmentSpec {
	GLenum internalFormat = GL_RGBA16F;
	GLenum format = GL_RGBA;
	GLenum type = GL_FLOAT;
	GLenum minFilter = GL_LINEAR;
	GLenum magFilter = GL_LINEAR;
	GLenum wrap = GL_CLAMP_TO_EDGE;
};

enum class DepthMode {
	None,
	Depth,
	DepthStencil
};

class Framebuffer {
public:
	Framebuffer(int width, int height,
							 std::vector<AttachmentSpec> colorSpecs,
							 DepthMode depthMode = DepthMode::None);
	~Framebuffer();

	Framebuffer(const Framebuffer &) = delete;
	Framebuffer &operator=(const Framebuffer &) = delete;
	Framebuffer(Framebuffer &&other) noexcept;
	Framebuffer &operator=(Framebuffer &&other) noexcept;

	void bind() const;
	static void bindDefault();

	void resize(int width, int height);

	unsigned int getColorAttachment(size_t index = 0) const;
	unsigned int getDepthAttachment() const { return m_depthTexture; }

	int getWidth() const { return m_width; };
	int getHeight() const { return m_height; };
	bool isComplete() const;

private:
	void create();
	void destroy();

	unsigned int m_fbo = 0, m_depthRBO = 0, m_depthTexture = 0;
	std::vector<unsigned int> m_colorTextures;
	std::vector<AttachmentSpec> m_specs;
	DepthMode m_depthMode;
	int m_width, m_height;
};