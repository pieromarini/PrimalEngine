#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <vector>
#include <map>

#include "core/math/linear_algebra/matrix.h"
#include "core/math/linear_algebra/vector.h"
#include "render_command.h"

namespace primal::renderer {

  class Renderer;
  class Mesh;
  class Material;
  class RenderTarget;

  class CommandBuffer {
	public:
	  CommandBuffer(Renderer* renderer);
	  ~CommandBuffer();

	  // pushes render state relevant to a single render call to the command buffer.
	  void push(Mesh* mesh, Material* material, math::mat4 transform = {}, math::mat4 prevTransform = {}, math::vec3 boxMin = math::vec3{-99999.0}, math::vec3 boxMax = math::vec3{99999.0}, RenderTarget* target = nullptr);

	  // clears the command buffer; usually done after issuing all the stored render commands.
	  void clear();
	  // sorts the command buffer; first by shader, then by texture bind.
	  // TODO: build an approach using texture arrays (every push would add relevant material textures
	  // to texture array (if it wans't there already), and then add a texture index to each material
	  // slot; profile if the added texture adjustments actually saves performance!
	  void sort();

	  // returns the list of render commands. For minimizing state changes it is advised to first
	  // call Sort() before retrieving and issuing the render commands.
	  std::vector<RenderCommand> getDeferredRenderCommands(bool cull = false);

	  // returns the list of render commands of both deferred and forward pushes that require
	  // alpha blending; which have to be rendered last.
	  std::vector<RenderCommand> getAlphaRenderCommands(bool cull = false);

	  // returns the list of custom render commands per render target.
	  std::vector<RenderCommand> getCustomRenderCommands(RenderTarget* target, bool cull = false);

	  // returns the list of post-processing render commands.
	  std::vector<RenderCommand> getPostProcessingRenderCommands();

	  // returns the list of all render commands with mesh shadow casting
	  std::vector<RenderCommand> getShadowCastRenderCommands();

	private:
	  Renderer* m_renderer;

	  std::vector<RenderCommand> m_deferredRenderCommands;
	  std::vector<RenderCommand> m_alphaRenderCommands;
	  std::vector<RenderCommand> m_postProcessingRenderCommands;
	  std::map<RenderTarget*, std::vector<RenderCommand>> m_customRenderCommands;
  };

}

#endif
