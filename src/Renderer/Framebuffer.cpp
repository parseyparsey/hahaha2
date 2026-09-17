#include <Framebuffer.h>
#include <iostream>
#include <utility>

Framebuffer::Framebuffer(int width, int height,
						 std::vector<AttachmentSpec> colorSpecs,
						 DepthMode depthMode = DepthMode::None) 
	: m_specs(std::move(colorSpecs), m_depthMode(depthMode), m_width(width),
		m_height(height) {
	create();
}

Framebuffer::~Framebuffer() {
	destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept {
	*this = std::move(other);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
	if (this != &other) {
		destroy();
		m_fbo = other.m_fbo;
		m_depthRBO = other.m_depthRBO;
		m_depthTexture = other.m_depthTexture;
		m_colorTextures = std::move(other.m_colorTextures);
		m_specs = std::move(other.m_specs);
		m_depthMode = other.m_depthMode;
		m_width = other.m_width;
		m_height = other.m_height;
		other.m_fbo = other.m_depthRBO = other.m_depthTexture = 0;
	}
	return *this;
}

void Framebuffer::create() {
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	m_colorTextures.resize(m_specs.size());
	glGenTextures((GLsizei)m_colorTextures.size(), m_colorTextures.data());

	std::vector<GLenum> drawBuffers;
	for (size_t i = 0; i < m_colorTextures.size(); ++i) {
		const auto &spec = m_specs[i];
		glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, spec.internalFormat, m_width, m_height,
					 0, spec.format, spec.type, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, spec.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, spec.magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, spec.wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, spec.wrap);

		GLenum attachment = GL_COLOR_ATTACHMENT0 + (GLenum)i;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D,
							   m_colorTextures[i], 0);
		drawBuffers.push_back(attachment);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!drawBuffers.empty())
		glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());
	else
		glDrawBuffer(GL_NONE); // depth-only FBO (e.g. shadow maps)

	if (m_depthMode == DepthMode::Depth ||
		m_depthMode == DepthMode::DepthStencil) {
		GLenum internalFormat = (m_depthMode == DepthMode::DepthStencil)
									? GL_DEPTH24_STENCIL8
									: GL_DEPTH_COMPONENT24;
		GLenum attachPoint = (m_depthMode == DepthMode::DepthStencil)
								 ? GL_DEPTH_STENCIL_ATTACHMENT
								 : GL_DEPTH_ATTACHMENT;

		glGenRenderbuffers(1, &m_depthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m_width,
							  m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER,
								  m_depthRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	if (!isComplete())
		std::cout << "Framebuffer incomplete! status: "
				  << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
	if (!m_colorTextures.empty())
		glDeleteTextures((GLsizei)m_colorTextures.size(),
						 m_colorTextures.data());
	if (m_depthRBO)
		glDeleteRenderbuffers(1, &m_depthRBO);
	if (m_depthTexture)
		glDeleteTextures(1, &m_depthTexture);
	if (m_fbo)
		glDeleteFramebuffers(1, &m_fbo);
	m_fbo = m_depthRBO = m_depthTexture = 0;
	m_colorTextures.clear();
}

void Framebuffer::resize(int width, int height) {
	if (width == m_width && height == m_height)
		return; // guard — no wasted reallocation
	if (width <= 0 || height <= 0)
		return; // guard — no invalid allocation

	destroy();
	m_width = width;
	m_height = height;
	create();
}

void Framebuffer::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::bindDefault() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::getColorAttachment(size_t index) const {
	return (index < m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

bool Framebuffer::isComplete() const {
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}