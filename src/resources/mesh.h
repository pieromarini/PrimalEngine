#ifndef MESH_H
#define MESH_H

#include <vector>
#include <functional>
#include <cstdint>

#include "core/math/linear_algebra/vector.h"

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
	  Mesh(std::vector<math::vec3> positions, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::vec3> positions, std::vector<math::vec2> uv, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::vec3> positions, std::vector<math::vec2> uv, std::vector<math::vec3> normals, std::vector<uint32_t> indices);
	  Mesh(std::vector<math::vec3> positions, std::vector<math::vec2> uv, std::vector<math::vec3> normals, std::vector<math::vec3> tangents, std::vector<math::vec3> bitangents, std::vector<uint32_t> indices);

	  void setPositions(std::vector<math::vec3> positions);
	  void setUVs(std::vector<math::vec2> uv);
	  void setNormals(std::vector<math::vec3> normals);
	  void setTangents(std::vector<math::vec3> tangents, std::vector<math::vec3> bitangents);

	  void finalize(bool interleaved = true);

	  void fromSDF(std::function<float(math::vec3)>& sdf, float maxDistance, uint16_t gridResolution);

	  uint32_t m_VAO = 0;
	  uint32_t m_VBO;
	  uint32_t m_EBO;

	  std::vector<math::vec3> m_positions;
	  std::vector<math::vec2> m_uv;
	  std::vector<math::vec3> m_normals;
	  std::vector<math::vec3> m_tangents;
	  std::vector<math::vec3> m_bitangents;

	  TOPOLOGY topology = TRIANGLES;
	  std::vector<uint32_t> m_indices;

	private:
	  void calculateNormals(bool smooth = true);
	  void calculateTangents();
  };

}

#endif
