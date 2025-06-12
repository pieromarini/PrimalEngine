#version 450

struct FontDraw {
	vec4 textColor;
};

layout (set = 0, binding = 0) uniform UBO {
	mat4 projection;
	mat4 view;
	float pxRange;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D msdfTexture;

layout (set = 0, binding = 3, std430) readonly buffer Draws {
  FontDraw draws[];
};

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main() {
	FontDraw draw = draws[drawId];
	/*
	vec3 s = texture(msdfTexture, inUV).rgb;
	float sigDist = median(s.r, s.g, s.b) - 0.5;

	float screenPxDist = pxRange * sigDist;
	float opacity = clamp(screenPxDist + 0.5, 0.0, 1.0);

	outFragColor = vec4(ubo.outlineColor.rgb, opacity);
	*/

	/*
	vec3 smpl = texture(msdfTexture, inUV).rgb;
	ivec2 sz = textureSize(msdfTexture, 0).xy;
	float dx = dFdx(inUV.x) * sz.x; 
	float dy = dFdy(inUV.y) * sz.y;
	float toPixels = 8.0 * inversesqrt(dx * dx + dy * dy);
	float sigDist = median(smpl.r, smpl.g, smpl.b) - 0.5;
	float opacity = clamp(sigDist * toPixels + 0.5, 0.0, 1.0);
	outFragColor = vec4(draw.textColor.rgb, opacity);
	*/

	vec2 msdfUnit = ubo.pxRange / vec2(textureSize(msdfTexture, 0));
	vec3 s = texture(msdfTexture, inUV).rgb;
	float sigDist = median(s.r, s.g, s.b) - 0.5;
	sigDist = sigDist * dot(msdfUnit, 0.5 / fwidth(inUV));
	float opacity = clamp(sigDist + 0.5, 0.0, 1.0);

	outFragColor = vec4(draw.textColor.rgb, opacity);
}
