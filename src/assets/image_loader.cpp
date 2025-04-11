#include "image_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace pm {

ImageAsset loadPNG(std::string name, std::string filename) {
	ImageAsset asset;

	asset.name = name;
	asset.imageType = ImageType::PNG;

	int32_t width{}, height{}, nrChannels{}; 
	asset.data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 4);
	asset.width = width;
	asset.height = height;
	asset.dataSize = width * height * 4;

	return asset;
}

ImageAsset loadPNG(std::string name, unsigned char* data, uint32_t size) {
	ImageAsset asset;

	asset.name = name;
	asset.imageType = ImageType::PNG;

	int32_t width{}, height{}, nrChannels{}; 
	asset.data = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &nrChannels, 4);

	asset.width = width;
	asset.height = height;
	asset.dataSize = size;

	return asset;
}

void destroyImageAsset(ImageAsset& asset) {
	if (asset.data) {
		stbi_image_free(asset.data);
	}
}

}// namespace pm
