#include <cmath>

#include "camera_frustum.h"
#include "camera.h"
#include "core/math/linear_algebra/quaternion.h"
#include "core/math/math.h"

namespace primal {

  void CameraFrustum::update(Camera* camera) {
	float tan = 2.0 * std::tan(camera->FOV * 0.5f);
	float nearHeight = tan * camera->Near;
	float nearWidth = nearHeight * camera->Aspect;
	float farHeight = tan * camera->Far;
	float farWidth = farHeight * camera->Aspect;

	math::vec3 nearCenter = camera->Position + camera->Forward * camera->Near;
	math::vec3 farCenter = camera->Position + camera->Forward * camera->Far;

	math::vec3 v;

	// left plane
	v = (nearCenter - camera->Right * nearWidth * 0.5f) - camera->Position;
	Left.setNormalD(math::cross(math::normalize(v), camera->Up), nearCenter - camera->Right * nearWidth * 0.5f);

	// right plane
	v = (nearCenter + camera->Right * nearWidth * 0.5f) - camera->Position;
	Right.setNormalD(math::cross(camera->Up, math::normalize(v)), nearCenter + camera->Right * nearWidth * 0.5f);

	// top plane
	v = (nearCenter + camera->Up * nearHeight * 0.5f) - camera->Position;
	Top.setNormalD(math::cross(math::normalize(v), camera->Right), nearCenter + camera->Up * nearHeight * 0.5f);

	// bottom plane
	v = (nearCenter - camera->Up * nearHeight * 0.5f) - camera->Position;
	Bottom.setNormalD(math::cross(camera->Right, math::normalize(v)), nearCenter - camera->Up * nearHeight * 0.5f);

	// near plane
	Near.setNormalD(camera->Forward, nearCenter);
	Far.setNormalD(-camera->Forward, farCenter);
  }
  bool CameraFrustum::intersect(math::vec3 point) {
	for (int i = 0; i < 6; ++i) {
	  if (planes(i).distance(point) < 0) {
		return false;
	  }
	}
	return true;
  }

  bool CameraFrustum::intersect(math::vec3 point, float radius) {
	for (int i = 0; i < 6; ++i) {
	  if (planes(i).distance(point) < -radius) {
		return false;
	  }
	}
	return true;
  }

  bool CameraFrustum::intersect(math::vec3 boxMin, math::vec3 boxMax) {
	for (int i = 0; i < 6; ++i) {
	  math::vec3 positive = boxMin;
	  if (planes(i).Normal.x >= 0) {
		positive.x = boxMax.x;
	  }
	  if (planes(i).Normal.y >= 0) {
		positive.y = boxMax.y;
	  }
	  if (planes(i).Normal.z >= 0) {
		positive.z = boxMax.z;
	  }
	  if (planes(i).distance(positive) < 0) {
		return false;
	  }
	}
	return true;
  }

}
