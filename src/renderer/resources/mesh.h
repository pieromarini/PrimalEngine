#ifndef MESH_H
#define MESH_H

#include <vector>
#include <functional>
#include <cstdint>

#include "core/math/vector2.h"
#include "core/math/vector3.h"

namespace primal::renderer {

  enum TOPOLOGY {
	POINTS,
	LINES,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
  };

  class Mesh {
	public:
	  Mesh();
	  Mesh(std::vector<math::Vector3> positions, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::Vector3> positions, std::vector<math::Vector2> uv, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::Vector3> positions, std::vector<math::Vector2> uv, std::vector<math::Vector3> normals, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::Vector3> positions, std::vector<math::Vector2> uv, std::vector<math::Vector3> normals, std::vector<math::Vector3> tangents, std::vector<math::Vector3> bitangents, std::vector<uint32_t> indices);

	  void setPositions(std::vector<math::Vector3> positions);
	  void setUVs(std::vector<math::Vector2> uv);
	  void setNormals(std::vector<math::Vector3> normals);
	  void setTangents(std::vector<math::Vector3> tangents, std::vector<math::Vector3> bitangents);

	  void finalize(bool interleaved = true);

	  void fromSDF(std::function<float(math::Vector3)>& sdf, float maxDistance, uint16_t gridResolution);

	  uint32_t m_VAO = 0;
	  uint32_t m_VBO;
	  uint32_t m_EBO;

	  std::vector<math::Vector3> m_positions;
	  std::vector<math::Vector2> m_uv;
	  std::vector<math::Vector3> m_normals;
	  std::vector<math::Vector3> m_tangents;
	  std::vector<math::Vector3> m_bitangents;

	  TOPOLOGY topology = TRIANGLES;
	  std::vector<uint32_t> m_indices;

	private:
	  void calculateNormals(bool smooth = true);
	  void calculateTangents();
  };

}

#endif
