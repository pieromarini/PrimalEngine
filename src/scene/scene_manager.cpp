#include "scene_manager.h"
#include "scene.h"
#include "tools/log.h"

namespace primal {

  SceneManager& SceneManager::instance() {
	static SceneManager instance;
	return instance;
  }

  bool SceneManager::registerScene(const std::string_view name, Func<class Scene*> scene) {
	m_scenes.insert_or_assign(SID(name.data()), scene);	
	m_sceneNames.emplace_back(name.data());
	return true;
  }

  std::vector<std::string> SceneManager::getSceneNames() const {
	return m_sceneNames;
  }

  void SceneManager::loadScene() {
	if (pendingLoadScene != nullptr) {
	  if (loadedScene != nullptr) {
		unloadScene();
	  }
	  loadedScene = pendingLoadScene;
	  pendingLoadScene = nullptr;
	  PRIMAL_CORE_INFO("Loading... {0}", loadedScene->getName());
	  loadedScene->load();
	  PRIMAL_CORE_INFO("Loading Complete");
	  loadedScene->m_sceneLoaded = true;
	}
  }

  void SceneManager::unloadScene() {
	if (loadedScene != nullptr) {
	  loadedScene->unload();
	  PRIMAL_CORE_INFO("Unloaded {0}", loadedScene->getName());
	  loadedScene->~Scene();
	  loadedScene = nullptr;
	}
  }

  void SceneManager::loadScene(std::string_view sceneName) {
	pendingLoadScene = m_scenes.at(SID(sceneName.data()))();
  }

}
