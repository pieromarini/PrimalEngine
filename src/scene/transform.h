#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vector>

#include "core/data_structures/delegate.h"
#include "core/math/matrix4.h"
#include "core/math/matrix3.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/math/quaternion.h"

namespace primal {

  class Transform {
	public:
	  Transform() = delete;
	  explicit Transform(class Entity* const entity);
	  ~Transform() = default;

	  math::Vector3 getWorldPos();
	  [[nodiscard]] math::Vector3 getLocalPos() const;

	  void setWorldPos(const math::Vector3& newWorldPos);
	  void setLocalPos(const math::Vector3& newLocalPos);

	  void translateWorld(const math::Vector3& delta);
	  void translateLocal(const math::Vector3& delta);

	  math::Quaternion getWorldRot();
	  [[nodiscard]] math::Quaternion getLocalRot() const;

	  math::Vector3 getWorldEulerAngles();
	  [[nodiscard]] math::Vector3 getLocalEulerAngles() const;

	  void setWorldRot(const math::Quaternion& newWorldRot);
	  void setWorldRot(const math::Vector3& worldEulers);
	  void setLocalRot(const math::Quaternion& newLocalRot);
	  void setLocalRot(const math::Vector3& localEulers);

	  void rotateWorld(const math::Vector3& eulerAngles);
	  void rotateWorld(const math::Vector3& axis, float angle);
	  void rotateLocal(const math::Vector3& eulerAngles);
	  void rotateLocal(const math::Vector3& axisWorldSpace, float angle);

	  math::Vector3 getWorldScale();
	  [[nodiscard]] math::Vector3 getLocalScale() const;

	  void setLocalScale(const math::Vector3& newScale);
	  void setWorldScale(const math::Vector3& newWorldScale);

	  void setParent(Transform* const transform, bool inheritTransform = true);
	  [[nodiscard]] Transform* getParent() const { return m_parent; }

	  math::Vector3 getForward();
	  math::Vector3 getUp();
	  math::Vector3 getLeft();
	  math::Vector3 getAxis(int i);

	  const math::Matrix4& getLocalToWorldMatrix();
	  const math::Matrix4& getWorldToLocalMatrix();

	  void setLocalToWorldMatrix(const math::Matrix4& newMatrix);

	  void lookAt(const math::Vector3& target, const math::Vector3& worldUp = math::Vector3::up);
	  void lookAt(Transform& target, const math::Vector3& worldUp = math::Vector3::up);

	  [[nodiscard]] std::size_t getChildCount() const { return m_children.size(); }
	  Transform* getChild(uint16_t childIndex);

	  [[nodiscard]] inline std::string getName() const;

	  math::Vector3 worldPosFromLocalPos(const math::Vector3& localPoint);

	  math::Vector3 LocalPosFromWorldPos(const math::Vector3& worldPoint);

	  math::Vector3 worldDirFromLocalDir(const math::Vector3& localDirection);

	  math::Vector3 localDirFromWorldDir(const math::Vector3& worldDirection);

	  void forChildren(const Action<Transform*>& action);

	  void forDescendants(const Action<Transform*>& action);

	  void setWorldTransform(const math::Vector3& inPosition, const math::Vector3& inEulerAngles, const math::Vector3& inScale);

	  using iterator = std::vector<Transform*>::iterator;
	  using const_iterator = std::vector<Transform*>::const_iterator;

	  iterator begin() { return m_children.begin(); }
	  [[nodiscard]] const_iterator begin() const { return m_children.begin(); }

	  iterator end() { return m_children.end(); }
	  [[nodiscard]] const_iterator end() const { return m_children.end(); }

	  void addChild(Transform* transform);
	  void removeChild(Transform* transform);

	  class Entity* const entity{ nullptr };

	private:
	  friend class Entity;
	  void recalculateLocalToWorldMatrix();

	  // cached results
	  math::Quaternion m_worldRot;
	  math::Vector3 m_worldScale;
	  math::Matrix4 m_localToWorldMatrix{};
	  math::Matrix4 m_worldToLocalMatrix{};

	  // union
	  std::array<math::Vector3, 3> m_axis;
	  math::Vector3& m_left = m_axis[0];
	  math::Vector3& m_up = m_axis[1];
	  math::Vector3& m_forward = m_axis[2];

	  // local storage
	  math::Vector3 m_localPos{ math::Vector3::zero };
	  math::Quaternion m_localRot{ math::Quaternion::identity };
	  math::Vector3 m_localScale{ math::Vector3::one };

	  void setDirty();
	  bool isDirty{ true };
	  bool isWorldToLocalDirty{ true };

	  Transform* m_parent{ nullptr };
	  std::vector<Transform*> m_children;
  };

}

#endif
