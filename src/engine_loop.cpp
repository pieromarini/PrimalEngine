#include "core/core.h"

#include "engine_loop.h"
#include "core/memory/memory_manager.h"
#include "modules/graphics/graphics_module.h"
#include "modules/graphics/window_module.h"
#include "input/input_module.h"
#include "scene/scene.h"
#include "tools/log.h"
#include "scene/scene_manager.h"

namespace primal {
  
  EngineLoop& EngineLoop::instance() {
	static EngineLoop instance;
	return instance;
  }

  EngineLoop::EngineLoop() : m_isRunning{ false } {
	Log::init();

	m_memoryManager = new MemoryManager{};

	m_windowModule = MemoryManager::newOnStack<WindowModule>();
	m_graphicsModule = MemoryManager::newOnStack<GraphicsModule>();
	m_inputModule = MemoryManager::newOnStack<InputModule>();
  }

  EngineLoop::~EngineLoop() {
	m_graphicsModule->~GraphicsModule();
	m_windowModule->~WindowModule();
	m_inputModule->~InputModule();

	delete m_memoryManager;
  }

  void EngineLoop::init() {
	m_windowModule->init();
	m_graphicsModule->init(m_windowModule->m_windowHandle);
	m_inputModule->init(m_windowModule->m_windowHandle);

	m_isRunning = true;

	// NOTE: loading test scene.
	SceneManager::instance().loadScene("InputScene");
	SceneManager::instance().loadScene();

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

	SceneManager::instance().loadedScene->update();

	// TODO: Update events here

	SceneManager::instance().loadedScene->lateUpdate();

	m_graphicsModule->update(deltaTime);

	m_windowModule->update(deltaTime);

	m_memoryManager->update();

	// NOTE: load new scene if there is a pending load.
	if (SceneManager::instance().pendingLoadScene) {
	  SceneManager::instance().unloadScene();
	  m_inputModule->clear();
	  SceneManager::instance().loadScene();
	}
  }

  void EngineLoop::shutdown() {
	SceneManager::instance().unloadScene();
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
