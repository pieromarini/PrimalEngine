#pragma once

#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "material.h"
#include "geometry.h"

namespace pm {

using EntityId = uint32_t;

struct Entity {
	EntityId id{};
	MaterialIndex materialIndex{};
	std::shared_ptr<Mesh> mesh{};

	// Scene Hierarchy
	glm::mat4 localTransform{};
	glm::mat4 worldTransform{};
	std::weak_ptr<Entity> parent{};
	std::vector<std::shared_ptr<Entity>> children{};
};

void Entity_refreshTransform(std::shared_ptr<Entity>& entity, const glm::mat4& parentMatrix);
void Entity_flattenHierarchy(std::shared_ptr<Entity>& entity, const glm::mat4& parentMatrix, std::vector<std::shared_ptr<Entity>>& flatEntities);

using EntityStorage = std::unordered_map<EntityId, Entity>;

EntityStorage EntityStorage_init();
bool EntityStorage_add(EntityStorage* storage, EntityId id, Entity entity);
bool EntityStorage_remove(EntityStorage* storage, EntityId id);
uint32_t EntityStorage_size(EntityStorage* storage);
Entity& EntityStorage_get(EntityStorage* storage, EntityId id);

}// namespace pm
