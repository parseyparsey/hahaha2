#include "UniformBuffer.h"
#include <iostream>
#include <utility>

UniformBuffer::UniformBuffer(size_t size, GLuint bindingPoint, GLenum usage) :
	m_binding(bindingPoint), m_usage(usage), m_size(size) {
	create();
}

UniformBuffer::~UniformBuffer() { destroy(); }

UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept {
	*this = std::move(other);
}

UniformBuffer &UniformBuffer::operator=(UniformBuffer &&other) noexcept {
	if (this != &other) {
		destroy();
		m_ubo = other.m_ubo;
		m_binding = other.m_binding;
		m_usage = other.m_usage;
		m_size = other.m_size;
		other.m_ubo = 0;
		other.m_size = 0;
	}
	return *this;
}

void UniformBuffer::create() {
	glGenBuffers(1, &m_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)m_size, nullptr, m_usage);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	bindToPoint();
}

void UniformBuffer::destroy() {
	if (m_ubo)
		glDeleteBuffers(1, &m_ubo);
	m_ubo = 0;
}

void UniformBuffer::update(const void *data, size_t size, size_t offset) const {
	if (offset + size > m_size) {
		std::cout << "UniformBuffer overflow! offset " << offset << " + size "
				  << size << " > " << m_size << std::endl;
		return;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, (GLintptr)offset, (GLsizeiptr)size,
					data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::bindToPoint() const {
	glBindBufferBase(GL_UNIFORM_BUFFER, m_binding, m_ubo);
}

void UniformBuffer::bindRange(size_t offset, size_t size) const {
	glBindBufferRange(GL_UNIFORM_BUFFER, m_binding, m_ubo, (GLintptr)offset,
					  (GLsizeiptr)size);
}