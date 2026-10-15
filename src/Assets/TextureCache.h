#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "texture.h"

class TextureCache {
public:
	unsigned int get(const std::string &path, bool srgb, bool flip) {
		std::string key = path + (srgb ? "#srgb" : "#lin");
		auto it = m_cache.find(key);
		if (it != m_cache.end())
			return it->second->ID;

		auto tex = std::make_unique<Texture>(path.c_str(), flip, srgb, 1);
		unsigned int id = tex->ID;
		m_cache.emplace(std::move(key), std::move(tex));
		return id;
	}

	size_t size() const { return m_cache.size(); }

private:
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_cache;
};