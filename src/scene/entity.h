#ifndef ENTITY_H
#define ENTITY_H

#include <typeinfo>
#include <typeindex>
#include <execution>
#include <bitset>
#include <array>
#include "component.h"
#include "tools/log.h"
#include "transform.h"
#include "scene_manager.h"
#include "scene.h"
#include "core/math/vector3.h"

namespace primal {

  class Entity {
	public:
	  ~Entity();
	  bool operator==(const Entity& rhs) const;

	  Transform* transform{};

	  void setName(std::string_view name) {
		m_entityName = name;
	  }

	  std::string getName() const {
		return m_entityName;
	  }

	  auto getEntityID() const {
		return m_entityID;
	  }

	  std::string getEntityIDString() const;

	  static Entity* create(std::string name, class Entity* parent = nullptr, bool entityStatic = false);

	  static void destroy(Entity* entity);

	  static void destroyHelper(Entity* entity);

	  static void destroyImmediately(Entity* entity);

	  static Entity* getEntityByName(const std::string& name);

	  static std::list<Entity*> getEntitiesByName(const std::string& name);
	  
	  void setActive(bool isActive);
	  bool getActive() const;

	  bool isMoveable() const;

	  template<typename T, typename... Args>
	  T* addComponent(Args&&... args);

	  template<typename T, bool IsActive, typename... Args>
	  T* addComponent(Args&&... args);

	  template<typename T>
	  T* getComponent();

	  template<typename T>
	  std::vector<T*> getComponents();

	  template<typename T>
	  T* getComponentInParent();

	  template<typename T>
	  std::vector<T*> getComponentsInParent();

	  template<typename T>
	  T* getComponentInChildren();

	  template<typename T>
	  std::vector<T*> getComponentsInChildren();

	  /* TODO: Implement
	  template<typename T>
	  T* getComponentInDescendant();

	  template<typename T>
	  std::vector<T*> getComponentsInDescendant();
	  */

	  void setTransform(const math::Vector3& worldPos = math::Vector3::zero,
					    const math::Vector3& worldEulerAngles = math::Vector3::zero,
						const math::Vector3& localScale = math::Vector3::one);

	  void setLayer(int layer);
	  void setLayer(std::string layer);
	  int getLayerIndex() const;
	  std::string getLayerName() const;

	  const bool isStatic;

	private:
	  enum class EntityAttributes {
		IS_ACTIVE,
		NEED_DESTROY,
		IS_TRANSFORM_DIRTY
	  };

	  friend class GraphicsModule;
	  friend class Scene;

	  std::vector<std::type_index> m_componentTypes;
	  std::vector<class Component*> m_components;
	  Transform m_internalTransform;

	  std::bitset<3> m_attributes;
	  uint64_t m_entityID;
	  std::string m_entityName;

	  int m_layer{ 0 };

	  void onEnable();
	  void guiUpdate();
	  void update();
	  void fixedUpdate();
	  void lateUpdate();
	  void checkDestroy();
	  void onDisable();

	  void setAttribute(EntityAttributes attr, bool value);
	  bool getAttribute(EntityAttributes attr) const;
	  Entity(const std::string& name, bool entityStatic = false);
  };

  template<typename T, bool isActive, typename... Args>
  T* Entity::addComponent(Args&&... args) {
	if constexpr (!std::is_base_of<class Component, T>::value) {
	  PRIMAL_CORE_ERROR("Entity::addComponent => {0} is not a derived class from Component class", typeid(T).name());
	} else {
	  std::type_index typeIndex{typeid(T)};
	  if (std::any_of(
			std::execution::par, Component::uniqueComponents().begin(),
			Component::uniqueComponents().end(),
			[typeIndex](std::type_index type) { return type == typeIndex; }) &&
		  std::any_of(
			std::execution::par, m_componentTypes.begin(), m_componentTypes.end(),
			[typeIndex](std::type_index type) { return type == typeIndex; })) {
		PRIMAL_CORE_ERROR("Entity::addComponent => Adding multiple excluded components {0}", typeIndex.name());
	  }
	  auto component = new T(std::forward<Args>(args)...);
	  (Entity *&)(component->entity) = this;
	  (Transform *&)(component->transform) = transform;
	  component->setActive(isActive);
	  if (isActive) {
		component->awake();
		component->setAttribute(Component::ComponentAttributes::HAS_AWAKEN, true);
		component->onEnable();
	  }
	  m_componentTypes.emplace_back(typeIndex);
	  m_components.emplace_back(component);

	  SceneManager::instance().loadedScene->addComponentToStart(component);
	  return component;
	}
  }

  template<typename T, typename... Args>
  T* Entity::addComponent(Args&&... args) {
	return addComponent<T, true>(std::forward<Args>(args)...);
  }

  template<typename T>
	T* Entity::getComponent() {
	  auto types = Component::childrenTypes();
	  auto availableTypes = types.at(std::type_index(typeid(T)));
	  for (int i = 0; i < m_componentTypes.size(); ++i) {
		auto componentType = m_componentTypes[i];
		if (std::any_of(std::execution::par, availableTypes.begin(), availableTypes.end(), [componentType](std::type_index x) { return x == componentType; })) {
		  return static_cast<T*>(m_components[i]);
		}
	  }
	  return nullptr;
	}

  template<typename T>
	std::vector<T*> Entity::getComponents() {
	  auto types = Component::childrenTypes();
	  auto availableTypes = types.at(std::type_index(typeid(T)));
	  std::vector<T*> returnValue;
	  for (int i = 0; i < m_componentTypes.size(); ++i) {
		auto componentType = m_componentTypes[i];
		if (std::any_of(std::execution::par, availableTypes.begin(), availableTypes.end(), [componentType](std::type_index x) { return x == componentType; })) {
		  returnValue.emplace_back(static_cast<T*>(m_components[i]));
		}
	  }
	  return returnValue;
	}

  template<typename T>
	inline T* Entity::getComponentInParent() {
	  return transform->m_parent->entity->getComponent<T>();
	}

  template<typename T>
  inline std::vector<T*> Entity::getComponentsInParent() {
	return transform->m_parent->entity->getComponents<T>();
  }

  template<typename T>
  inline T* Entity::getComponentInChildren() {
	T* component = nullptr;
	for (auto& child : *transform) {
	  component = child->entity->getComponent<T>();
	}
	return component;
  }

  template<typename T>
  inline std::vector<T*> Entity::getComponentsInChildren() {
	std::vector<T*> components;

	for (auto& child : *transform) {
	  std::vector<T*> c;
	  c = child->entity->getComponents<T>();
	  components.insert(components.end(), c.begin(), c.end());
	}
	return components;
  }

}

#endif
