#include "cube.h"

#include "core/math/vector2.h"
#include "core/math/vector3.h"

namespace primal {

  Cube::Cube() {
	m_positions = std::vector<math::Vector3>{
		math::Vector3(-0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, -0.5f),
		math::Vector3(0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, -0.5f),
		math::Vector3(-0.5f, -0.5f, -0.5f),
		math::Vector3(-0.5f, 0.5f, -0.5f),

		math::Vector3(-0.5f, -0.5f, 0.5f),
		math::Vector3(0.5f, -0.5f, 0.5f),
		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(-0.5f, 0.5f, 0.5f),
		math::Vector3(-0.5f, -0.5f, 0.5f),

		math::Vector3(-0.5f, 0.5f, 0.5f),
		math::Vector3(-0.5f, 0.5f, -0.5f),
		math::Vector3(-0.5f, -0.5f, -0.5f),
		math::Vector3(-0.5f, -0.5f, -0.5f),
		math::Vector3(-0.5f, -0.5f, 0.5f),
		math::Vector3(-0.5f, 0.5f, 0.5f),

		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, -0.5f),
		math::Vector3(0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(0.5f, -0.5f, 0.5f),

		math::Vector3(-0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, -0.5f, -0.5f),
		math::Vector3(0.5f, -0.5f, 0.5f),
		math::Vector3(0.5f, -0.5f, 0.5f),
		math::Vector3(-0.5f, -0.5f, 0.5f),
		math::Vector3(-0.5f, -0.5f, -0.5f),

		math::Vector3(-0.5f, 0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(0.5f, 0.5f, -0.5f),
		math::Vector3(0.5f, 0.5f, 0.5f),
		math::Vector3(-0.5f, 0.5f, -0.5f),
		math::Vector3(-0.5f, 0.5f, 0.5f),
	};

	m_uv = std::vector<math::Vector2>{
		math::Vector2(0.0f, 0.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(0.0f, 0.0f),
		math::Vector2(0.0f, 1.0f),

		math::Vector2(0.0f, 0.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(0.0f, 0.0f),

		math::Vector2(1.0f, 0.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(0.0f, 0.0f),
		math::Vector2(1.0f, 0.0f),

		math::Vector2(1.0f, 0.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(0.0f, 0.0f),

		math::Vector2(0.0f, 1.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(0.0f, 0.0f),
		math::Vector2(0.0f, 1.0f),

		math::Vector2(0.0f, 1.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(1.0f, 1.0f),
		math::Vector2(1.0f, 0.0f),
		math::Vector2(0.0f, 1.0f),
		math::Vector2(0.0f, 0.0f),
	};
	m_normals = std::vector<math::Vector3>{
		math::Vector3(0.0f, 0.0f, -1.0f),
		math::Vector3(0.0f, 0.0f, -1.0f),
		math::Vector3(0.0f, 0.0f, -1.0f),
		math::Vector3(0.0f, 0.0f, -1.0f),
		math::Vector3(0.0f, 0.0f, -1.0f),
		math::Vector3(0.0f, 0.0f, -1.0f),

		math::Vector3(0.0f, 0.0f, 1.0f),
		math::Vector3(0.0f, 0.0f, 1.0f),
		math::Vector3(0.0f, 0.0f, 1.0f),
		math::Vector3(0.0f, 0.0f, 1.0f),
		math::Vector3(0.0f, 0.0f, 1.0f),
		math::Vector3(0.0f, 0.0f, 1.0f),

		math::Vector3(-1.0f, 0.0f, 0.0f),
		math::Vector3(-1.0f, 0.0f, 0.0f),
		math::Vector3(-1.0f, 0.0f, 0.0f),
		math::Vector3(-1.0f, 0.0f, 0.0f),
		math::Vector3(-1.0f, 0.0f, 0.0f),
		math::Vector3(-1.0f, 0.0f, 0.0f),

		math::Vector3(1.0f, 0.0f, 0.0f),
		math::Vector3(1.0f, 0.0f, 0.0f),
		math::Vector3(1.0f, 0.0f, 0.0f),
		math::Vector3(1.0f, 0.0f, 0.0f),
		math::Vector3(1.0f, 0.0f, 0.0f),
		math::Vector3(1.0f, 0.0f, 0.0f),

		math::Vector3(0.0f, -1.0f, 0.0f),
		math::Vector3(0.0f, -1.0f, 0.0f),
		math::Vector3(0.0f, -1.0f, 0.0f),
		math::Vector3(0.0f, -1.0f, 0.0f),
		math::Vector3(0.0f, -1.0f, 0.0f),
		math::Vector3(0.0f, -1.0f, 0.0f),

		math::Vector3(0.0f, 1.0f, 0.0f),
		math::Vector3(0.0f, 1.0f, 0.0f),
		math::Vector3(0.0f, 1.0f, 0.0f),
		math::Vector3(0.0f, 1.0f, 0.0f),
		math::Vector3(0.0f, 1.0f, 0.0f),
		math::Vector3(0.0f, 1.0f, 0.0f),
	};

	topology = renderer::TRIANGLES;
	finalize();
  }

}
