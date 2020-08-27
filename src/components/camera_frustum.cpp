#include <cmath>

#include "camera_frustum.h"
#include "camera.h"

namespace primal {

  void CameraFrustum::update(Camera* camera) {
	float tan = 2.0 * std::tan(camera->FOV * 0.5f);
	float nearHeight = tan * camera->Near;
	float nearWidth = nearHeight * camera->Aspect;
	float farHeight = tan * camera->Far;
	float farWidth = farHeight * camera->Aspect;

	math::Vector3 nearCenter = camera->Position + camera->Forward * camera->Near;
	math::Vector3 farCenter = camera->Position + camera->Forward * camera->Far;

	math::Vector3 v;

	// left plane
	v = (nearCenter - camera->Right * nearWidth * 0.5f) - camera->Position;
	Left.setNormalD(math::Vector3::cross(v.normalize(), camera->Up), nearCenter - camera->Right * nearWidth * 0.5f);

	// right plane
	v = (nearCenter + camera->Right * nearWidth * 0.5f) - camera->Position;
	Right.setNormalD(math::Vector3::cross(camera->Up, v.normalize()), nearCenter + camera->Right * nearWidth * 0.5f);

	// top plane
	v = (nearCenter + camera->Up * nearHeight * 0.5f) - camera->Position;
	Top.setNormalD(math::Vector3::cross(v.normalized(), camera->Right), nearCenter + camera->Up * nearHeight * 0.5f);

	// bottom plane
	v = (nearCenter - camera->Up * nearHeight * 0.5f) - camera->Position;
	Bottom.setNormalD(math::Vector3::cross(camera->Right, v.normalize()), nearCenter - camera->Up * nearHeight * 0.5f);

	// near plane
	Near.setNormalD(camera->Forward, nearCenter);
	Far.setNormalD(-camera->Forward, farCenter);
  }
  // ------------------------------------------------------------------------
  bool CameraFrustum::intersect(math::Vector3 point) {
	for (int i = 0; i < 6; ++i) {
	  if (Planes(i).Distance(point) < 0) {
		return false;
	  }
	}
	return true;
  }

  bool CameraFrustum::intersect(math::Vector3 point, float radius) {
	for (int i = 0; i < 6; ++i) {
	  if (Planes(i).Distance(point) < -radius) {
		return false;
	  }
	}
	return true;
  }

  bool CameraFrustum::intersect(math::Vector3 boxMin, math::Vector3 boxMax) {
	for (int i = 0; i < 6; ++i) {
	  math::Vector3 positive = boxMin;
	  if (Planes(i).Normal.x >= 0) {
		positive.x = boxMax.x;
	  }
	  if (Planes(i).Normal.y >= 0) {
		positive.y = boxMax.y;
	  }
	  if (Planes(i).Normal.z >= 0) {
		positive.z = boxMax.z;
	  }
	  if (Planes(i).Distance(positive) < 0) {
		return false;
	  }
	}
	return true;
  }

}
