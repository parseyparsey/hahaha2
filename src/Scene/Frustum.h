#pragma once
#include <cmath>
#include <glm/glm.hpp>

struct Plane {
	glm::vec3 normal{0.0f, 1.0f, 0.0f}; // points into the frustum
	float distance = 0.0f;
	float signedDistance(const glm::vec3 &p) const {
		return glm::dot(normal, p) - distance;
	}
};

struct AABB {
	glm::vec3 center, extents;
};

struct Frustum {
	Plane planes[6]; // left, right, bottom, top, near, far

	explicit Frustum(const glm::mat4 &vp) { // Gribb-Hartmann extraction
		auto row = [&](int i) {
			return glm::vec4(vp[0][i], vp[1][i], vp[2][i], vp[3][i]);
		};
		glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

		glm::vec4 eq[6] = {
						   r3 + r0, // L
						   r3 - r0, // R
						   r3 + r1, // B
						   r3 - r1, // T
						   r3 + r2, // N
						   r3 - r2  // F
		};

		for (int i = 0; i < 6; i++) {
			float len = glm::length(glm::vec3(eq[i]));
			planes[i].normal = glm::vec3(eq[i]) / len;
			planes[i].distance = -eq[i].w / len;
		}
	}

	// same test as the article's isOnOrForwardPlane: cull only if fully behind
	// a plane
	bool intersects(const AABB &b) const {
		for (const Plane &p : planes) {
			float r = b.extents.x * std::abs(p.normal.x) +
					  b.extents.y * std::abs(p.normal.y) +
					  b.extents.z * std::abs(p.normal.z);
			if (p.signedDistance(b.center) < -r)
				return false;
		}
		return true;
	}
};

// local min/max + any world matrix -> world-space AABB (handles rotation and
// non-uniform scale)
inline AABB transformAABB(const glm::vec3 &bmin, const glm::vec3 &bmax,
						  const glm::mat4 &m) {
	glm::vec3 c = (bmin + bmax) * 0.5f, e = (bmax - bmin) * 0.5f;
	AABB out;
	out.center = glm::vec3(m * glm::vec4(c, 1.0f));
	for (int i = 0; i < 3; i++)
		out.extents[i] = std::abs(m[0][i]) * e.x + std::abs(m[1][i]) * e.y +
						 std::abs(m[2][i]) * e.z;
	return out;
}