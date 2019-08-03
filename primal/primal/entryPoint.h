#pragma once

#ifdef PRIMAL_PLATFORM_LINUX

extern primal::Application* primal::createApplication();

int main(int argc, char** argv) {
  primal::Log::init();

  auto app = primal::createApplication();
  app->run();
  delete app;
}

#endif
