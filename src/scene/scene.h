#ifndef SCENE_H
#define SCENE_H

#include <list>
#include <queue>
#include <set>
#include <string>

// NOTE: create method should use a stack allocator.
#define DEFINE_SCENE(NAME)                                                \
  class NAME : public primal::Scene, public primal::SceneRegistry<NAME> { \
	public:                                                               \
	  bool isRegisteredInSceneManager() const { return registered };      \
	  static inline primal::Func<NAME*> createMethod = []() {             \
		  return new NAME();                                              \
	  };                                                                  \
	  static std::string getSceneName() { return #NAME; };                \
	  std::string getName() const override { return #NAME; };             \
																		  \
	private:

#define DEFINE_SCENE_END \
  }						 \
  ;

namespace primal {

  class Scene {
	public:
	  class Entity* sceneRoot;
	  Scene();
	  virtual ~Scene() = default;
	  [[nodiscard]] virtual std::string getName() const = 0;
	  [[nodiscard]] virtual std::list<class Entity*> getEntites() const;

	  class Entity* getEntityByName(const std::string_view name);
	  std::list<class Entity*> getEntitiesByName(const std::string_view name);

	  virtual void load() = 0;
	  virtual void onUnload() {}
	  [[nodiscard]] bool isSceneLoaded() const;

	  friend class Entity;
	  friend class EngineLoop;
	  friend class SceneManager;

	  class Entity* addEntity(std::string name, class Entity* parent, bool entityStatic = false);

	protected:
	  std::list<class Entity*> m_entities;
	  std::queue<class Component*> m_componentsToStart;
	  std::set<class Component*> m_componentsToDestroy;

	private:
	  std::list<class Entity*> m_entitesToRemove;
	  void addComponentToStart(class Component* component);
	  void startComponents();

	  void unload();
	  void update();
	  void guiUpdate();
	  void fixedUpdate();
	  void lateUpdate();

	  bool m_sceneLoaded{ false };

	  // TODO: Implement a PoolAllocator for entites to live in.
	  // ie: PoolAllocator<Entity> m_pool;
  };

}

#endif
