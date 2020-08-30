#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include "util/sid.h"

namespace primal {

  template<typename result, typename... T>
  using Func = std::function<result(T...)>;

  template<typename T>
  class SceneRegistry {
	protected:
	  static bool registered;
  };

  class SceneManager {
	public:
	  struct SceneConfig {
		std::string startScene { "start_scene", "EmptyScene" };
	  };

	  class Scene* loadedScene{ nullptr };

	  static SceneManager& instance();

	  bool registerScene(const std::string_view name, Func<class Scene*> scene);

	  std::vector<std::string> getSceneNames() const;

	  SceneManager() = default;
	  ~SceneManager() = default;

	  void loadScene(std::string_view sceneName);

	private:
	  std::vector<std::string> m_sceneNames;
	  std::unordered_map<StringId, Func<class Scene*>> m_scenes;
	  class Scene* pendingLoadScene{ nullptr };

	  void loadScene();
	  void unloadScene();

	  friend class EngineLoop;
  };

  template<typename T>
  bool SceneRegistry<T>::registered = SceneManager::instance().registerScene(T::getSceneName(), T::createMethod());

}

#endif
