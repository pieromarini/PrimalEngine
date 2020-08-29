#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <string>
#include <vector>

#include "core/data_structures/delegate.h"
#include "core/math/linear_algebra/vector.h"
#include "core/math/linear_algebra/matrix.h"
#include "core/math/linear_algebra/quaternion.h"

namespace primal {

  class Transform {
	public:
	  Transform() = delete;
	  explicit Transform(class Entity* const entity);
	  ~Transform() = default;

	  math::vec3 getWorldPos();
	  [[nodiscard]] math::vec3 getLocalPos() const;

	  void setWorldPos(const math::vec3& newWorldPos);
	  void setLocalPos(const math::vec3& newLocalPos);

	  void translateWorld(const math::vec3& delta);
	  void translateLocal(const math::vec3& delta);

	  math::quaternion getWorldRot();
	  [[nodiscard]] math::quaternion getLocalRot() const;

	  math::vec3 getWorldEulerAngles();
	  [[nodiscard]] math::vec3 getLocalEulerAngles() const;

	  void setWorldRot(const math::quaternion& newWorldRot);
	  void setWorldRot(const math::vec3& worldEulers);
	  void setLocalRot(const math::quaternion& newLocalRot);
	  void setLocalRot(const math::vec3& localEulers);

	  void rotateWorld(const math::vec3& eulerAngles);
	  void rotateWorld(const math::vec3& axis, float angle);
	  void rotateLocal(const math::vec3& eulerAngles);
	  void rotateLocal(const math::vec3& axisWorldSpace, float angle);

	  math::vec3 getWorldScale();
	  [[nodiscard]] math::vec3 getLocalScale() const;

	  void setLocalScale(const math::vec3& newScale);
	  void setWorldScale(const math::vec3& newWorldScale);

	  void setParent(Transform* const transform, bool inheritTransform = true);
	  [[nodiscard]] Transform* getParent() const { return m_parent; }

	  math::vec3 getForward();
	  math::vec3 getUp();
	  math::vec3 getLeft();
	  math::vec3 getAxis(int i);

	  const math::mat4& getLocalToWorldMatrix();
	  const math::mat4& getWorldToLocalMatrix();

	  void setLocalToWorldMatrix(const math::mat4& newMatrix);

	  void lookAt(const math::vec3& target, const math::vec3& worldUp = math::vec3::UP);
	  void lookAt(Transform& target, const math::vec3& worldUp = math::vec3::UP);

	  [[nodiscard]] std::size_t getChildCount() const { return m_children.size(); }
	  Transform* getChild(uint16_t childIndex);

	  [[nodiscard]] inline std::string getName() const;

	  math::vec3 worldPosFromLocalPos(const math::vec3& localPoint);

	  math::vec3 LocalPosFromWorldPos(const math::vec3& worldPoint);

	  math::vec3 worldDirFromLocalDir(const math::vec3& localDirection);

	  math::vec3 localDirFromWorldDir(const math::vec3& worldDirection);

	  void forChildren(const Action<Transform*>& action);

	  void forDescendants(const Action<Transform*>& action);

	  void setWorldTransform(const math::vec3& inPosition, const math::vec3& inEulerAngles, const math::vec3& inScale);

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
	  math::quaternion m_worldRot;
	  math::vec3 m_worldScale;
	  math::mat4 m_localToWorldMatrix{};
	  math::mat4 m_worldToLocalMatrix{};

	  // union
	  std::array<math::vec3, 3> m_axis;
	  math::vec3& m_left = m_axis[0];
	  math::vec3& m_up = m_axis[1];
	  math::vec3& m_forward = m_axis[2];

	  // local storage
	  math::vec3 m_localPos{ 0 };
	  math::quaternion m_localRot{ 0.0, 0.0, 0.0, 1.0 };
	  math::vec3 m_localScale{ 1.0, 1.0, 1.0 };

	  void setDirty();
	  bool isDirty{ true };
	  bool isWorldToLocalDirty{ true };

	  Transform* m_parent{ nullptr };
	  std::vector<Transform*> m_children;
  };

}

#endif
