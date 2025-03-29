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
	Entity* parent{};
	std::vector<Entity*> children{};
};

void Entity_refreshTransform(Entity* entity, const glm::mat4& parentMatrix);
void Entity_flattenHierarchyNoTransform(Entity* entity, std::vector<Entity*>& flatEntities);
void Entity_flattenHierarchy(Entity* entity, const glm::mat4& parentMatrix, std::vector<Entity*>& flatEntities);

using EntityStorage = std::unordered_map<EntityId, Entity>;

EntityStorage EntityStorage_init();
bool EntityStorage_add(EntityStorage* storage, EntityId id, Entity entity);
bool EntityStorage_remove(EntityStorage* storage, EntityId id);
uint32_t EntityStorage_size(EntityStorage* storage);
Entity& EntityStorage_get(EntityStorage* storage, EntityId id);

}// namespace pm
