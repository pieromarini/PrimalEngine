#include <algorithm>

#include "layerStack.h"

namespace primal {

  LayerStack::LayerStack() { }

  LayerStack::~LayerStack() {
	for (Layer* layer : m_Layers)
	  delete layer;
  }

  void LayerStack::pushLayer(Layer* layer) {
	m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
	m_LayerInsertIndex++;
	layer->onAttach();
  }

  void LayerStack::pushOverlay(Layer* overlay) {
	m_Layers.emplace_back(overlay);
	overlay->onAttach();
  }

  void LayerStack::popLayer(Layer* layer) {
	auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
	if (it != m_Layers.end()) {
	  layer->onDetach();
	  m_Layers.erase(it);
	  m_LayerInsertIndex--;
	}
  }

  void LayerStack::popOverlay(Layer* overlay) {
	auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
	if (it != m_Layers.end()) {
	  overlay->onDetach();
	  m_Layers.erase(it);
	}
  }

}
