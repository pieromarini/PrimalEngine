#include "../../platform/openGL/openGLRendererAPI.h"

#include "renderCommand.h"

namespace primal {

  RendererAPI* RenderCommand::s_rendererAPI = new OpenGLRendererAPI;

}
