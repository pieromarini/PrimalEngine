#include "entity.h"

namespace pm {

void Entity_refreshTransform(std::shared_ptr<Entity>& entity, const glm::mat4& parentMatrix) {
	entity->worldTransform = parentMatrix * entity->localTransform;
	for (auto c : entity->children) {
		Entity_refreshTransform(c, entity->worldTransform);
	}
}

void Entity_flattenHierarchy(std::shared_ptr<Entity>& entity, const glm::mat4& parentMatrix, std::vector<std::shared_ptr<Entity>>& flatEntities) {
	entity->worldTransform = parentMatrix * entity->localTransform;
	flatEntities.push_back(entity);
	for (auto c : entity->children) {
		Entity_flattenHierarchy(c, entity->worldTransform, flatEntities);
	}
}

EntityStorage EntityStorage_init() {
	return {};
}

bool EntityStorage_add(EntityStorage* storage, EntityId id, Entity entity) {
	if (storage->contains(id)) {
		std::cout << std::format("[EntityStorage] Cannot add entity with id {} because it already exists.", id);
		return false;
	}
	storage->emplace(id, entity);
	return true;
}

bool EntityStorage_remove(EntityStorage* storage, EntityId id) {
	return storage->erase(id);
}

uint32_t EntityStorage_size(EntityStorage* storage) {
	return storage->size();
}

Entity& EntityStorage_get(EntityStorage* storage, EntityId id) {
	return storage->at(id);
}

}// namespace pm
