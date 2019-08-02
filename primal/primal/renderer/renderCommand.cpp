#include "../../platform/openGL/openGLRendererAPI.h"

#include "renderCommand.h"

namespace primal {

  RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}
