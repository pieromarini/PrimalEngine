#pragma once

#include "core.h"
#include "application.h"

#ifdef PRIMAL_PLATFORM_LINUX

extern primal::Application* primal::createApplication();

int main(int argc, char** argv) {

  primal::Log::init();

  PRIMAL_PROFILE_BEGIN_SESSION("Startup", "PrimalProfile-Startup.json");
  auto app = primal::createApplication();
  PRIMAL_PROFILE_END_SESSION();

  PRIMAL_PROFILE_BEGIN_SESSION("Runtime", "PrimalProfile-Runtime.json");
  app->run();
  PRIMAL_PROFILE_END_SESSION();

  PRIMAL_PROFILE_BEGIN_SESSION("Shutdown", "PrimalProfile-Shutdown.json");
  delete app;
  PRIMAL_PROFILE_END_SESSION();
}

#endif
