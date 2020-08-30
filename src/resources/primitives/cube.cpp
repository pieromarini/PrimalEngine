#include "cube.h"

#include "core/math/linear_algebra/vector.h"

namespace primal {

  Cube::Cube() {
	m_positions = std::vector<math::vec3>{
		math::vec3(-0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, -0.5f),
		math::vec3(0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, -0.5f),
		math::vec3(-0.5f, -0.5f, -0.5f),
		math::vec3(-0.5f, 0.5f, -0.5f),

		math::vec3(-0.5f, -0.5f, 0.5f),
		math::vec3(0.5f, -0.5f, 0.5f),
		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(-0.5f, 0.5f, 0.5f),
		math::vec3(-0.5f, -0.5f, 0.5f),

		math::vec3(-0.5f, 0.5f, 0.5f),
		math::vec3(-0.5f, 0.5f, -0.5f),
		math::vec3(-0.5f, -0.5f, -0.5f),
		math::vec3(-0.5f, -0.5f, -0.5f),
		math::vec3(-0.5f, -0.5f, 0.5f),
		math::vec3(-0.5f, 0.5f, 0.5f),

		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, -0.5f),
		math::vec3(0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(0.5f, -0.5f, 0.5f),

		math::vec3(-0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, -0.5f, -0.5f),
		math::vec3(0.5f, -0.5f, 0.5f),
		math::vec3(0.5f, -0.5f, 0.5f),
		math::vec3(-0.5f, -0.5f, 0.5f),
		math::vec3(-0.5f, -0.5f, -0.5f),

		math::vec3(-0.5f, 0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(0.5f, 0.5f, -0.5f),
		math::vec3(0.5f, 0.5f, 0.5f),
		math::vec3(-0.5f, 0.5f, -0.5f),
		math::vec3(-0.5f, 0.5f, 0.5f),
	};

	m_uv = std::vector<math::vec2>{
		math::vec2(0.0f, 0.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(0.0f, 0.0f),
		math::vec2(0.0f, 1.0f),

		math::vec2(0.0f, 0.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(0.0f, 0.0f),

		math::vec2(1.0f, 0.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(0.0f, 0.0f),
		math::vec2(1.0f, 0.0f),

		math::vec2(1.0f, 0.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(0.0f, 0.0f),

		math::vec2(0.0f, 1.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(0.0f, 0.0f),
		math::vec2(0.0f, 1.0f),

		math::vec2(0.0f, 1.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(1.0f, 1.0f),
		math::vec2(1.0f, 0.0f),
		math::vec2(0.0f, 1.0f),
		math::vec2(0.0f, 0.0f),
	};
	m_normals = std::vector<math::vec3>{
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f),
		math::vec3(0.0f, 0.0f, -1.0f),

		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),
		math::vec3(0.0f, 0.0f, 1.0f),

		math::vec3(-1.0f, 0.0f, 0.0f),
		math::vec3(-1.0f, 0.0f, 0.0f),
		math::vec3(-1.0f, 0.0f, 0.0f),
		math::vec3(-1.0f, 0.0f, 0.0f),
		math::vec3(-1.0f, 0.0f, 0.0f),
		math::vec3(-1.0f, 0.0f, 0.0f),

		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f),
		math::vec3(1.0f, 0.0f, 0.0f),

		math::vec3(0.0f, -1.0f, 0.0f),
		math::vec3(0.0f, -1.0f, 0.0f),
		math::vec3(0.0f, -1.0f, 0.0f),
		math::vec3(0.0f, -1.0f, 0.0f),
		math::vec3(0.0f, -1.0f, 0.0f),
		math::vec3(0.0f, -1.0f, 0.0f),

		math::vec3(0.0f, 1.0f, 0.0f),
		math::vec3(0.0f, 1.0f, 0.0f),
		math::vec3(0.0f, 1.0f, 0.0f),
		math::vec3(0.0f, 1.0f, 0.0f),
		math::vec3(0.0f, 1.0f, 0.0f),
		math::vec3(0.0f, 1.0f, 0.0f),
	};

	topology = renderer::TRIANGLES;
	finalize();
  }

}
