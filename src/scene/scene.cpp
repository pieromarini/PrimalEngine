#include "scene.h"
#include "entity.h"
#include "transform.h"

namespace primal {

  Entity* Scene::getEntityByName(const std::string_view name) {
	for (const auto& entity : m_entities) {
	  if (entity->m_entityName == name) {
		return entity;
	  }
	}
	PRIMAL_CORE_WARN("Scene::getEntityByName: Entity {0} not found", name);
	return nullptr;
  }

  std::list<Entity*> Scene::getEntitiesByName(const std::string_view name) {
	std::list<Entity*> entities;
	for (const auto& entity : m_entities) {
	  if (entity->m_entityName == name) {
		entities.emplace_back(entity);
	  }
	}
	return entities;
  }

  bool Scene::isSceneLoaded() const {
	return m_sceneLoaded;
  }

  std::list<Entity*> Scene::getEntites() const {
	return m_entities;
  }

  void Scene::unload() {
	onUnload();
	// NOTE: Should free pool here.
	// ie. m_pool.free(sceneRoot);
	for (auto& entity : m_entities) {
	  // ie. m_pool.free(entity);
	  entity = nullptr;
	}
  }

  void Scene::addComponentToStart(Component* component) {
	m_componentsToStart.push(component);
  }

  void Scene::startComponents() {
	while (!m_componentsToStart.empty()) {
	  m_componentsToStart.front()->start();
	  m_componentsToStart.pop();
	}
  }

  Entity* Scene::addEntity(std::string name, Entity* parent, bool entityStatic) {
	// get entity from pool
	// ie. pool.get(name, entityStatic);
	auto entity = new Entity(name, entityStatic);
	entity->transform->setParent(parent != nullptr ? parent->transform : sceneRoot->transform);
	m_entities.push_back(entity);
	return entity;
  }

  void Scene::update() {
	startComponents();
	for (const auto& entity : m_entities) {
	  if (entity->getActive())
		entity->update();
	}
  }

  void Scene::guiUpdate() {
	for (const auto& entity : m_entities) {
	  entity->guiUpdate();
	}
  }

  void Scene::fixedUpdate() {
	startComponents();
	for (const auto& entity : m_entities) {
	  entity->fixedUpdate();
	}
  }

  void Scene::lateUpdate() {
	for (const auto& entity : m_entities) {
	  entity->lateUpdate();
	}

	for (auto& entity : m_entities) {
	  if (entity->getAttribute(Entity::EntityAttributes::NEED_DESTROY)) {
		// NOTE: delete from pool
		// ie. pool.free(entity);
		delete entity;
		entity = nullptr;
	  }
	}

	m_entities.remove_if([](Entity* entity) { return entity == nullptr; });
  }

  // NOTE: init pool here and get root from pool.
  Scene::Scene() : sceneRoot{ new Entity("Root") } {}
}
