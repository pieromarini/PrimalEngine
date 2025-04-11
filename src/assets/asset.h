#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace pm {

struct AtlasInfo {
	std::string type; // sdf, msdf, etc
	int distanceRange;
	int distanceRangeMiddle;
	float size;
	int width;
	int height;
	std::string yOrigin;
};

struct Metrics {
	float emSize;
	float lineHeight;
	float ascender;
	float descender;
	float underlineY;
	float underlineThickness;
};

struct PlaneBounds {
	float left;
	float bottom;
	float right;
	float top;
};

struct AtlasBounds {
	float left;
	float bottom;
	float right;
	float top;
};

struct Glyph {
	int unicode;
	float advance;
	PlaneBounds planeBounds;
	AtlasBounds atlasBounds;
};

struct KerningPair {
	int unicode1;
	int unicode2;
	float advance;
};

struct MSDFFont {
	AtlasInfo atlas;
	Metrics metrics;
	std::vector<Glyph> glyphs;
	std::vector<KerningPair> kerning;
};

struct ModelAsset {
	std::string name;
};

enum ImageType {
	JPG,
	PNG,
	BMP,
	TIFF
};

struct ImageAsset {
	std::string name;

	ImageType imageType;
	uint32_t width;
	uint32_t height;

	unsigned char* data;
	uint32_t dataSize;
};

struct FontAsset {
	std::string name;
	MSDFFont metadata;
	ImageAsset image;
};

struct AssetStorage {
	void* data;
	uint32_t size;
};

}// namespace pm
