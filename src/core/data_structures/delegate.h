#ifndef DELEGATE_H
#define DELEGATE_H

#include <list>
#include <algorithm>
#include <functional>

#include "handle_bin.h"

namespace primal {

  // TODO: Move to a global access header.
  template<typename... T>
  using Action = std::function<void(T...)>;

  template<typename... ActionArgs>
  class Delegate {
	public:
	  using KeyPair = std::pair<uint16_t, Action<const ActionArgs&...>>;

	  Delegate() = default;
	  Delegate(const Delegate&) = delete;
	  Delegate(Delegate&&) = default;

	  Delegate& operator=(const Delegate&) = delete;
	  Delegate& operator=(Delegate&&) = default;
	  ~Delegate() = default;

	  uint64_t subscribe(Action<const ActionArgs&...> action);
	  void unsubscribe(uint64_t& handle);
	  void invoke(const ActionArgs&... args);
	  void clear();

	private:
	  std::list<KeyPair> m_actions;
	  HandleBin m_handleBin;
  };

  template<typename... ActionArgs>
  uint64_t Delegate<ActionArgs...>::subscribe(Action<const ActionArgs&...> action) {
	auto handle = m_handleBin.getHandle();
	m_actions.emplace_back(handle, action);
	return handle;
  }

  // TODO: refactor / format
  template<typename... ActionArgs>
  void Delegate<ActionArgs...>::unsubscribe(uint64_t& handle) {
	if (std::find_if(m_actions.begin(), m_actions.end(), [handle](const KeyPair& a) { return a.first == handle; }) != m_actions.end()) {
	  m_actions.remove_if([handle](const KeyPair& action) { return action.first == handle; });
	  handle = 0;
	}
  }

  template<typename... ActionArgs>
  void Delegate<ActionArgs...>::invoke(const ActionArgs&... args) {
	std::list<Action<const ActionArgs&...>> callbacks;
	for (const auto& callback : m_actions) {
	  callbacks.push_back(callback.second);
	}

	for (const auto& callback : callbacks) {
	  callback(args...);
	}
  }

  template<typename... ActionArgs>
  void Delegate<ActionArgs...>::clear() {
	m_actions.clear();
	m_handleBin.clear();
  }

};


#endif
