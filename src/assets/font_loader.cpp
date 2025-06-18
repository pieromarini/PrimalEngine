#include "font_loader.h"
#include "assets/image_loader.h"
#include "core/core.h"
#include <chrono>
#include <fastgltf/parser.hpp>
#include <format>
#include <ratio>
#include <simdjson.h>

namespace pm {

MSDFFont loadFontMetadata(std::string_view metadataPath) {
	auto start = std::chrono::high_resolution_clock::now();
	MSDFFont font{};

	simdjson::ondemand::parser parser;
	auto json = simdjson::padded_string::load(metadataPath);
	simdjson::ondemand::document doc = parser.iterate(json);

	auto root = doc.get_object();

	auto atlas = root["atlas"].get_object();
	font.atlas.type = atlas["type"].get_string().value();
	font.atlas.distanceRange = (int32_t)atlas["distanceRange"].get_int64();
	font.atlas.distanceRangeMiddle = (int32_t)atlas["distanceRangeMiddle"].get_int64();
	font.atlas.size = (float)atlas["size"].get_double();
	font.atlas.width = (int32_t)atlas["width"].get_int64();
	font.atlas.height = (int32_t)atlas["height"].get_int64();
	font.atlas.yOrigin = atlas["yOrigin"].get_string().value();

	auto metrics = root["metrics"].get_object();
	font.metrics.emSize = (float)metrics["emSize"].get_double();
	font.metrics.lineHeight = (float)metrics["lineHeight"].get_double();
	font.metrics.ascender = (float)metrics["ascender"].get_double();
	font.metrics.descender = (float)metrics["descender"].get_double();
	font.metrics.underlineY = (float)metrics["underlineY"].get_double();
	font.metrics.underlineThickness = (float)metrics["underlineThickness"].get_double();

	auto glyphs = root["glyphs"].get_array();
	for (auto glyph : glyphs) {
		Glyph g{};
		auto unicode = glyph["unicode"];
		i32 value{};
		if (unicode.error() != simdjson::NO_SUCH_FIELD) {
			value = (i32)unicode.get_int64();
		} else {
			auto index = glyph["index"];
			// NOTE(piero): For now, don't allow to continue if we have an invalid font.
			assert(index.error() != simdjson::NO_SUCH_FIELD);
			value = (i32)index.get_int64();
		}
		g.unicode = value;
		g.advance = (float)glyph["advance"].get_double();

		// Check if we have plane bounds
		simdjson::ondemand::object planeBounds;
		auto error = glyph["planeBounds"].get_object().get(planeBounds);

		if (!error) {
			auto planeBounds = glyph["planeBounds"];
			g.planeBounds.left = (float)planeBounds["left"].get_double();
			g.planeBounds.bottom = (float)planeBounds["bottom"].get_double();
			g.planeBounds.right = (float)planeBounds["right"].get_double();
			g.planeBounds.top = (float)planeBounds["top"].get_double();
		}

		// Check if we have atlas bounds
		simdjson::ondemand::object atlasBounds;
		error = glyph["atlasBounds"].get_object().get(atlasBounds);

		if (!error) {
			auto atlasBounds = glyph["atlasBounds"];
			g.atlasBounds.left = (float)atlasBounds["left"].get_double();
			g.atlasBounds.bottom = (float)atlasBounds["bottom"].get_double();
			g.atlasBounds.right = (float)atlasBounds["right"].get_double();
			g.atlasBounds.top = (float)atlasBounds["top"].get_double();
		}

		font.glyphs.push_back(g);
	}

	// Check kernings
	simdjson::ondemand::array kerning;
	auto error = root["kerning"].get_array().get(kerning);

	if (!error) {
		for (auto pair : kerning) {
			KerningPair k{};
			k.unicode1 = (int32_t)pair["unicode1"].get_uint64();
			k.unicode2 = (int32_t)pair["unicode2"].get_uint64();
			k.advance = (float)pair["advance"].get_double();

			font.kerning.push_back(k);
		}
	}
	auto parseTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();
	std::cout << std::format("Font Metadata parse time: {:.4}us\n", parseTime);

	return font;
}

FontAsset loadFontSDF(std::string assetName, std::string texturePath, std::string metadataPath) {
	FontAsset asset{};

	asset.name = assetName;

	asset.metadata = loadFontMetadata(metadataPath);

	// Parse PNG texture
	asset.image = loadPNG(assetName + "_atlas", texturePath);

	return asset;
}

void destroyFontSDF(FontAsset& asset) {
	destroyImageAsset(asset.image);
}

}// namespace pm
