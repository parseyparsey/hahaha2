#pragma once
#include <glad/glad.h>
#include <cstddef>

class UniformBuffer {
public:
	UniformBuffer(size_t size, GLuint bindingPoint,
				  GLenum usage = GL_DYNAMIC_DRAW);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer &) = delete;
	UniformBuffer &operator=(const UniformBuffer &) = delete;
	UniformBuffer(UniformBuffer &&other) noexcept;
	UniformBuffer &operator=(UniformBuffer &&other) noexcept;

	void update(const void *data, size_t size, size_t offset = 0) const;

	template <typename T> void set(const T &value, size_t offset = 0) const {
		update(&value, sizeof(T), offset);
	}

	void bindToPoint() const;
	void bindRange(size_t size, size_t offset) const;

	GLuint id() const { return m_ubo; }
	GLuint bindingPoint() const { return m_binding; }
	size_t size() const { return m_size; }

private:
	void create();
	void destroy();

	GLuint m_ubo;
	GLuint m_binding;
	GLuint m_usage = GL_DYNAMIC_DRAW;
	size_t m_size;
};