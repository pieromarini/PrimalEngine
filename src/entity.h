#pragma once

#include <memory>
#include <unordered_map>

#include "core/math/geometry.h"
#include "core/math/math.h"
#include "renderer/material.h"


namespace pm {

using EntityId = uint32_t;

struct Entity {
	EntityId id{};
	MaterialIndex materialIndex{};
	std::shared_ptr<Mesh> mesh{};

	// Scene Hierarchy
	mat4 localTransform{};
	mat4 worldTransform{};
	Entity* parent{};
	std::vector<Entity*> children{};
};

void Entity_refreshTransform(Entity* entity, const mat4& parentMatrix);
void Entity_flattenHierarchyNoTransform(Entity* entity, std::vector<Entity*>& flatEntities);
void Entity_flattenHierarchy(Entity* entity, const mat4& parentMatrix, std::vector<Entity*>& flatEntities);

using EntityStorage = std::unordered_map<EntityId, Entity>;

EntityStorage EntityStorage_init();
bool EntityStorage_add(EntityStorage* storage, EntityId id, Entity entity);
bool EntityStorage_remove(EntityStorage* storage, EntityId id);
uint32_t EntityStorage_size(EntityStorage* storage);
Entity& EntityStorage_get(EntityStorage* storage, EntityId id);

}// namespace pm
