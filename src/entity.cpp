#include "entity.h"
#include <queue>

namespace pm {

void Entity_refreshTransform(Entity* entity, const glm::mat4& parentMatrix) {
	entity->worldTransform = parentMatrix * entity->localTransform;
	for (auto c : entity->children) {
		Entity_refreshTransform(c, entity->worldTransform);
	}
}

void Entity_flattenHierarchyNoTransform(Entity* entity, std::vector<Entity*>& flatEntities) {
	std::queue<Entity*> q;
	q.push(entity);

	while (!q.empty()) {
		auto& e = q.front();
		q.pop();
		flatEntities.push_back(e);
		for (auto c : e->children) {
			q.push(c);
		}
	}
}

void Entity_flattenHierarchy(Entity* entity, const glm::mat4& parentMatrix, std::vector<Entity*>& flatEntities) {
	std::queue<Entity*> q;
	q.push(entity);

	while (!q.empty()) {
		auto& e = q.front();
		q.pop();
		if (e->parent) {
			e->worldTransform = e->parent->worldTransform * e->localTransform;
		} else {
			e->worldTransform = parentMatrix * e->localTransform;
		}
		flatEntities.push_back(e);
		for (auto c : e->children) {
			q.push(c);
		}
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
