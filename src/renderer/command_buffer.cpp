#include <algorithm>
#include <tuple>

#include "command_buffer.h"

#include "shading/material.h"
#include "components/camera.h"
#include "resources/mesh.h"
#include "renderer.h"

namespace primal::renderer {

  CommandBuffer::CommandBuffer(Renderer* renderer) {
	m_renderer = renderer;
  }

  CommandBuffer::~CommandBuffer() {
	clear();
  }

  void CommandBuffer::push(Mesh* mesh, Material* material, math::mat4 transform, math::mat4 prevTransform, math::vec3 boxMin, math::vec3 boxMax, RenderTarget* target) {
	RenderCommand command{};
	command.mesh = mesh;
	command.material = material;
	command.transform = transform;
	command.prevTransform = prevTransform;
	command.boxMin = boxMin;
	command.boxMax = boxMax;

	if (material->blend) {
	  material->type = MATERIAL_CUSTOM;
	  m_alphaRenderCommands.push_back(command);
	} else {
	  // check the type of the material and process differently where necessary
	  if (material->type == MATERIAL_DEFAULT) {
		m_deferredRenderCommands.push_back(command);
	  } else if (material->type == MATERIAL_CUSTOM) {
		if (m_customRenderCommands.find(target) != m_customRenderCommands.end())
		  m_customRenderCommands[target].push_back(command);
		else {
		  m_customRenderCommands[target] = std::vector<RenderCommand>();
		  m_customRenderCommands[target].push_back(command);
		}
	  } else if (material->type == MATERIAL_POST_PROCESS) {
		m_postProcessingRenderCommands.push_back(command);
	  }
	}
  }

  void CommandBuffer::clear() {
	m_deferredRenderCommands.clear();
	m_customRenderCommands.clear();
	m_postProcessingRenderCommands.clear();
	m_alphaRenderCommands.clear();
  }

  // custom per-element sort compare function used by the CommandBuffer::Sort() function.
  bool renderSortDeferred(const RenderCommand& a, const RenderCommand& b) {
	return a.material->getShader()->m_id < b.material->getShader()->m_id;
  }

  // sort render state
  bool renderSortCustom(const RenderCommand& a, const RenderCommand& b) {
	/* 
	   We want strict weak ordering, which states that if two objects x and y are equivalent
	   then both f(x,y) and f(y,x) should be false. Thus when comparing the object to itself
	   the comparison should always equal false.
	   We also want to do multiple sort comparisons in a single pass, so we encapsulate all 
	   relevant properties inside an n-tuple with n being equal to the number of sort queries
	   we want to do. The tuple < comparison operator initially compares its left-most element
	   and then works along the next elements of the tuple until an outcome is clear.
	   Another (non C++11) alternative is to write out both the < and > case with the == case
	   defaulting to false as in:
	   if(a1 < b1)
	   return true;
	   if(b1 > a1)
	   return false;

	   if(a2 < b2)
	   return true;
	   if(b2 > a2)
	   return false;
	   [...] and so on for each query you want to perform
	   return false;
	   */
	return std::make_tuple(a.material->blend, a.material->getShader()->m_id) < std::make_tuple(b.material->blend, b.material->getShader()->m_id);
  }

  bool renderSortShader(const RenderCommand& a, const RenderCommand& b) {
	return a.material->getShader()->m_id < b.material->getShader()->m_id;
  }

  void CommandBuffer::sort() {
	std::sort(m_deferredRenderCommands.begin(), m_deferredRenderCommands.end(), renderSortDeferred);
	for (auto rtIt = m_customRenderCommands.begin(); rtIt != m_customRenderCommands.end(); rtIt++) {
	  std::sort(rtIt->second.begin(), rtIt->second.end(), renderSortCustom);
	}
  }

  std::vector<RenderCommand> CommandBuffer::getDeferredRenderCommands(bool cull) {
	if (cull) {
	  std::vector<RenderCommand> commands;
	  for (auto it = m_deferredRenderCommands.begin(); it != m_deferredRenderCommands.end(); ++it) {
		RenderCommand command = *it;
		if (m_renderer->GetCamera()->Frustum.intersect(command.boxMin, command.boxMax)) {
		  commands.push_back(command);
		}
	  }
	  return commands;
	} else {
	  return m_deferredRenderCommands;
	}
  }

  std::vector<RenderCommand> CommandBuffer::getCustomRenderCommands(RenderTarget* target, bool cull) {
	// only cull when on main/null render target
	if (target == nullptr && cull) {
	  std::vector<RenderCommand> commands;
	  for (auto it = m_customRenderCommands[target].begin(); it != m_customRenderCommands[target].end(); ++it) {
		RenderCommand command = *it;
		if (m_renderer->GetCamera()->Frustum.intersect(command.boxMin, command.boxMax)) {
		  commands.push_back(command);
		}
	  }
	  return commands;
	} else {
	  return m_customRenderCommands[target];
	}
  }

  std::vector<RenderCommand> CommandBuffer::getAlphaRenderCommands(bool cull) {
	if (cull) {
	  std::vector<RenderCommand> commands;
	  for (auto it = m_alphaRenderCommands.begin(); it != m_alphaRenderCommands.end(); ++it) {
		RenderCommand command = *it;
		if (m_renderer->GetCamera()->Frustum.intersect(command.boxMin, command.boxMax)) {
		  commands.push_back(command);
		}
	  }
	  return commands;
	} else {
	  return m_alphaRenderCommands;
	}
  }

  std::vector<RenderCommand> CommandBuffer::getPostProcessingRenderCommands() {
	return m_postProcessingRenderCommands;
  }

  std::vector<RenderCommand> CommandBuffer::getShadowCastRenderCommands() {
	std::vector<RenderCommand> commands;
	for (auto it = m_deferredRenderCommands.begin(); it != m_deferredRenderCommands.end(); ++it) {
	  if (it->material->shadowCast) {
		commands.push_back(*it);
	  }
	}
	for (auto it = m_customRenderCommands[nullptr].begin(); it != m_customRenderCommands[nullptr].end(); ++it) {
	  if (it->material->shadowCast) {
		commands.push_back(*it);
	  }
	}
	return commands;
  }

}
