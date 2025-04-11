#version 450

layout (binding = 1) uniform sampler2D msdfTexture;

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
  vec4 outlineColor;
  float outlineWidth;
  float outline;
} ubo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

// This should be in UBO, probably
const float pxRange = 2.0;  // Distance field pixel range in atlas

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main() {
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
	outFragColor = vec4(ubo.outlineColor.rgb, opacity);
	*/

	vec2 msdfUnit = pxRange / vec2(textureSize(msdfTexture, 0));
  vec3 s = texture(msdfTexture, inUV).rgb;
	float sigDist = median(s.r, s.g, s.b) - 0.5;
	sigDist = sigDist * dot(msdfUnit, 0.5 / fwidth(inUV));
	float opacity = clamp(sigDist + 0.5, 0.0, 1.0);

	outFragColor = vec4(ubo.outlineColor.rgb, opacity);

	/*
  float distance = texture(msdfTexture, inUV).a;
  float smoothWidth = fwidth(distance);
  float alpha = smoothstep(0.5 - smoothWidth, 0.5 + smoothWidth, distance);
  vec3 rgb = vec3(alpha);

  if (ubo.outline > 0.0) {
    float w = 1.0 - ubo.outlineWidth;
    alpha = smoothstep(w - smoothWidth, w + smoothWidth, distance);
    rgb += mix(vec3(alpha), ubo.outlineColor.rgb, alpha);
	}

  outFragColor = vec4(rgb, alpha);
	*/
}
