#include "core/core.h"

#include "engine_loop.h"
#include "modules/graphics/graphics_module.h"
#include "modules/graphics/window_module.h"
#include "input/input_module.h"

namespace primal {
  
  EngineLoop& EngineLoop::instance() {
	static EngineLoop instance;
	return instance;
  }

  // TODO: use stack allocator for modules.
  EngineLoop::EngineLoop() {
	m_windowModule = new WindowModule();
	m_graphicsModule = new GraphicsModule();
	m_inputModule = new InputModule();
  }

  // TODO: each module will call it's own destructor. (after implementing the stack allocator)
  EngineLoop::~EngineLoop() {
	delete m_graphicsModule;
	delete m_windowModule;
	delete m_inputModule;
  }

  void EngineLoop::init() {
	m_windowModule->init();
	m_graphicsModule->init(m_windowModule->m_windowHandle);
	m_inputModule->init(m_windowModule->m_windowHandle);

	m_isRunning = true;
	startGameClock();
  }

  void EngineLoop::run() {
	PRIMAL_CORE_ASSERT(!m_isRunning, "Engine is	already running");
	init();

	while (m_isRunning) {
	  update();
	}

	shutdown();
  }

  void EngineLoop::update() {
	getGameClock().updateTime();

	variableUpdate(getGameClock().getDeltaTime());
  }

  // NOTE: used when physics is implemented
  void EngineLoop::fixedUpdate(const float deltaTime) const {

  }

  void EngineLoop::variableUpdate(const float deltaTime) const {
	m_inputModule->update(deltaTime);
	m_windowModule->update(deltaTime);
	m_graphicsModule->update(deltaTime);
  }

  void EngineLoop::shutdown() {
	m_inputModule->shutdown();
	m_graphicsModule->shutdown();
	m_windowModule->shutdown();
  }

  void EngineLoop::startGameClock() const {
	getGameClock();
  }

  Clock& EngineLoop::getGameClock() {
	static Clock gameTime{};
	return gameTime;
  }

}
