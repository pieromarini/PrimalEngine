#ifndef PRIMAL_H
#define PRIMAL_H

#include "tools/log.h"
#include "tools/instrumentor.h"

#include "util/util.h"

#include "application.h"

#include "core/config.h"
#include "core/math/math.h"
#include "core/time/time.h"
#include "core/memory/ptr.h"

#include "core/core.h"
#include "core/time.h"

#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "scene/component.h"
#include "scene/entity.h"
#include "scene/layers.h"
#include "scene/transform.h"

#include "resources/resources.h"
#include "resources/mesh.h"
#include "resources/primitives/cube.h"
#include "resources/primitives/sphere.h"
#include "resources/primitives/quad.h"

#include "input/input.h"

#include "renderer/renderer.h"
#include "renderer/render_target.h"
#include "renderer/render_command.h"
#include "renderer/post_processor.h"
#include "renderer/pbr.h"
#include "renderer/material_library.h"
#include "renderer/shading/shader.h"
#include "renderer/shading/shading_types.h"
#include "renderer/shading/texture.h"
#include "renderer/shading/material.h"

#include "components/camera.h"
#include "components/fly_camera.h"
#include "components/mesh_component.h"

#include "components/directional_light.h"
#include "components/point_light.h"


#endif
