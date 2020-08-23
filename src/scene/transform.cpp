#include "transform.h"
#include "entity.h"
#include "scene.h"
#include "scene_manager.h"
#include "tools/log.h"

namespace primal {

  Transform::Transform(Entity* const entity) : entity(entity) {}

  void Transform::setLocalToWorldMatrix(const math::Matrix4& newMatrix) {
	if (!entity->isMoveable()) return;

	m_localToWorldMatrix = newMatrix;
	for (int i = 0; i < math::Matrix3::ROW_COUNT; ++i) {
	  m_axis[i] = m_localToWorldMatrix.getCol(i).getVector3().normalized();
	}
	setDirty();
  }

  math::Vector3 Transform::getWorldPos() {
	return getLocalToWorldMatrix().getCol(3).getVector3();
  }

  math::Vector3 Transform::getLocalPos() const { return m_localPos; }

  void Transform::setWorldPos(const math::Vector3& newWorldPos) {
	if (!entity->isMoveable()) return;
	setDirty();

	if (m_parent == nullptr) {
	  m_localPos = newWorldPos;
	} else {
	  m_localPos = (m_parent->getWorldToLocalMatrix() * math::Vector4{ newWorldPos, 1 })
		.getVector3();
	}
  }

  void Transform::setLocalPos(const math::Vector3& newLocalPos) {
	if (!entity->isMoveable()) return;

	m_localPos = newLocalPos;
	setDirty();
  }

  void Transform::translateWorld(const math::Vector3& delta) {
	setWorldPos(getWorldPos() + delta);
  }

  void Transform::translateLocal(const math::Vector3& delta) {
	setLocalPos(m_localPos + delta);
  }

  math::Quaternion Transform::getWorldRot() {
	if (m_parent == nullptr) {
	  m_worldRot = m_localRot;
	} else {
	  m_worldRot = m_parent->getWorldRot() * m_localRot;
	}

	return m_worldRot;
  }

  math::Quaternion Transform::getLocalRot() const { return m_localRot; }

  math::Vector3 Transform::getWorldEulerAngles() {
	return getWorldRot().getEulerAngles();
  }

  math::Vector3 Transform::getLocalEulerAngles() const {
	return m_localRot.getEulerAngles();
  }

  void Transform::setWorldRot(const math::Quaternion& newWorldRot) {
	if (!entity->isMoveable()) return;

	m_worldRot = newWorldRot;
	setDirty();

	if (m_parent == nullptr) {
	  m_localRot = m_worldRot;
	} else {
	  m_localRot = m_parent->getWorldRot().getInverse() * m_worldRot;
	}
  }

  void Transform::setWorldRot(const math::Vector3& worldEulers) {
	setWorldRot(math::Quaternion::fromEulerAngles(worldEulers));
  }

  void Transform::setLocalRot(const math::Quaternion& newLocalRot) {
	if (!entity->isMoveable()) return;

	m_localRot = newLocalRot;
	setDirty();
  }

  void Transform::setLocalRot(const math::Vector3& localEulers) {
	setLocalRot(math::Quaternion::fromEulerAngles(localEulers));
  }

  void Transform::rotateWorld(const math::Vector3& eulerAngles) {
	setWorldRot(math::Quaternion::fromEulerAngles(eulerAngles) * getWorldRot());
  }

  void Transform::rotateWorld(const math::Vector3& m_axis, const float angle) {
	setWorldRot(math::Quaternion::fromAngleAxis(m_axis, angle) * getWorldRot());
  }

  void Transform::rotateLocal(const math::Vector3& eulerAngles) {
	// first, get the basis vectors in local space
	bool hasParent = getParent() != nullptr;
	math::Vector3 left, up, forward;
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

	setLocalRot(math::Quaternion::fromEulerAngles(left * eulerAngles.x + up * eulerAngles.y + forward * eulerAngles.z) * m_localRot);
  }

  void Transform::rotateLocal(const math::Vector3& axisWorldSpace,
	  const float angle) {
	math::Vector3 localAxis =
	  getParent() != nullptr ? getParent()->localDirFromWorldDir(axisWorldSpace) : axisWorldSpace;
	setLocalRot(math::Quaternion::fromAngleAxis(localAxis, angle) * m_localRot);
  }

  math::Vector3 Transform::getWorldScale() {
	if (m_parent == nullptr) {
	  m_worldScale = m_localScale;
	} else {
	  m_worldScale = math::Vector3::scale(m_parent->getWorldScale(), m_localScale);
	}

	return m_worldScale;
  }

  math::Vector3 Transform::getLocalScale() const { return m_localScale; }

  void Transform::setLocalScale(const math::Vector3& newScale) {
	if (!entity->isMoveable()) return;

	m_localScale = newScale;
	setDirty();
  }

  void Transform::setWorldScale(const math::Vector3& newWorldScale) {
	if (!entity->isMoveable()) return;

	m_worldScale = newWorldScale;
	setDirty();

	if (m_parent == nullptr) {
	  m_localScale = m_worldScale;
	} else {
	  m_localScale =
		math::Vector3::reverseScale(m_worldScale, m_parent->getWorldScale());
	}
  }

  void Transform::setParent(Transform* const transform, bool inheritTransform) {
	if (!entity->isMoveable()) return;

	Transform* targetTransform = transform;
	if (transform == nullptr) {
	  targetTransform =
		SceneManager::Instance().loadedScene->sceneRoot->transform;
	}
	if (m_parent == targetTransform) {
	  PRIMAL_CORE_WARN("You are trying to set {0}'s m_parent to ({1}), who is already its m_parent.", getName(), targetTransform->getName());
	  return;
	}
	math::Vector3 originalPos, originalScale;
	math::Quaternion originalRot;
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

  math::Vector3 Transform::getForward() {
	getLocalToWorldMatrix();
	return m_axis[2];
  }

  math::Vector3 Transform::getUp() {
	getLocalToWorldMatrix();
	return m_axis[1];
  }

  math::Vector3 Transform::getLeft() {
	getLocalToWorldMatrix();
	return m_axis[0];
  }

  math::Vector3 Transform::getAxis(const int i) {
	getLocalToWorldMatrix();
	return m_axis[i];
  }

  void Transform::lookAt(const math::Vector3& target,
	  const math::Vector3& worldUp) {
	math::Vector3 forwardDir = (target - getLocalPos()).normalized();
	math::Vector3 rightDir = math::Vector3::cross(forwardDir, worldUp).normalized();
	math::Vector3 upDir = math::Vector3::cross(rightDir, forwardDir);
	setLocalRot(math::Quaternion::fromLookRotation(forwardDir, upDir));
  }

  void Transform::lookAt(Transform& target, const math::Vector3& worldUp) {
	lookAt(target.getWorldPos(), worldUp);
  }

  Transform* Transform::getChild(const uint16_t childIndex) {
	if (childIndex >= getChildCount()) {
	  PRIMAL_CORE_ERROR("Transform::getChild: transform of ({0}) only has {1} m_children but you are asking for the {2}th one", getName().c_str(), getChildCount(), childIndex);
	}
	return m_children[childIndex];
  }

  std::string Transform::getName() const { return entity->getName(); }

  math::Vector3 Transform::worldPosFromLocalPos(const math::Vector3& localPoint) {
	return (getLocalToWorldMatrix() * math::Vector4{ localPoint, 1 }).getVector3();
  }

  math::Vector3 Transform::LocalPosFromWorldPos(const math::Vector3& worldPoint) {
	return (getWorldToLocalMatrix() * math::Vector4{ worldPoint, 1 }).getVector3();
  }

  math::Vector3 Transform::worldDirFromLocalDir(
	  const math::Vector3& localDirection) {
	return (getLocalToWorldMatrix() * math::Vector4{ localDirection, 0 })
	  .getVector3();
  }

  math::Vector3 Transform::localDirFromWorldDir(
	  const math::Vector3& worldDirection) {
	return (getWorldToLocalMatrix() * math::Vector4{ worldDirection, 0 })
	  .getVector3();
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

  void Transform::setWorldTransform(const math::Vector3& inPosition,
	  const math::Vector3& inEulerAngles,
	  const math::Vector3& inScale) {
	if (!entity->isMoveable()) return;

	setWorldPos(inPosition);
	setWorldRot(inEulerAngles);
	setLocalScale(inScale);
  }

  const math::Matrix4& Transform::getLocalToWorldMatrix() {
	if (isDirty) {
	  recalculateLocalToWorldMatrix();
	  isDirty = false;
	}
	return m_localToWorldMatrix;
  }

  const math::Matrix4& Transform::getWorldToLocalMatrix() {
	if (isWorldToLocalDirty) {
	  m_worldToLocalMatrix = getLocalToWorldMatrix().inverse();
	  isWorldToLocalDirty = false;
	}
	return m_worldToLocalMatrix;
  }

  void Transform::recalculateLocalToWorldMatrix() {
	math::Matrix4 m_localToParentMatrix{};
	m_localToParentMatrix.setTopLeftMatrix3(m_localRot.getMatrix3()); // rotation
	math::Matrix4 temp;
	temp.setDiagonal(m_localScale.x, m_localScale.y, m_localScale.z, 1);
	m_localToParentMatrix = m_localToParentMatrix * temp; // scale
	m_localToParentMatrix.setCol(3, m_localPos, 1);

	if (m_parent != nullptr) {
	  m_localToWorldMatrix = m_parent->getLocalToWorldMatrix() * m_localToParentMatrix;
	} else {
	  m_localToWorldMatrix = m_localToParentMatrix;
	}
	for (int i = 0; i < math::Matrix3::ROW_COUNT; ++i) {
	  m_axis[i] = m_localToWorldMatrix.getCol(i).getVector3().normalized();
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
