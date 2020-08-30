#ifndef CAMERA_FRUSTUM_H
#define CAMERA_FRUSTUM_H

#include "core/math/linear_algebra/operation.h"
#include "core/math/linear_algebra/vector.h"

namespace primal {

  class Camera;

  /* 
	Data container for the 3D plane equation variables in Cartesian space. A plane equation can be
	defined by its Normal (perpendicular to the plane) and a distance value D obtained from any
	point on the plane itself (projected onto the normal vector).
  */
  struct FrustumPlane {
	math::vec3 Normal;
	float D;

	void setNormalD(math::vec3 normal, math::vec3 point) {
	  Normal = math::normalize(normal);
	  D = math::dot(normal, point);
	}

	float distance(math::vec3 point) {
	  return math::dot(Normal, point) + D;
	}
  };

  /*
	Container object managing all 6 camera frustum planes as calculated from any Camera object.
	The CameraFrustum allows to quickly determine (using simple collision primitives like point,
	sphere, box) whether an object is within the frustum (or crossing the frustum's edge(s)). 
	This gives us the option to significantly reduce draw calls for objects that aren't visible
	anyways. Note that the frustum needs to be re-calculated every frame.
  */
  class CameraFrustum {
	public:
	  FrustumPlane Left;
	  FrustumPlane Right;
	  FrustumPlane Top;
	  FrustumPlane Bottom;
	  FrustumPlane Near;
	  FrustumPlane Far;

	public:
	  CameraFrustum() {}

	  FrustumPlane& planes(int i) { return (&Left)[i]; }
	  const FrustumPlane& planes(int i) const { return (&Left)[i]; }

	  void update(Camera* camera);

	  bool intersect(math::vec3 point);
	  bool intersect(math::vec3 point, float radius);
	  bool intersect(math::vec3 boxMin, math::vec3 boxMax);
  };

}

#endif
