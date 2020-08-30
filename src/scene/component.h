#ifndef COMPONENT_H
#define COMPONENT_H

#include <bitset>
#include <list>
#include <set>
#include <typeindex>
#include <unordered_map>

#define DEFINE_COMPONENT(NAME, BASE, UNIQUE) \
  template<bool Unique>                      \
  class ComponentRegistry<class NAME, BASE, Unique> { \
	protected:                                                \
	  static bool NAME##Registered;                           \
  };                                                          \
  class NAME : public BASE, public ComponentRegistry<NAME, BASE, UNIQUE> { \
	protected:																	   \
	  static bool isRegistered() { return NAME##Registered; }                      \
	private:


#define DEFINE_COMPONENT_END(NAME, BASE)                                 \
  }                                                                      \
  ;                                                                      \
  template<bool Unique>                                                  \
  bool ComponentRegistry<NAME, BASE, Unique>::NAME##Registered = \
	Component::registerComponent(std::type_index(typeid(NAME)), std::type_index(typeid(BASE)), Unique);


#define REGISTER_COMPONENT(NAME, BASE, UNIQUE)                           \
  bool ComponentRegistry<NAME, BASE, UNIQUE>::NAME##Registered = \
	Component::registerComponent(std::type_index(typeid(NAME)), std::type_index(typeid(BASE)), UNIQUE);


namespace primal {

  template<typename Current, typename Base, bool Exclude>
  class ComponentRegistry {};

  class Component {
	public:
	  virtual ~Component() = default;
	  void setActive(bool value);
	  bool getActive() const;
	  // NOTE: might need to change from raw pointer.
	  class Entity* const entity;
	  class Transform* const transform;
	  static void destroy(Component* component);

	  virtual void awake() {}

	  virtual void onEnable() {}

	  virtual void start() {}

	  virtual void guiUpdate() {
		setAttribute(ComponentAttributes::NEED_GUI_UPDATE, false);
	  }

	  virtual void update() {
		setAttribute(ComponentAttributes::NEED_UPDATE, false);
	  }

	  virtual void lateUpdate() {
		setAttribute(ComponentAttributes::NEED_LATE_UPDATE, false);
	  }

	  virtual void fixedUpdate() {
		setAttribute(ComponentAttributes::NEED_FIXED_UPDATE, false);
	  }

	  virtual void onDestroy() {}
	  virtual void onDisable() {}

	  static bool registerComponent(std::type_index curr, std::type_index base, bool isUnique);

	protected:
	  enum class ComponentAttributes {
		IS_ACTIVE,
		HAS_AWAKEN,
		NEED_DESTROY,
		NEED_GUI_UPDATE,
		NEED_UPDATE,
		NEED_LATE_UPDATE,
		NEED_FIXED_UPDATE
	  };

	  Component();

	  void setAttribute(ComponentAttributes attr, bool value);
	  bool getAttribute(ComponentAttributes attr) const;

	private:
	  friend class Entity;
	  // TODO: Rename?
	  using ChildrenMap = std::unordered_map<std::type_index, std::list<std::type_index>>;

	  std::bitset<7> m_attributes;

	  static ChildrenMap& childrenTypes() {
		static ChildrenMap children{};
		return children;
	  }

	  static std::set<std::type_index>& uniqueComponents() {
		static std::set<std::type_index> uniques{};
		return uniques;
	  }

	  static void flattenComponentList();
	  static void flattenHelper(std::type_index parent, std::type_index curr);
	  static bool m_isFlattened;
  };

}

#endif
