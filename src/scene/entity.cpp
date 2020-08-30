#include "entity.h"
#include "component.h"
#include "layers.h"

namespace primal {

  void Entity::onEnable() {
	for (auto& component : m_components) {
	  component->onEnable();
	}
  }

  void Entity::guiUpdate() {
	for (auto& component : m_components) {
	  if (component && component->getActive() && component->getAttribute(Component::ComponentAttributes::NEED_GUI_UPDATE)) {
		component->guiUpdate();
	  }
	}
  }
  void Entity::update() {
	for (auto& component : m_components) {
	  if (component && component->getActive() && component->getAttribute(Component::ComponentAttributes::NEED_UPDATE)) {
		component->update();
	  }
	}
  }

  void Entity::fixedUpdate() {
	for (auto& component : m_components) {
	  if (component && component->getActive() && component->getAttribute(Component::ComponentAttributes::NEED_FIXED_UPDATE)) {
		component->fixedUpdate();
	  }
	}
  }

  void Entity::lateUpdate() {
	for (auto& component : m_components) {
	  if (component && component->getActive() && component->getAttribute(Component::ComponentAttributes::NEED_LATE_UPDATE)) {
		component->lateUpdate();
	  }
	}
	checkDestroy();
  }

  void Entity::checkDestroy() {

	if (getAttribute(EntityAttributes::NEED_DESTROY)) {
	  if (getActive()) {
		onDisable();
	  }
	  destroyImmediately(this);
	} else {
	  auto typeIter = m_componentTypes.begin();
	  auto componentIter = m_components.begin();
	  while (typeIter != m_componentTypes.end() && componentIter != m_components.end()) {
		auto component = *componentIter;
		if (component->getAttribute(Component::ComponentAttributes::NEED_DESTROY)) {
		   if (component->getActive())
			 component->onDisable();

		   component->onDestroy();
		   component->~Component();
		   m_components.erase(componentIter);
		   m_componentTypes.erase(typeIter);
		} else {
		  ++componentIter;
		  ++typeIter;
		}
	  }
	}
  }

  void Entity::onDisable() {
	for (auto& component : m_components) {
	  if (component->getActive()) {
		component->onDisable();
	  }
	}
  }

  void Entity::setAttribute(EntityAttributes attr, bool value) {
	m_attributes.set(static_cast<int>(attr), value);
  }

  bool Entity::getAttribute(EntityAttributes attr) const {
	return m_attributes.test(static_cast<int>(attr));
  }

  Entity::Entity(const std::string& name, bool entityStatic) : m_internalTransform { this }, m_attributes{ 0b101 }, m_entityName{ name }, transform{ &m_internalTransform }, isStatic{ entityStatic } {
	onEnable();
  }

  Entity::~Entity() {
	destroy(this);
	checkDestroy();
  }

  bool Entity::operator==(const Entity& rhs) const {
	return m_entityID == rhs.m_entityID;
  }

  std::string Entity::getEntityIDString() const {
	// TODO: implement
	return "";
  }

  Entity* Entity::create(std::string name, class Entity* parent, bool entityStatic) {
	return SceneManager::instance().loadedScene->addEntity(name, parent, entityStatic);
  }



  void Entity::destroy(Entity* entity) {
	if (entity->getAttribute(EntityAttributes::NEED_DESTROY)) {
	  return;
	}
	if (entity->transform->getParent()) {
	  entity->transform->getParent()->removeChild(entity->transform);
	}
	destroyHelper(entity);
  }

  void Entity::destroyHelper(Entity* entity) {
	std::vector<Transform*> removingChildren;
	entity->setAttribute(EntityAttributes::NEED_DESTROY, true);
	for (auto child : entity->transform->m_children) {
	  removingChildren.push_back(child);
	  destroyHelper(child->entity);
	}

	for (auto child : removingChildren) {
	  entity->transform->removeChild(child);
	}
	entity->transform->m_parent = nullptr;
  }

  void Entity::destroyImmediately(Entity* entity) {
	for (auto& component : entity->m_components) {
	  component->onDestroy();
	}
	entity->m_components.clear();
	if (entity->transform->getParent()) {
	  entity->transform->getParent()->removeChild(entity->transform);
	}
  }

  Entity* Entity::getEntityByName(const std::string& name) {
	return SceneManager::instance().loadedScene->getEntityByName(name);
  }

  std::list<Entity*> Entity::getEntitiesByName(const std::string& name) {
	return SceneManager::instance().loadedScene->getEntitiesByName(name);
  }

  void Entity::setActive(bool isActive) {
	auto active = getAttribute(EntityAttributes::IS_ACTIVE); 
	setAttribute(EntityAttributes::IS_ACTIVE, isActive);
	if (!active && isActive) {
	  onEnable();
	} else {
	  onDisable();
	}
  }

  bool Entity::getActive() const {
	return getAttribute(EntityAttributes::IS_ACTIVE);
  }

  bool Entity::isMoveable() const {
	// NOTE: Tentative interface.
	// return !isStatic || !SceneManager::instance().loadedScene->isLoaded();
	return !isStatic;
  }

  void Entity::setTransform(const math::vec3& worldPos, const math::vec3& worldEulerAngles, const math::vec3& localScale) {
	setAttribute(EntityAttributes::IS_TRANSFORM_DIRTY, true);
	transform->setWorldTransform(worldPos, worldEulerAngles, localScale);
  }

  void Entity::setLayer(int layer) {
	m_layer = Layers::checkLayer(layer);
  }

  void Entity::setLayer(std::string layer) {
	m_layer = Layers::nameToLayer(layer);
  }

  int Entity::getLayerIndex() const {
	return m_layer;
  }

  std::string Entity::getLayerName() const {
	return Layers::layerToName(m_layer);
  }

}
