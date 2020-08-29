#include "transform.h"
#include "core/math/linear_algebra/transformation.h"
#include "core/math/trigonometry/conversions.h"
#include "entity.h"
#include "scene.h"
#include "scene_manager.h"
#include "tools/log.h"

namespace primal {

  Transform::Transform(Entity* const entity) : entity(entity) {}

  void Transform::setLocalToWorldMatrix(const math::mat4& newMatrix) {
	if (!entity->isMoveable()) return;

	m_localToWorldMatrix = newMatrix;
	for (int i = 0; i < 3; ++i) {
	  m_axis[i] = math::normalize(m_localToWorldMatrix[i].xyz);
	}
	setDirty();
  }

  math::vec3 Transform::getWorldPos() {
	return getLocalToWorldMatrix()[3].xyz;
  }

  math::vec3 Transform::getLocalPos() const { return m_localPos; }

  void Transform::setWorldPos(const math::vec3& newWorldPos) {
	if (!entity->isMoveable()) return;
	setDirty();

	if (m_parent == nullptr) {
	  m_localPos = newWorldPos;
	} else {
	  m_localPos = (m_parent->getWorldToLocalMatrix() * math::vec4{ newWorldPos, 1 }).xyz;
	}
  }

  void Transform::setLocalPos(const math::vec3& newLocalPos) {
	if (!entity->isMoveable()) return;

	m_localPos = newLocalPos;
	setDirty();
  }

  void Transform::translateWorld(const math::vec3& delta) {
	setWorldPos(getWorldPos() + delta);
  }

  void Transform::translateLocal(const math::vec3& delta) {
	setLocalPos(m_localPos + delta);
  }

  math::quaternion Transform::getWorldRot() {
	if (m_parent == nullptr) {
	  m_worldRot = m_localRot;
	} else {
	  m_worldRot = m_parent->getWorldRot() * m_localRot;
	}

	return m_worldRot;
  }

  math::quaternion Transform::getLocalRot() const { return m_localRot; }

  math::vec3 Transform::getWorldEulerAngles() {
	return getWorldRot().getEulerAngles();
  }

  math::vec3 Transform::getLocalEulerAngles() const {
	return m_localRot.getEulerAngles();
  }

  void Transform::setWorldRot(const math::quaternion& newWorldRot) {
	if (!entity->isMoveable()) return;

	m_worldRot = newWorldRot;
	setDirty();

	if (m_parent == nullptr) {
	  m_localRot = m_worldRot;
	} else {
	  m_localRot = m_parent->getWorldRot().getInverse() * m_worldRot;
	}
  }

  void Transform::setWorldRot(const math::vec3& worldEulers) {
	setWorldRot(math::quaternion::fromEulerAngles(worldEulers));
  }

  void Transform::setLocalRot(const math::quaternion& newLocalRot) {
	if (!entity->isMoveable()) return;

	m_localRot = newLocalRot;
	setDirty();
  }

  void Transform::setLocalRot(const math::vec3& localEulers) {
	setLocalRot(math::quaternion::fromEulerAngles(localEulers));
  }

  void Transform::rotateWorld(const math::vec3& eulerAngles) {
	setWorldRot(math::quaternion::fromEulerAngles(eulerAngles) * getWorldRot());
  }

  void Transform::rotateWorld(const math::vec3& m_axis, const float angle) {
	setWorldRot(math::quaternion{ m_axis, math::deg2Rad(angle) } * getWorldRot());
  }

  void Transform::rotateLocal(const math::vec3& eulerAngles) {
	// first, get the basis vectors in local space
	bool hasParent = getParent() != nullptr;
	math::vec3 left, up, forward;
	if (hasParent) {
	  Transform* m_parent = getParent();
	  left = m_parent->localDirFromWorldDir(getLeft());
	  up = m_parent->localDirFromWorldDir(getUp());
	  forward = m_parent->localDirFromWorldDir(getForward());
	} else {
	  left = getLeft();
	  up = getUp();
	  forward = getForward();
	}

	setLocalRot(math::quaternion::fromEulerAngles(left * eulerAngles.x + up * eulerAngles.y + forward * eulerAngles.z) * m_localRot);
  }

  void Transform::rotateLocal(const math::vec3& axisWorldSpace,
	  const float angle) {
	math::vec3 localAxis =
	  getParent() != nullptr ? getParent()->localDirFromWorldDir(axisWorldSpace) : axisWorldSpace;
	setLocalRot(math::quaternion{ localAxis, math::deg2Rad(angle) } * m_localRot);
  }

  math::vec3 Transform::getWorldScale() {
	if (m_parent == nullptr) {
	  m_worldScale = m_localScale;
	} else {
	  m_worldScale = math::scale(m_parent->getWorldScale(), m_localScale);
	}

	return m_worldScale;
  }

  math::vec3 Transform::getLocalScale() const { return m_localScale; }

  void Transform::setLocalScale(const math::vec3& newScale) {
	if (!entity->isMoveable()) return;

	m_localScale = newScale;
	setDirty();
  }

  void Transform::setWorldScale(const math::vec3& newWorldScale) {
	if (!entity->isMoveable()) return;

	m_worldScale = newWorldScale;
	setDirty();

	if (m_parent == nullptr) {
	  m_localScale = m_worldScale;
	} else {
	  m_localScale = math::reverseScale(m_worldScale, m_parent->getWorldScale());
	}
  }

  void Transform::setParent(Transform* const transform, bool inheritTransform) {
	if (!entity->isMoveable()) return;

	Transform* targetTransform = transform;
	if (transform == nullptr) {
	  targetTransform =
		SceneManager::instance().loadedScene->sceneRoot->transform;
	}
	if (m_parent == targetTransform) {
	  PRIMAL_CORE_WARN("You are trying to set {0}'s m_parent to ({1}), who is already its m_parent.", getName(), targetTransform->getName());
	  return;
	}
	math::vec3 originalPos, originalScale;
	math::quaternion originalRot;
	if (!inheritTransform) {
	  originalPos = getWorldPos();
	  originalRot = getWorldRot();
	  originalScale = getWorldScale();
	}

	if (m_parent != nullptr) {
	  m_parent->removeChild(this);
	}
	if (targetTransform != nullptr) {
	  targetTransform->addChild(this);
	}
	m_parent = targetTransform;
	if (!inheritTransform) {
	  setWorldPos(originalPos);
	  setWorldRot(originalRot);
	  setWorldScale(originalScale);
	}
	setDirty();
  }

  math::vec3 Transform::getForward() {
	getLocalToWorldMatrix();
	return m_axis[2];
  }

  math::vec3 Transform::getUp() {
	getLocalToWorldMatrix();
	return m_axis[1];
  }

  math::vec3 Transform::getLeft() {
	getLocalToWorldMatrix();
	return m_axis[0];
  }

  math::vec3 Transform::getAxis(const int i) {
	getLocalToWorldMatrix();
	return m_axis[i];
  }

  void Transform::lookAt(const math::vec3& target, const math::vec3& worldUp) {
	math::vec3 forwardDir = math::normalize(target - getLocalPos());
	math::vec3 rightDir = math::normalize(math::cross(forwardDir, worldUp));
	math::vec3 upDir = math::cross(rightDir, forwardDir);
	setLocalRot(math::quaternion::fromLookRotation(forwardDir, upDir));
  }

  void Transform::lookAt(Transform& target, const math::vec3& worldUp) {
	lookAt(target.getWorldPos(), worldUp);
  }

  Transform* Transform::getChild(const uint16_t childIndex) {
	if (childIndex >= getChildCount()) {
	  PRIMAL_CORE_ERROR("Transform::getChild: transform of ({0}) only has {1} m_children but you are asking for the {2}th one", getName().c_str(), getChildCount(), childIndex);
	}
	return m_children[childIndex];
  }

  std::string Transform::getName() const { return entity->getName(); }

  math::vec3 Transform::worldPosFromLocalPos(const math::vec3& localPoint) {
	return (getLocalToWorldMatrix() * math::vec4{ localPoint, 1 }).xyz;
  }

  math::vec3 Transform::LocalPosFromWorldPos(const math::vec3& worldPoint) {
	return (getWorldToLocalMatrix() * math::vec4{ worldPoint, 1 }).xyz;
  }

  math::vec3 Transform::worldDirFromLocalDir(const math::vec3& localDirection) {
	return (getLocalToWorldMatrix() * math::vec4{ localDirection, 0 }).xyz;
  }

  math::vec3 Transform::localDirFromWorldDir(const math::vec3& worldDirection) {
	return (getWorldToLocalMatrix() * math::vec4{ worldDirection, 0 }).xyz;
  }

  void Transform::forChildren(const Action<Transform*>& action) {
	for (auto& child : m_children) {
	  action(child);
	}
  }

  void Transform::forDescendants(const Action<Transform*>& action) {
	for (auto& child : m_children) {
	  action(child);
	  child->forDescendants(action);
	}
  }

  void Transform::setWorldTransform(const math::vec3& inPosition,
	  const math::vec3& inEulerAngles,
	  const math::vec3& inScale) {
	if (!entity->isMoveable()) return;

	setWorldPos(inPosition);
	setWorldRot(inEulerAngles);
	setLocalScale(inScale);
  }

  const math::mat4& Transform::getLocalToWorldMatrix() {
	if (isDirty) {
	  recalculateLocalToWorldMatrix();
	  isDirty = false;
	}
	return m_localToWorldMatrix;
  }

  const math::mat4& Transform::getWorldToLocalMatrix() {
	if (isWorldToLocalDirty) {
	  m_worldToLocalMatrix = math::inverse(getLocalToWorldMatrix());
	  isWorldToLocalDirty = false;
	}
	return m_worldToLocalMatrix;
  }

  void Transform::recalculateLocalToWorldMatrix() {
	math::mat4 m_localToParentMatrix{};
	m_localToParentMatrix.setTopLeftMatrix3(m_localRot.getMatrix3()); // rotation
	math::mat4 temp;
	temp.setDiagonal({ m_localScale.x, m_localScale.y, m_localScale.z, 1 });
	m_localToParentMatrix = m_localToParentMatrix * temp; // scale
	m_localToParentMatrix.setCol(3, { m_localPos.x, m_localPos.y, m_localPos.z, 1 });

	if (m_parent != nullptr) {
	  m_localToWorldMatrix = m_parent->getLocalToWorldMatrix() * m_localToParentMatrix;
	} else {
	  m_localToWorldMatrix = m_localToParentMatrix;
	}
	for (int i = 0; i < 3; ++i) {
	  m_axis[i] = math::normalize(m_localToWorldMatrix[i].xyz);
	}
  }

  void Transform::addChild(Transform* transform) {
	m_children.push_back(transform);
  }

  void Transform::removeChild(Transform* transform) {
	for (auto it = m_children.begin(); it != m_children.end(); ++it) {
	  if (*it == transform) {
		m_children.erase(it);
		return;
	  }
	}

	PRIMAL_CORE_ERROR("Transform::removeChild => child {0} doesn't exist!", transform->getName());
  }

  void Transform::setDirty() {
	isDirty = true;
	isWorldToLocalDirty = true;
	forDescendants([](Transform* trans) {
	  trans->isDirty = true;
	  trans->isWorldToLocalDirty = true;
	});
  }

}
