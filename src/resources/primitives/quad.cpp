#include "quad.h"

namespace primal {

  Quad::Quad() {
	m_positions = {
	  {
		-1.0f,
		1.0f,
		0.0f,
	  },
	  {
		-1.0f,
		-1.0f,
		0.0f,
	  },
	  {
		1.0f,
		1.0f,
		0.0f,
	  },
	  {
		1.0f,
		-1.0f,
		0.0f,
	  },
	};
	m_uv = {
	  {
		0.0f,
		1.0f,
	  },
	  {
		0.0f,
		0.0f,
	  },
	  {
		1.0f,
		1.0f,
	  },
	  {
		1.0f,
		0.0f,
	  },
	};
	topology = renderer::TRIANGLE_STRIP;

	finalize();
  }

  Quad::Quad(float width, float height) {
	m_positions = {
	  {
		-width,
		height,
		0.0f,
	  },
	  {
		-width,
		-height,
		0.0f,
	  },
	  {
		width,
		height,
		0.0f,
	  },
	  {
		width,
		-height,
		0.0f,
	  },
	};
	m_uv = {
	  {
		0.0f,
		1.0f,
	  },
	  {
		0.0f,
		0.0f,
	  },
	  {
		1.0f,
		1.0f,
	  },
	  {
		1.0f,
		0.0f,
	  },
	};
	topology = renderer::TRIANGLE_STRIP;

	finalize();
  }

}
