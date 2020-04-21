#include "platform/openGL/openGLRendererAPI.h"

#include "renderCommand.h"
#include "rendererAPI.h"

namespace primal {

  scope_ptr<RendererAPI> RenderCommand::s_rendererAPI = RendererAPI::create();

}
