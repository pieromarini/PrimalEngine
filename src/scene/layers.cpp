#include "layers.h"
#include "tools/log.h"
#include "util/sid.h"

namespace primal {

  std::array<std::string, Layers::MAX_LAYERS> Layers::s_layers = { "Default" };
  std::unordered_map<StringId, int> Layers::s_layerIndex;
  int Layers::s_size = READONLY_LAYERS;

  Layers::Constructor::Constructor() {
	s_layerIndex.reserve(MAX_LAYERS);
	for (int i = 0; i< READONLY_LAYERS; ++i) {
	  s_layerIndex.emplace(SID(s_layers.at(i).data()), s_size);
	}
  }

  int Layers::newLayer(const std::string_view layerName) {
	if (s_size == MAX_LAYERS) {
	  PRIMAL_CORE_ERROR("Layers::newLayer: Layer max reached ({0})", MAX_LAYERS);
	}
	s_layers.at(s_size) = layerName;
	s_layerIndex.emplace(SID(layerName.data()), s_size);
	return s_size++;
  }

  int Layers::nameToLayer(const std::string_view layerName) {
	auto it = s_layerIndex.find(SID(layerName.data()));
	if (it == s_layerIndex.end()) {
	  PRIMAL_CORE_ERROR("Layers::nameToLayer: Layer doesn't exist {0}", layerName);
	}
	return it->second;
  }

  std::string Layers::layerToName(int layer) {
	if (layer > MAX_LAYERS) {
	  PRIMAL_CORE_ERROR("Layers::layerToName: Layer max reached ({0})", MAX_LAYERS);
	}
	return s_layers.at(layer);
  }

  int Layers::checkLayer(int layer) {
	if (layer > MAX_LAYERS) {
	  PRIMAL_CORE_ERROR("Layers::layerToName: Layer max reached ({0})", MAX_LAYERS);
	}
	return layer;
  }

  std::bitset<Layers::MAX_LAYERS> Layers::layerMask(std::initializer_list<int> mask) {
	std::bitset<MAX_LAYERS> bits;
	for (const auto& i : mask) {
	  bits.set(i, true);
	}
	return bits;
  }

  std::bitset<Layers::MAX_LAYERS> Layers::layerMask(std::initializer_list<std::string> mask) {
	std::bitset<MAX_LAYERS> bits;
	for (const auto& i : mask) {
	  bits.set(nameToLayer(i), true);
	}
	return bits;
  }

}
