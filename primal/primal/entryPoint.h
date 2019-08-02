#pragma once

#ifdef PRIMAL_PLATFORM_LINUX

extern primal::Application* primal::createApplication();

int main(int argc, char** argv) {
  primal::Log::init();
  PRIMAL_CORE_WARN("Initialized Log!");

  int a = 5;
  PRIMAL_INFO("Hello! Var={0}", a);

  auto app = primal::createApplication();
  app->run();
  delete app;
}

#endif
