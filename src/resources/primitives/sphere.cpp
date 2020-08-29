#include "sphere.h"
#include "core/math/trigonometry/conversions.h"

#include <cmath>

namespace primal {

  // F(u,v, r) = [cos(u)*sin(v)*r, cos(v), sin(u)*sin(v)*r] where
  // u is longitude [0, 2PI] and v is lattitude [0, PI]
  Sphere::Sphere(unsigned int xSegments, unsigned int ySegments) {
	for (unsigned int y = 0; y <= ySegments; ++y) {
	  for (unsigned int x = 0; x <= xSegments; ++x) {
		float xSegment = (float)x / (float)ySegments;
		float ySegment = (float)y / (float)ySegments;
		float xPos = std::cos(xSegment * math::PI * 2) * std::sin(ySegment * math::PI);
		float yPos = std::cos(ySegment * math::PI);
		float zPos = std::sin(xSegment * math::PI * 2) * std::sin(ySegment * math::PI);

		m_positions.emplace_back(xPos, yPos, zPos);
		m_uv.emplace_back(xSegment, ySegment);
		m_normals.emplace_back(xPos, yPos, zPos);
	  }
	}

	bool oddRow = false;
	for (int y = 0; y < ySegments; ++y) {
	  for (int x = 0; x < xSegments; ++x) {
		m_indices.push_back((y + 1) * (xSegments + 1) + x);
		m_indices.push_back(y * (xSegments + 1) + x);
		m_indices.push_back(y * (xSegments + 1) + x + 1);

		m_indices.push_back((y + 1) * (xSegments + 1) + x);
		m_indices.push_back(y * (xSegments + 1) + x + 1);
		m_indices.push_back((y + 1) * (xSegments + 1) + x + 1);
	  }
	}

	topology = renderer::TRIANGLES;
	finalize();
  }

}
