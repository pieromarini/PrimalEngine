#include "component.h"
#include "entity.h"
#include "transform.h"

namespace primal {

  bool Component::m_isFlattened = false;

  bool Component::registerComponent(std::type_index curr, std::type_index base, bool isUnique) {
	if (isUnique)
	  uniqueComponents().insert(curr);

	auto children = childrenTypes(); 

	if (children.count(curr) > 0) {
	  children.at(curr).push_front(curr);
	} else {
	  children.emplace(curr, std::list<std::type_index>{curr});
	}

	if (children.count(base) > 0) {
	  children.at(base).emplace_back(curr);
	} else {
	  children.emplace(base, std::list<std::type_index>{curr});
	}

	return true;
  }

  void Component::flattenComponentList() {
	if (m_isFlattened)
	  return;	

	m_isFlattened = true;
	auto children = childrenTypes();

	std::type_index componentIndex{ typeid(Component) }; 
	auto componentList = &children.at(componentIndex);
	for (auto& childList : *componentList) {
	  if (childList != componentIndex) {
		flattenHelper(componentIndex, childList);
	  }
	}
  }

  void Component::flattenHelper(std::type_index parent, std::type_index curr) {
	auto children = childrenTypes();
	auto parentList = &children.at(parent);
	auto componentList = &children.at(curr);

	for (auto& childList : *componentList) {
	  if (childList != curr) {
		parentList->push_back(childList);
		flattenHelper(curr, childList);
	  }
	}
  }

  Component::Component() : m_attributes{ 0b1111001 }, entity{ nullptr }, transform { nullptr } {
	if (!m_isFlattened)
	  flattenComponentList();
  }

  void Component::setAttribute(ComponentAttributes attr, bool value) {
	m_attributes.set(static_cast<int>(attr), value);
  }

  bool Component::getAttribute(ComponentAttributes attr) const {
	return m_attributes.test(static_cast<int>(attr));
  }

  void Component::setActive(bool value) {
	bool isActive = getAttribute(ComponentAttributes::IS_ACTIVE);
	setAttribute(ComponentAttributes::IS_ACTIVE, value);

	if (!isActive && value) {
	  if (!getAttribute(ComponentAttributes::HAS_AWAKEN)) {
		awake();
		setAttribute(ComponentAttributes::HAS_AWAKEN, true);
	  }
	  onEnable();
	} else if (isActive && getAttribute(ComponentAttributes::HAS_AWAKEN)) {
	  onDisable();
	}
  }

  bool Component::getActive() const {
	return getAttribute(ComponentAttributes::IS_ACTIVE);
  }

  void Component::destroy(Component *component) {
	component->setAttribute(ComponentAttributes::NEED_DESTROY, true);
  }

}
