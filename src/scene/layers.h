#ifndef LAYERS_H
#define LAYERS_H

#include <initializer_list>
#include <bitset>
#include <string>
#include <array>
#include <unordered_map>

#include "util/sid.h"

namespace primal {

  class Layers {
	public:
	  static constexpr int MAX_LAYERS = 32;
	  static constexpr int READONLY_LAYERS = 2;

	  static int newLayer(const std::string_view layerName);
	  static int nameToLayer(const std::string_view layerName);

	  static std::string layerToName(int layer);
	  static int checkLayer(int layer);

	  static std::bitset<MAX_LAYERS> layerMask(std::initializer_list<int> mask);
	  static std::bitset<MAX_LAYERS> layerMask(std::initializer_list<std::string> mask);


	private:
	  static std::array<std::string, MAX_LAYERS> s_layers;
	  static std::unordered_map<StringId, int> s_layerIndex;
	  static int s_size;

	  Layers() = default;

	  struct Constructor {
		Constructor();
	  };
	  static Constructor s_construct;
  };

}


#endif
